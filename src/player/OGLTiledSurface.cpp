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

#include "OGLTiledSurface.h"
#include "Player.h"
#include "SDLDisplayEngine.h"

#include "../base/MathHelper.h"
#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/ScopeTimer.h"
#include "../base/ObjectCounter.h"
#include "../base/Point.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

OGLTiledSurface::OGLTiledSurface(SDLDisplayEngine * pEngine)
    : OGLSurface(pEngine),
      m_NumTextures(-1, -1),
      m_MaxTileSize(-1,-1)
{
    ObjectCounter::get()->incRef(&typeid(*this));
}

OGLTiledSurface::~OGLTiledSurface()
{
    m_bBound = false;
    getEngine()->deregisterSurface(this);
    ObjectCounter::get()->decRef(&typeid(*this));
}

void OGLTiledSurface::create(const IntPoint& Size, PixelFormat pf, bool bFastDownload)
{
//    cerr << "create: " << Size << ", " << Bitmap::getPixelFormatString(pf) << endl;
    if (m_bBound && getSize() == Size && getPixelFormat() == pf) {
        // If nothing's changed, we can ignore everything.
        return;
    }
    m_bBound = false;
#if defined __APPLE__ || defined _WIN32
    if (!isOneTexture(Size)) {
        bFastDownload = false;
    }
#endif
    OGLSurface::create(Size, pf, bFastDownload);
        
    calcTileSizes();
    initTileVertices(m_TileVertices);
}

void OGLTiledSurface::setMaxTileSize(const IntPoint& MaxTileSize)
{
    m_bBound = false;
    m_MaxTileSize = MaxTileSize;
    if (m_MaxTileSize.x != -1) {
        m_MaxTileSize.x = nextpow2(m_MaxTileSize.x/2+1);
    }
    if (m_MaxTileSize.y != -1) {
        m_MaxTileSize.y = nextpow2(m_MaxTileSize.y/2+1);
    }
    if (getBmp(0)) {
        calcTileSizes();
        initTileVertices(m_TileVertices);
    }
}

VertexGrid OGLTiledSurface::getOrigVertexCoords()
{
    if (!m_bBound) {
        bind();
    }
    VertexGrid Grid;
    initTileVertices(Grid);
    return Grid;
}

VertexGrid OGLTiledSurface::getWarpedVertexCoords()
{
    if (!m_bBound) {
        bind();
    }
    return m_TileVertices;
}

void OGLTiledSurface::setWarpedVertexCoords(const VertexGrid& Grid)
{
    if (!m_bBound) {
        bind();
    }
    bool bGridOK = true;
    if (Grid.size() != (unsigned)(m_NumTiles.y+1)) {
        bGridOK = false;
    }
    for (unsigned i = 0; i< Grid.size(); ++i) {
        if (Grid[i].size() != (unsigned)(m_NumTiles.x+1)) {
            bGridOK = false;
        }
    }
    if (!bGridOK) {
        throw Exception(AVG_ERR_OUT_OF_RANGE, 
                "setWarpedVertexCoords() called with incorrect grid size.");
    }
    m_TileVertices = Grid;
}

string getGlModeString(int Mode) 
{
    switch (Mode) {
        case GL_ALPHA:
            return "GL_ALPHA";
        case GL_RGB:
            return "GL_RGB";
        case GL_RGBA:
            return "GL_RGBA";
        case GL_BGR:
            return "GL_BGR";
        case GL_BGRA:
            return "GL_BGRA";
        case GL_YCBCR_422_APPLE:
            return "GL_YCBCR_422_APPLE";
        default:
            return "UNKNOWN";
    }
}

