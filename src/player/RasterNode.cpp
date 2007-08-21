//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
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

#include "../avgconfig.h"

#include "RasterNode.h"

#include "MathHelper.h"
#include "DisplayEngine.h"

#include "../base/Logger.h"
#include "../base/XMLHelper.h"

using namespace std;

namespace avg {

RasterNode::RasterNode (const xmlNodePtr xmlNode, Player * pPlayer)
    : Node(xmlNode, pPlayer),
      m_pSurface(0),
      m_Angle(0),
      m_Pivot(-32767, -32767),
      m_MaxTileSize(IntPoint(-1,-1)),
      m_sBlendMode("blend")
{
    m_Angle = getDefaultedDoubleAttr (xmlNode, "angle", 0);
    m_Pivot.x = getDefaultedDoubleAttr (xmlNode, "pivotx", -32767);
    m_Pivot.y = getDefaultedDoubleAttr (xmlNode, "pivoty", -32767);
    m_MaxTileSize.x = getDefaultedIntAttr (xmlNode, "maxtilewidth", -1);
    m_MaxTileSize.y = getDefaultedIntAttr (xmlNode, "maxtileheight", -1);
    setBlendModeStr(getDefaultedStringAttr (xmlNode, "blendmode", "blend"));
}

RasterNode::~RasterNode()
{
    if (m_pSurface) {
        delete m_pSurface;
        m_pSurface = 0;
    }
}

void RasterNode::setDisplayEngine(DisplayEngine * pEngine)
{
    Node::setDisplayEngine(pEngine);

    m_bHasCustomPivot = ((m_Pivot.x != -32767) && (m_Pivot.y != -32767));

    if (m_MaxTileSize != IntPoint(-1, -1)) {
        OGLSurface * pOGLSurface = 
            dynamic_cast<OGLSurface*>(getSurface());
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
    Node::disconnect();
}

VertexGrid RasterNode::getOrigVertexCoords()
{
    OGLSurface * pOGLSurface = getOGLSurface();
    return pOGLSurface->getOrigVertexCoords();
}

VertexGrid RasterNode::getWarpedVertexCoords() 
{
    OGLSurface * pOGLSurface = getOGLSurface();
    return pOGLSurface->getWarpedVertexCoords();
}

void RasterNode::setWarpedVertexCoords(const VertexGrid& Grid)
{
    OGLSurface * pOGLSurface = getOGLSurface();
    pOGLSurface->setWarpedVertexCoords(Grid);
}

double RasterNode::getAngle() const
{
    return m_Angle;
}

void RasterNode::setAngle(double Angle)
{
    m_Angle = fmod(Angle, 2*PI);
    invalidate();
}

double RasterNode::getPivotX() const
{
    return m_Pivot.x;
}

void RasterNode::setPivotX(double Pivotx)
{
    m_Pivot = getPivot();
    m_Pivot.x = Pivotx;
    m_bHasCustomPivot = true;
}

double RasterNode::getPivotY() const
{
    return m_Pivot.y;
}

void RasterNode::setPivotY(double Pivoty)
{
    m_Pivot = getPivot();
    m_Pivot.y = Pivoty;
    m_bHasCustomPivot = true;
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

NodePtr RasterNode::getElementByPos (const DPoint & pos)
{
    // Node isn't pickable if it's tilted or warped.
    if (fabs(m_Angle)<0.0001 && m_MaxTileSize == IntPoint(-1, -1)) {
        return Node::getElementByPos(pos);
    } else {
        return NodePtr();
    }
}

Bitmap* RasterNode::getBitmap()
{
    Bitmap * pBmp = new Bitmap(*(m_pSurface->lockBmp()));
    m_pSurface->unlockBmps();
    return pBmp;
}

/*
string RasterNode::getImageFormat()
{
    PixelFormat pf = m_pSurface->lockBmp()->getPixelFormat();
    m_pSurface->unlockBmps();
    switch(pf) {
        case B8G8R8:
            return "RGB";
        case B8G8R8A8:
            return "RGBA";
        case B8G8R8X8:
            return "RGBX";
        case A8B8G8R8:
            return "ABGR";
        case X8B8G8R8:
            return "XBGR";
        case R8G8B8:
            return "RGB";
        case R8G8B8A8:
            return "RGBA";
        case R8G8B8X8:
            return "RGBX";
        case A8R8G8B8:
            return "ARGB";
        case X8R8G8B8:
            return "XRGB";
        case I8:
            return "L";
        case YCbCr422:
            return "YCbCr";
        default:
            return Bitmap::getPixelFormatString(pf);
    }
}
*/

DPoint RasterNode::getPivot()
{
    if (m_bHasCustomPivot) {
        return m_Pivot;
    } else {
        const DRect& vpt = getRelViewport();
        return DPoint (vpt.Width()/2, vpt.Height()/2);
    }
}

OGLSurface * RasterNode::getOGLSurface()
{
    OGLSurface * pOGLSurface = dynamic_cast<OGLSurface *>(getSurface());
    return pOGLSurface; 
}

DisplayEngine::BlendMode RasterNode::getBlendMode() const
{
    return m_BlendMode;
}

ISurface * RasterNode::getSurface()
{
    if (!m_pSurface) {
        m_pSurface = getEngine()->createSurface();
    }
    return m_pSurface;
}

string RasterNode::getTypeStr ()
{
    return "RasterNode";
}

}

