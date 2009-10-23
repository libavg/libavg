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
        .addArg(Arg<string>("maskhref", "", false, offsetof(RasterNode, m_sMaskHref)));
}

RasterNode::RasterNode()
    : m_pSurface(0),
      m_Material(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, false)
{
}

RasterNode::~RasterNode()
{
    if (m_pSurface) {
        delete m_pSurface;
        m_pSurface = 0;
    }
}

void RasterNode::setArgs(const ArgList& Args)
{
    AreaNode::setArgs(Args);
    if ((!ispow2(m_MaxTileSize.x) && m_MaxTileSize.x != -1)
            || (!ispow2(m_MaxTileSize.y) && m_MaxTileSize.y != -1)) 
    {
        throw Exception(AVG_ERR_OUT_OF_RANGE, 
                "maxtilewidth and maxtileheight must be powers of two.");
    }
    m_Material.setUseMipmaps(Args.getArgVal<bool>("mipmap"));
}

void RasterNode::setRenderingEngines(DisplayEngine * pDisplayEngine, 
        AudioEngine * pAudioEngine)
{
    AreaNode::setRenderingEngines(pDisplayEngine, pAudioEngine);

    getSurface();
    m_pSurface->attach(dynamic_cast<SDLDisplayEngine*>(pDisplayEngine));
    if (m_MaxTileSize != IntPoint(-1, -1)) {
        m_pSurface->setTileSize(m_MaxTileSize);
    }
    m_pSurface->setMaterial(m_Material);
    setBlendModeStr(m_sBlendMode);
    if (m_Material.getHasMask()) {
        m_pSurface->createMask(m_pMaskBmp->getSize());
        downloadMask();
    }
}

void RasterNode::disconnect(bool bKill)
{
    if (m_pSurface) {
        m_pSurface->destroy();
    }
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
                m_Material.setMask(true);
                setMaterial(m_Material);
            }
        } catch (Magick::Exception & ex) {
            m_sMaskFilename = "";
            if (getState() != Node::NS_UNCONNECTED) {
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
        if (getState() == Node::NS_CANRENDER && m_Material.getHasMask()) {
            m_pSurface->createMask(m_pMaskBmp->getSize());
            downloadMask();
        }
    }
}

VertexGrid RasterNode::getOrigVertexCoords()
{
    checkDisplayAvailable("getOrigVertexCoords");
    OGLTiledSurface * pOGLSurface = getSurface();
    return pOGLSurface->getOrigVertexCoords();
}

VertexGrid RasterNode::getWarpedVertexCoords() 
{
    checkDisplayAvailable("getWarpedVertexCoords");
    OGLTiledSurface * pOGLSurface = getSurface();
    return pOGLSurface->getWarpedVertexCoords();
}

void RasterNode::setWarpedVertexCoords(const VertexGrid& Grid)
{
    checkDisplayAvailable("setWarpedVertexCoords");
    OGLTiledSurface * pOGLSurface = getSurface();
    pOGLSurface->setWarpedVertexCoords(Grid);
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

void RasterNode::setBlendModeStr(const std::string& sBlendMode)
{
    m_sBlendMode = sBlendMode;
    m_BlendMode = DisplayEngine::stringToBlendMode(sBlendMode);
}

const std::string& RasterNode::getMaskHRef() const
{
    return m_sMaskHref;
}

void RasterNode::setMaskHRef(const string& href)
{
    m_sMaskHref = href;
    checkReload();
}

NodePtr RasterNode::getElementByPos(const DPoint & pos)
{
    // Node isn't pickable if it's warped.
    if (m_MaxTileSize == IntPoint(-1, -1)) {
        return AreaNode::getElementByPos(pos);
    } else {
        return NodePtr();
    }
}

Bitmap* RasterNode::getBitmap()
{
    Bitmap * pBmp = 0;
    if (m_pSurface) {
        pBmp = new Bitmap(*(m_pSurface->lockBmp()));
        m_pSurface->unlockBmps();
    }
    return pBmp;
}

DisplayEngine::BlendMode RasterNode::getBlendMode() const
{
    return m_BlendMode;
}

OGLTiledSurface * RasterNode::getSurface()
{
    if (!m_pSurface) {
        m_pSurface = new OGLTiledSurface(m_Material);
    }
    return m_pSurface;
}

const MaterialInfo& RasterNode::getMaterial() const
{
    return m_Material;
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
    m_pSurface->downloadTexture();
}

void RasterNode::checkDisplayAvailable(std::string sMsg)
{
    if (!(getState() == Node::NS_CANRENDER)) {
        throw Exception(AVG_ERR_UNSUPPORTED,
            string(sMsg) + ": cannot access vertex coordinates before Player.play().");
    }
}

}
