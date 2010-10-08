//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2008 Ulrich von Zadow
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  Current versions can be found at www.libavg.de
//

#include "RasterNode.h"

#include "NodeDefinition.h"
#include "SDLDisplayEngine.h"
#include "OGLSurface.h"

#include "../base/MathHelper.h"
#include "../base/Logger.h"
#include "../base/XMLHelper.h"
#include "../base/Exception.h"

#include <Magick++.h>

using namespace std;

namespace avg {

NodeDefinition RasterNode::createDefinition()
{
    return NodeDefinition("rasternode")
        .extendDefinition(AreaNode::createDefinition())
        .addArg(Arg<int>("maxtilewidth", -1, false, 
                offsetof(RasterNode, m_MaxTileSize.x)))
        .addArg(Arg<int>("maxtileheight", -1, false, 
                offsetof(RasterNode, m_MaxTileSize.y)))
        .addArg(Arg<string>("blendmode", "blend", false, 
                offsetof(RasterNode, m_sBlendMode)))
        .addArg(Arg<bool>("mipmap", false))
        .addArg(Arg<UTF8String>("maskhref", "", false, offsetof(RasterNode, m_sMaskHref)))
        .addArg(Arg<DPoint>("maskpos", DPoint(0,0), false,
                offsetof(RasterNode, m_MaskPos)))
        .addArg(Arg<DPoint>("masksize", DPoint(0,0), false,
                offsetof(RasterNode, m_MaskSize)))
        .addArg(Arg<DTriple>("gamma", DTriple(1.0,1.0,1.0), false,
                offsetof(RasterNode, m_Gamma)))
        .addArg(Arg<DTriple>("contrast", DTriple(1.0,1.0,1.0), false,
                offsetof(RasterNode, m_Contrast)))
        .addArg(Arg<DTriple>("intensity", DTriple(1.0,1.0,1.0), false,
                offsetof(RasterNode, m_Intensity)));
}

RasterNode::RasterNode()
    : m_pSurface(0),
      m_Material(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, false),
      m_bBound(false),
      m_TileSize(-1,-1),
      m_pVertexes(0)
{
}

RasterNode::~RasterNode()
{
    if (m_pSurface) {
        delete m_pSurface;
        m_pSurface = 0;
    }
}

void RasterNode::setArgs(const ArgList& args)
{
    AreaNode::setArgs(args);
    if ((!ispow2(m_MaxTileSize.x) && m_MaxTileSize.x != -1)
            || (!ispow2(m_MaxTileSize.y) && m_MaxTileSize.y != -1)) 
    {
        throw Exception(AVG_ERR_OUT_OF_RANGE, 
                "maxtilewidth and maxtileheight must be powers of two.");
    }
    m_Material.setUseMipmaps(args.getArgVal<bool>("mipmap"));
}

void RasterNode::setRenderingEngines(DisplayEngine* pDisplayEngine, 
        AudioEngine* pAudioEngine)
{
    AreaNode::setRenderingEngines(pDisplayEngine, pAudioEngine);

    getSurface();
    m_pSurface->attach(dynamic_cast<SDLDisplayEngine*>(pDisplayEngine));
    m_bBound = false;
    if (m_MaxTileSize != IntPoint(-1, -1)) {
        m_TileSize = m_MaxTileSize;
    }
    calcVertexGrid(m_TileVertices);
    m_pSurface->setMaterial(m_Material);
    m_pSurface->downloadTexture();
    setBlendModeStr(m_sBlendMode);
    if (m_Material.getHasMask()) {
        m_pSurface->createMask(m_pMaskBmp->getSize());
        downloadMask();
        setMaskCoords();
    }
    m_pSurface->setColorParams(m_Gamma, m_Intensity, m_Contrast);
    setupFX();
}

void RasterNode::disconnect(bool bKill)
{
    if (m_pVertexes) {
        delete m_pVertexes;
        m_pVertexes = 0;
    }
    if (m_pSurface) {
        m_pSurface->destroy();
    }
    m_pFBO = FBOPtr();
    AreaNode::disconnect(bKill);
}

void RasterNode::checkReload()
{
    string sLastMaskFilename = m_sMaskFilename;
    string sMaskFilename = m_sMaskHref;
    initFilename(sMaskFilename);
    if (sLastMaskFilename != sMaskFilename) {
        m_sMaskFilename = sMaskFilename;
        try {
            if (m_sMaskFilename != "") {
                AVG_TRACE(Logger::MEMORY, "Loading " << m_sMaskFilename);
                m_pMaskBmp = BitmapPtr(new Bitmap(m_sMaskFilename));
                setMaskCoords();
            }
        } catch (Magick::Exception & ex) {
            m_sMaskFilename = "";
            if (getState() != VisibleNode::NS_UNCONNECTED) {
                AVG_TRACE(Logger::ERROR, ex.what());
            } else {
                AVG_TRACE(Logger::MEMORY, ex.what());
            }
        }
        if (m_sMaskFilename == "") {
            m_pMaskBmp = BitmapPtr();
            m_Material.setMask(false);
            setMaterial(m_Material);
        }
        if (getState() == VisibleNode::NS_CANRENDER && m_Material.getHasMask()) {
            m_pSurface->createMask(m_pMaskBmp->getSize());
            downloadMask();
        }
    } else {
        setMaskCoords();
    }
}

VertexGrid RasterNode::getOrigVertexCoords()
{
    checkDisplayAvailable("getOrigVertexCoords");
    if (!m_bBound) {
        bind();
    }
    VertexGrid grid;
    calcVertexGrid(grid);
    return grid;
}

VertexGrid RasterNode::getWarpedVertexCoords() 
{
    checkDisplayAvailable("getWarpedVertexCoords");
    if (!m_bBound) {
        bind();
    }
    return m_TileVertices;
}

void RasterNode::setWarpedVertexCoords(const VertexGrid& grid)
{
    checkDisplayAvailable("setWarpedVertexCoords");
    if (!m_bBound) {
        bind();
    }
    bool bGridOK = true;
    IntPoint numTiles = getNumTiles();
    if (grid.size() != (unsigned)(numTiles.y+1)) {
        bGridOK = false;
    }
    for (unsigned i = 0; i < grid.size(); ++i) {
        if (grid[i].size() != (unsigned)(numTiles.x+1)) {
            bGridOK = false;
        }
    }
    if (!bGridOK) {
        throw Exception(AVG_ERR_OUT_OF_RANGE, 
                "setWarpedVertexCoords() called with incorrect grid size.");
    }
    m_TileVertices = grid;
    m_bVertexArrayDirty = true;
}

int RasterNode::getMaxTileWidth() const
{
    return m_MaxTileSize.x;
}

int RasterNode::getMaxTileHeight() const
{
    return m_MaxTileSize.y;
}

bool RasterNode::getMipmap() const
{
   return m_Material.getUseMipmaps();
}

const std::string& RasterNode::getBlendModeStr() const
{
    return m_sBlendMode;
}

void RasterNode::setBlendModeStr(const string& sBlendMode)
{
    m_sBlendMode = sBlendMode;
    m_BlendMode = DisplayEngine::stringToBlendMode(sBlendMode);
}

const UTF8String& RasterNode::getMaskHRef() const
{
    return m_sMaskHref;
}

void RasterNode::setMaskHRef(const UTF8String& sHref)
{
    m_sMaskHref = sHref;
    checkReload();
}

const DPoint& RasterNode::getMaskPos() const
{
    return m_MaskPos;
}

void RasterNode::setMaskPos(const DPoint& pos)
{
    m_MaskPos = pos;
    setMaskCoords();
}

const DPoint& RasterNode::getMaskSize() const
{
    return m_MaskSize;
}

void RasterNode::setMaskSize(const DPoint& size)
{
    m_MaskSize = size;
    setMaskCoords();
}

VisibleNodePtr RasterNode::getElementByPos(const DPoint & pos)
{
    // Node isn't pickable if it's warped.
    if (m_MaxTileSize == IntPoint(-1, -1)) {
        return AreaNode::getElementByPos(pos);
    } else {
        return VisibleNodePtr();
    }
}

DTriple RasterNode::getGamma() const
{
    return m_Gamma;
}

void RasterNode::setGamma(const DTriple& gamma)
{
    m_Gamma = gamma;
    if (m_pSurface) {
        m_pSurface->setColorParams(m_Gamma, m_Intensity, m_Contrast);
    }
}

DTriple RasterNode::getIntensity() const
{
    return m_Intensity;
}

void RasterNode::setIntensity(const DTriple& intensity)
{
    m_Intensity = intensity;
    if (m_pSurface) {
        m_pSurface->setColorParams(m_Gamma, m_Intensity, m_Contrast);
    }
}

DTriple RasterNode::getContrast() const
{
    return m_Contrast;
}

void RasterNode::setContrast(const DTriple& contrast)
{
    m_Contrast = contrast;
    if (m_pSurface) {
        m_pSurface->setColorParams(m_Gamma, m_Intensity, m_Contrast);
    }
}

void RasterNode::setEffect(FXNodePtr pFXNode)
{
    m_pFXNode = pFXNode;
    setupFX();
}

BitmapPtr RasterNode::getBitmap()
{
    if (m_pSurface) {
        return m_pSurface->readbackBmp();
    } else {
        return BitmapPtr(); 
    }
}

void RasterNode::blt32(const DPoint& destSize, double opacity, 
        DisplayEngine::BlendMode mode, bool bPremultipliedAlpha)
{
    blt(destSize, mode, opacity, Pixel32(255, 255, 255, 255), bPremultipliedAlpha);
}

void RasterNode::blta8(const DPoint& destSize, double opacity, 
        const Pixel32& color, DisplayEngine::BlendMode mode)
{
    blt(destSize, mode, opacity, color, false);
}

DisplayEngine::BlendMode RasterNode::getBlendMode() const
{
    return m_BlendMode;
}

OGLSurface * RasterNode::getSurface()
{
    if (!m_pSurface) {
        m_pSurface = new OGLSurface(m_Material);
    }
    return m_pSurface;
}

const MaterialInfo& RasterNode::getMaterial() const
{
    return m_Material;
}

void RasterNode::setMaskCoords()
{
    if (m_sMaskFilename != "") {
        m_Material.setMask(true);
        calcMaskCoords(m_Material);
        setMaterial(m_Material);
    }
}

void RasterNode::calcMaskCoords(MaterialInfo& material)
{
    DPoint maskSize;
    DPoint mediaSize = DPoint(getMediaSize());
    if (m_MaskSize == DPoint(0,0)) {
        maskSize = DPoint(1,1);
    } else {
        maskSize = DPoint(m_MaskSize.x/mediaSize.x, m_MaskSize.y/mediaSize.y);
    }
    DPoint maskPos = DPoint(m_MaskPos.x/mediaSize.x, m_MaskPos.y/mediaSize.y);
    material.setMaskCoords(maskPos, maskSize);
}

void RasterNode::bind() 
{
    if (!m_bBound) {
        calcTexCoords();
    }
    m_pSurface->downloadTexture();
    m_bBound = true;
}

void RasterNode::setMaterial(const MaterialInfo& material)
{
    m_Material = material;
    getSurface()->setMaterial(m_Material);
}

void RasterNode::downloadMask()
{
    BitmapPtr pBmp = m_pSurface->lockMaskBmp();
    pBmp->copyPixels(*m_pMaskBmp);
    m_pSurface->unlockMaskBmp();
    m_pSurface->downloadMaskTexture();
}

void RasterNode::checkDisplayAvailable(std::string sMsg)
{
    if (!(getState() == VisibleNode::NS_CANRENDER)) {
        throw Exception(AVG_ERR_UNSUPPORTED,
            string(sMsg) + ": cannot access vertex coordinates before node is bound.");
    }
    if (!m_pSurface->isCreated()) {
        throw Exception(AVG_ERR_UNSUPPORTED,
            string(sMsg) + ": Surface not available.");
    }
}

void RasterNode::setupFX()
{
    if (m_pSurface && m_pSurface->getSize() != IntPoint(-1, -1) && m_pFXNode) {
        if (!getDisplayEngine()->isUsingShaders()) {
            throw Exception(AVG_ERR_UNSUPPORTED,
                    "Can't use FX - unsupported on this hardware/driver combination.");
        }
        if (!m_pFBO || m_pFBO->getSize() != m_pSurface->getSize()) {
            m_pFBO = FBOPtr(new FBO(IntPoint(m_pSurface->getSize()), B8G8R8A8, 1, 1,
                    false, getMipmap()));
            m_pFXNode->setSize(m_pSurface->getSize());
            m_pFXNode->connect(getDisplayEngine());
        }
    }
}

void RasterNode::blt(const DPoint& destSize, DisplayEngine::BlendMode mode,
        double opacity, const Pixel32& color, bool bPremultipliedAlpha)
{
    if (!m_bBound) {
        bind();
    }
    setupFX();
    getDisplayEngine()->enableGLColorArray(false);
    getDisplayEngine()->enableTexture(true);
    m_pSurface->activate(getMediaSize());
    if (m_pFXNode) {
        m_pFBO->activate();
        clearGLBuffers(GL_COLOR_BUFFER_BIT);

        glColor4d(double(color.getR())/256, double(color.getG())/256, 
                double(color.getB())/256, 1);
        if (bPremultipliedAlpha) {
            glproc::BlendColor(1.0f, 1.0f, 1.0f, 1.0f);
        }
        getDisplayEngine()->setBlendMode(DisplayEngine::BLEND_BLEND, 
                bPremultipliedAlpha);
        
        glPushAttrib(GL_VIEWPORT_BIT | GL_ENABLE_BIT);
        glDisable(GL_MULTISAMPLE);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();

        m_pFBO->setupImagingProjection();
        m_pFBO->drawImagingVertexes();


        m_pFBO->deactivate();
        m_pFBO->copyToDestTexture();
        m_pSurface->deactivate();
/*
        static int i=0;
        stringstream ss;
        ss << "foo" << i << ".png";
        BitmapPtr pBmp = m_pFBO->getImage(0);
        pBmp->save(ss.str());
  */  
        m_pFXNode->apply(m_pFBO->getTex());

        glPopAttrib();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "RasterNode::blt(): glPopMatrix");

        m_pFXNode->getTex()->activate(GL_TEXTURE0);
/*        
        stringstream ss1;
        ss1 << "bar" << i << ".png";
        i++;
        m_pFXNode->getImage()->save(ss1.str());
*/
        if (getDisplayEngine()->isUsingShaders()) {
            glproc::UseProgramObject(0);
        }
        getDisplayEngine()->setBlendMode(mode, true);
        glColor4d(1.0, 1.0, 1.0, opacity);
    } else {
    //    pBmp->dump(true);
        getDisplayEngine()->setBlendMode(mode, bPremultipliedAlpha);
        glColor4d(double(color.getR())/256, double(color.getG())/256, 
                double(color.getB())/256, opacity);
    }
    glproc::BlendColor(1.0f, 1.0f, 1.0f, float(opacity));
    FBOPtr pMainFBO = getDisplayEngine()->getMainFBO();
    if (pMainFBO) {
        pMainFBO->activate();
    } else {
        if (m_pFBO) {
            m_pFBO->deactivate();
        }
    }
    glPushMatrix();
    glScaled(destSize.x, destSize.y, 1);