void OGLTiledSurface::bind() 
{
    if (m_bBound) {
        rebind();
    } else {
        vector<vector<OGLTexturePtr> > pOldTextures = m_pTextures;
        m_pTextures.clear();
        vector<OGLTexturePtr> v;
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                "OGLTiledSurface::bind: glPixelStorei(GL_UNPACK_ALIGNMENT)");
        
        for (int y=0; y<m_NumTextures.y; y++) {
            m_pTextures.push_back(v);
            for (int x=0; x<m_NumTextures.x; x++) {
                IntPoint CurSize = m_TextureSize;
                if (y == m_NumTextures.y-1) {
                    CurSize.y = getSize().y-y*m_TextureSize.y;
                }
                if (x == m_NumTextures.x-1) {
                    CurSize.x = getSize().x-x*m_TextureSize.x;
                }
                Rect<int> CurExtent(x*m_TextureSize.x, y*m_TextureSize.y,
                        x*m_TextureSize.x+CurSize.x, y*m_TextureSize.y+CurSize.y);
                IntRect TileIndexExtents(
                        safeCeil(double(CurExtent.tl.x)/m_TileSize.x), 
                        safeCeil(double(CurExtent.tl.y)/m_TileSize.y),
                        safeCeil(double(CurExtent.br.x)/m_TileSize.x), 
                        safeCeil(double(CurExtent.br.y)/m_TileSize.y));
                if (getEngine()->getTextureMode() == GL_TEXTURE_2D) {
                    CurSize.x = nextpow2(CurSize.x);
                    CurSize.y = nextpow2(CurSize.y);
                }
/*
                cerr << "Texture: " << x << ", " << y << endl;
                cerr << "CurExtent: " << CurExtent << endl;
                cerr << "m_TileSize: " << m_TileSize << endl;
                cerr << "TileIndexExtents: " << TileIndexExtents << endl;
*/                
                OGLTexturePtr pTexture;
                bool bReuseTexture;
                PixelFormat pf = getPixelFormat();
                if (int(pOldTextures.size()) != m_NumTextures.y || 
                        int(pOldTextures[0].size()) != m_NumTextures.x) 
                {
                    bReuseTexture = false;
                } else {
                    bReuseTexture = 
                            ((pOldTextures[y][x])->getTileIndexExtent() == TileIndexExtents &&
                             (pOldTextures[y][x]->getPixelFormat() == pf));
                }
                if (bReuseTexture) {
                    pTexture = pOldTextures[y][x];
                    pTexture->resize(CurExtent, CurSize, m_TileSize);
                } else {
                    pTexture = OGLTexturePtr(new OGLTexture(CurExtent, CurSize, 
                            m_TileSize, TileIndexExtents, pf, getEngine()));
                }
                m_pTextures[y].push_back(pTexture);
                OGLMemoryMode memMode = getMemMode();
                int width = getSize().x;
                if (memMode == PBO) {
                    if (pf == YCbCr420p || pf == YCbCrJ420p) {
                        for (int i=0; i<3; i++) {
                            bindPBO(i);
                            pTexture->downloadTexture(i, getBmp(i), width, memMode);
                        }
                    } else {
                        bindPBO();
                        pTexture->downloadTexture(0, getBmp(0), width, memMode);
                    }
                    unbindPBO();
                } else {
                    pTexture->downloadTexture(0, getBmp(0), width, memMode);
                }
            }
        }
        if (getMemMode() == PBO) {
            // TODO: Is this necessary?
            unbindPBO();
        }
        m_bBound = true;
    }
}

void OGLTiledSurface::rebind()
{
//    cerr << "OGLTiledSurface::rebind()" << endl;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "AVGOGLTiledSurface::rebind: glPixelStorei(GL_UNPACK_ALIGNMENT)");
    for (unsigned int y=0; y<m_pTextures.size(); y++) {
        for (unsigned int x=0; x<m_pTextures[y].size(); x++) {
            OGLTexturePtr pTexture = m_pTextures[y][x];
            OGLMemoryMode memMode = getMemMode();
            int width = getSize().x;
            if (memMode == PBO) {
                PixelFormat pf = getPixelFormat();
                if (pf == YCbCr420p || pf == YCbCrJ420p) {
                    for (int i=0; i<3; i++) {
                        bindPBO(i);
                        pTexture->downloadTexture(i, getBmp(i), width, memMode);
                    }
                } else {
                    bindPBO();
                    pTexture->downloadTexture(0, getBmp(0), width, memMode);
                }
                unbindPBO();
            } else {
                pTexture->downloadTexture(0, getBmp(0), width, memMode);
            }
        }
    }
    if (getMemMode() == PBO) {
        // TODO: Is this necessary?
        unbindPBO();
    }
}

void OGLTiledSurface::blt32(const DPoint& DestSize, double opacity, 
        DisplayEngine::BlendMode Mode)
{
    glColor4d(1.0, 1.0, 1.0, opacity);
    blt(DestSize, Mode);
}

void OGLTiledSurface::blta8(const DPoint& DestSize, double opacity, 
        const Pixel32& color, DisplayEngine::BlendMode Mode)
{
    glColor4d(double(color.getR())/256, double(color.getG())/256, 
            double(color.getB())/256, opacity);
    blt(DestSize, Mode);
}

void OGLTiledSurface::blt(const DPoint& DestSize, 
        DisplayEngine::BlendMode Mode)
{
    if (!m_bBound) {
        bind();
    }
    bltTexture(DestSize, Mode);
}

bool OGLTiledSurface::isOneTexture(IntPoint Size)
{
    if (Size.x > getEngine()->getMaxTexSize() || 
        Size.y > getEngine()->getMaxTexSize() ||
        getEngine()->getTextureMode() == GL_TEXTURE_2D)
    {
        return false;
    } else {
        return true;
    }
}

