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

using namespace std;

namespace avg {

NodeDefinition RasterNode::createDefinition()
{
    return NodeDefinition("rasternode")
        .extendDefinition(AreaNode::createDefinition())
        .addArg(Arg<int>("maxtilewidth", -1, false, offsetof(RasterNode, m_MaxTileSize.x)))
        .addArg(Arg<int>("maxtileheight", -1, false, offsetof(RasterNode, m_MaxTileSize.y)))
        .addArg(Arg<string>("blendmode", "blend", false, offsetof(RasterNode, m_sBlendMode)));
}

RasterNode::RasterNode()
    : m_pSurface(0),
      m_MaxTileSize(IntPoint(-1,-1)),
      m_sBlendMode("blend")
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
                "maxtilewidth and maxtilehight must be powers of two.");
    }
    setBlendModeStr(m_sBlendMode);
}

void RasterNode::setRenderingEngines(DisplayEngine * pDisplayEngine, 
        AudioEngine * pAudioEngine)
{
    AreaNode::setRenderingEngines(pDisplayEngine, pAudioEngine);

    OGLTiledSurface * pOGLSurface = getSurface();
    if (m_MaxTileSize != IntPoint(-1, -1) && pOGLSurface) {
        pOGLSurface->setMaxTileSize(m_MaxTileSize);
    }
    setBlendModeStr(m_sBlendMode);
}

void RasterNode::disconnect()
{
    if (m_pSurface) {
        delete m_pSurface;
        m_pSurface = 0;
    }
    AreaNode::disconnect();
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

const std::string& RasterNode::getBlendModeStr() const
{
    return m_sBlendMode;
}

void RasterNode::setBlendModeStr(const std::string& sBlendMode)
{
    m_sBlendMode = sBlendMode;
    if (m_sBlendMode == "blend") {
        m_BlendMode = DisplayEngine::BLEND_BLEND;
    } else if (m_sBlendMode == "add") {
        m_BlendMode = DisplayEngine::BLEND_ADD;
    } else if (m_sBlendMode == "min") {
        m_BlendMode = DisplayEngine::BLEND_MIN;
    } else if (m_sBlendMode == "max") {
        m_BlendMode = DisplayEngine::BLEND_MAX;
    } else {
        // TODO: throw exception here
    }
}

AreaNodePtr RasterNode::getElementByPos(const DPoint & pos)
{
    // Node isn't pickable if it's warped.
    if (m_MaxTileSize == IntPoint(-1, -1)) {
        return AreaNode::getElementByPos(pos);
    } else {
        return AreaNodePtr();
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
        DisplayEngine *pDisplayEngine = getDisplayEngine();
        if (pDisplayEngine) {
            m_pSurface = pDisplayEngine->createTiledSurface();
        } else {
            return 0;
        }
    }
    return m_pSurface;
}


OGLShaderPtr RasterNode::getFragmentShader() 
{
    //cerr<<"RasterNode::getFrag"<<endl;
    if (m_pSurface) {
        return m_pSurface->getFragmentShader();
    } else {
        return OGLShaderPtr();
    }
}

void RasterNode::checkDisplayAvailable(std::string sMsg)
{
    if (!(getState() == Node::NS_CANRENDER)) {
        throw Exception(AVG_ERR_UNSUPPORTED,
            string(sMsg) + ": cannot access vertex coordinates before Player.play().");
    }
}

}