    if (m_bVertexArrayDirty) {
        m_pVertexes->reset();
        for (unsigned y = 0; y < m_TileVertices.size()-1; y++) {
            for (unsigned x = 0; x < m_TileVertices[0].size()-1; x++) {
                int curVertex = m_pVertexes->getCurVert();
                m_pVertexes->appendPos(m_TileVertices[y][x], m_TexCoords[y][x]); 
                m_pVertexes->appendPos(m_TileVertices[y][x+1], m_TexCoords[y][x+1]); 
                m_pVertexes->appendPos(m_TileVertices[y+1][x+1], m_TexCoords[y+1][x+1]); 
                m_pVertexes->appendPos(m_TileVertices[y+1][x], m_TexCoords[y+1][x]); 
                m_pVertexes->appendQuadIndexes(
                        curVertex+1, curVertex, curVertex+2, curVertex+3);
            }
        }
        m_bVertexArrayDirty = false;
    }

    m_pVertexes->draw();
    m_pSurface->deactivate();

    glPopMatrix();
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "RasterNode::blt(): glPopMatrix 2");

    PixelFormat pf = m_pSurface->getPixelFormat();
    AVG_TRACE(Logger::BLTS, "(" << destSize.x << ", " << destSize.y << ")" 
            << ", m_pf: " << pf);
}