void OGLTiledSurface::calcTileSizes()
{
    IntPoint size = getSize();
    if (getEngine()->getTextureMode() == GL_TEXTURE_2D) {
        if ((size.x > 256 && nextpow2(size.x) > size.x*1.3) ||
                (size.y > 256 && nextpow2(size.y) > size.y*1.3)) 
        {
            m_TextureSize = IntPoint(nextpow2(size.x)/2, nextpow2(size.y)/2);
        } else {
            m_TextureSize = IntPoint(nextpow2(size.x), nextpow2(size.y));
        }
    } else {
        m_TextureSize = size;
    }
    if (size.x > getEngine()->getMaxTexSize()) {
        m_TextureSize.x = getEngine()->getMaxTexSize();
    }
    if (size.y > getEngine()->getMaxTexSize()) {
        m_TextureSize.y = getEngine()->getMaxTexSize();
    }
    m_NumTextures.x = safeCeil(double(size.x)/m_TextureSize.x);
    m_NumTextures.y = safeCeil(double(size.y)/m_TextureSize.y);

    m_TileSize = m_TextureSize;
    if (m_MaxTileSize.x != -1 && m_MaxTileSize.x < m_TextureSize.x) {
        m_TileSize.x = m_MaxTileSize.x;
    }
    if (m_MaxTileSize.y != -1 && m_MaxTileSize.y < m_TextureSize.y) {
        m_TileSize.y = m_MaxTileSize.y;
    }
    m_NumTiles.x = safeCeil(double(size.x)/m_TileSize.x);
    m_NumTiles.y = safeCeil(double(size.y)/m_TileSize.y);
/*    
    cerr << "----calcTileSizes: " << endl;
    cerr << "TextureSize: " << m_TextureSize << ", NumTextures: " << m_NumTextures << endl;
    cerr << "TileSize: " << m_TileSize << ", NumTiles: " << m_NumTiles << endl;
*/    
}

void OGLTiledSurface::initTileVertices(VertexGrid& Grid)
{
    std::vector<DPoint> TileVerticesLine(m_NumTiles.x+1);
    Grid = std::vector<std::vector<DPoint> >
                (m_NumTiles.y+1, TileVerticesLine);
    for (unsigned int y=0; y<Grid.size(); y++) {
        for (unsigned int x=0; x<Grid[y].size(); x++) {
            initTileVertex(x, y, Grid[y][x]);
        }
    }
    m_FinalVertices = std::vector<std::vector<DPoint> >(m_NumTiles.y+1, TileVerticesLine);
}

void OGLTiledSurface::initTileVertex(int x, int y, DPoint& Vertex) 
{
    if (x < m_NumTiles.x) {
        Vertex.x = double(m_TileSize.x*x) / getSize().x;
    } else {
        Vertex.x = 1;
    }
    if (y < m_NumTiles.y) {
        Vertex.y = double(m_TileSize.y*y) / getSize().y;
    } else {
        Vertex.y = 1;
    }
}

void OGLTiledSurface::bltTexture(const DPoint& DestSize, 
        DisplayEngine::BlendMode Mode)
{
    switch(Mode) {
        case DisplayEngine::BLEND_BLEND:
            glproc::BlendEquation(GL_FUNC_ADD);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            checkBlendModeError("blend");
            break;
        case DisplayEngine::BLEND_ADD:
            glproc::BlendEquation(GL_FUNC_ADD);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            checkBlendModeError("add");
            break;
        case DisplayEngine::BLEND_MIN:
            glproc::BlendEquation(GL_MIN);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            checkBlendModeError("min");
            break;
        case DisplayEngine::BLEND_MAX:
            glproc::BlendEquation(GL_MAX);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            checkBlendModeError("max");
            break;
    }

    for (unsigned int y=0; y<m_FinalVertices.size(); y++) {
        for (unsigned int x=0; x<m_FinalVertices[y].size(); x++) {
            m_FinalVertices[y][x] = calcFinalVertex(DestSize, m_TileVertices[y][x]);
        }
    }

    for (unsigned int y=0; y<m_pTextures.size(); y++) {
        for (unsigned int x=0; x<m_pTextures[y].size(); x++) {
            m_pTextures[y][x]->blt(&m_FinalVertices); 
        }
    }

    PixelFormat pf = getPixelFormat();
    AVG_TRACE(Logger::BLTS, "(" << DestSize.x << ", " 
            << DestSize.y << ")" << ", m_pf: " 
            << Bitmap::getPixelFormatString(pf) << ", " 
            << getGlModeString(getEngine()->getOGLSrcMode(pf)) << "-->" 
            << getGlModeString(getEngine()->getOGLDestMode(pf)));
}

DPoint OGLTiledSurface::calcFinalVertex(const DPoint& Size,
        const DPoint & NormalizedVertex)
{
    DPoint Point;
    Point.x = Size.x*NormalizedVertex.x;
    Point.y = Size.y*NormalizedVertex.y;
    return Point;
}

void OGLTiledSurface::checkBlendModeError(const char *mode) 
{    
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        static bool bErrorReported = false;
        if (!bErrorReported) {
            AVG_TRACE(Logger::WARNING, "Blendmode "<<mode<<
                    " not supported by OpenGL implementation.");
            bErrorReported = true;
        }
    }
}

int OGLTiledSurface::getTotalTexMemory()
{
    int iAmount = 0;
    if (m_bBound) {
        for (int y=0; y<m_NumTextures.y; y++) {
            for (int x=0; x<m_NumTextures.x; x++) {
                iAmount += m_pTextures[y][x]->getTexMemDim();
            }
        }
    }
    return iAmount;
}

OGLShaderPtr OGLTiledSurface::getFragmentShader()
{
    //cerr <<"OGLSurface::getFrag"<<endl;
    if (!m_pTextures.empty()) {
        return m_pTextures[0][0]->getFragmentShader();
    } else {
        return OGLShaderPtr();
    }
}

}