IntPoint RasterNode::getNumTiles()
{
    IntPoint size = m_pSurface->getSize();
    if (m_TileSize.x == -1) {
        return IntPoint(1,1);
    } else {
        return IntPoint(safeCeil(double(size.x)/m_TileSize.x),
                safeCeil(double(size.y)/m_TileSize.y));
    }
}

void RasterNode::calcVertexGrid(VertexGrid& grid)
{
    IntPoint numTiles = getNumTiles();
    std::vector<DPoint> TileVerticesLine(numTiles.x+1);
    grid = std::vector<std::vector<DPoint> > (numTiles.y+1, TileVerticesLine);
    for (unsigned y = 0; y < grid.size(); y++) {
        for (unsigned x = 0; x < grid[y].size(); x++) {
            calcTileVertex(x, y, grid[y][x]);
        }
    }
    if (m_pVertexes) {
        delete m_pVertexes;
    }
    m_bVertexArrayDirty = true;
    m_pVertexes = new VertexArray(numTiles.x*numTiles.y*4, numTiles.x*numTiles.y*6);
}

void RasterNode::calcTileVertex(int x, int y, DPoint& Vertex) 
{
    IntPoint numTiles = getNumTiles();
    if (x < numTiles.x) {
        Vertex.x = double(m_TileSize.x*x) / m_pSurface->getSize().x;
    } else {
        Vertex.x = 1;
    }
    if (y < numTiles.y) {
        Vertex.y = double(m_TileSize.y*y) / m_pSurface->getSize().y;
    } else {
        Vertex.y = 1;
    }
}

void RasterNode::calcTexCoords()
{
    DPoint textureSize = DPoint(m_pSurface->getTextureSize());
    DPoint imageSize = DPoint(m_pSurface->getSize());
    DPoint texCoordExtents = DPoint(imageSize.x/textureSize.x,
            imageSize.y/textureSize.y);

    DPoint texSizePerTile;
    if (m_TileSize.x == -1) {
        texSizePerTile = texCoordExtents;
    } else {
        texSizePerTile = DPoint(double(m_TileSize.x)/imageSize.x*texCoordExtents.x,
                double(m_TileSize.y)/imageSize.y*texCoordExtents.y);
    }

    IntPoint numTiles = getNumTiles();
    vector<DPoint> texCoordLine(numTiles.x+1);
    m_TexCoords = std::vector<std::vector<DPoint> > 
            (numTiles.y+1, texCoordLine);
    for (unsigned y = 0; y < m_TexCoords.size(); y++) {
        for (unsigned x = 0; x < m_TexCoords[y].size(); x++) {
            if (y == m_TexCoords.size()-1) {
                m_TexCoords[y][x].y = texCoordExtents.y;
            } else {
                m_TexCoords[y][x].y = texSizePerTile.y*y;
            }
            if (x == m_TexCoords[y].size()-1) {
                m_TexCoords[y][x].x = texCoordExtents.x;
            } else {
                m_TexCoords[y][x].x = texSizePerTile.x*x;
            }
        }
    }
    m_bVertexArrayDirty = true;
}
}
