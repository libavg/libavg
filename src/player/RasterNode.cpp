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

#ifdef AVG_ENABLE_GL
#include "OGLSurface.h"
#endif
#include "MathHelper.h"
#include "DisplayEngine.h"

#include "../base/Logger.h"
#include "../base/XMLHelper.h"

using namespace std;

namespace avg {

RasterNode::RasterNode ()
    : m_pSurface(0),
      m_Angle(0),
      m_Pivot(-32767, -32767),
      m_MaxTileSize(IntPoint(-1,-1)),
      m_sBlendMode("blend")
{
}

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
    delete m_pSurface;
}

void RasterNode::connect(DisplayEngine * pEngine, DivNode * pParent)
{
    Node::connect(pEngine, pParent);

    m_bHasCustomPivot = ((m_Pivot.x != -32767) && (m_Pivot.y != -32767));

    if (m_MaxTileSize != IntPoint(-1, -1)) {
#ifdef AVG_ENABLE_GL        
        OGLSurface * pOGLSurface = 
            dynamic_cast<OGLSurface*>(getSurface());
        if (!pOGLSurface) {
            AVG_TRACE(Logger::WARNING, 
                    "Node "+getID()+": "
                    "Custom tile sizes are only allowed when "
                    "the display engine is OpenGL. Ignoring.");
        } else {
            pOGLSurface->setMaxTileSize(m_MaxTileSize);
        }
#else
            AVG_TRACE(Logger::WARNING, 
                    "Node "+getID()+": "
                    "Custom tile sizes are only allowed when "
                    "the display engine is OpenGL. Ignoring.");
#endif        
    }
    setBlendModeStr(m_sBlendMode);
}

int RasterNode::getNumVerticesX()
{
#ifdef AVG_ENABLE_GL
    OGLSurface * pOGLSurface = getOGLSurface();
    return pOGLSurface->getNumVerticesX(); 
#else
    return 1;
#endif
}

int RasterNode::getNumVerticesY()
{
#ifdef AVG_ENABLE_GL
    OGLSurface * pOGLSurface = getOGLSurface();
    return pOGLSurface->getNumVerticesY(); 
#else
    return 1;
#endif
}

DPoint RasterNode::getOrigVertexCoord(int x, int y)
{
#ifdef AVG_ENABLE_GL
    OGLSurface * pOGLSurface = getOGLSurface();
    return pOGLSurface->getOrigVertexCoord(x, y);
#else
    return DPoint(0,0);
#endif
}

DPoint RasterNode::getWarpedVertexCoord(int x, int y) 
{
#ifdef AVG_ENABLE_GL
    OGLSurface * pOGLSurface = getOGLSurface();
    return pOGLSurface->getWarpedVertexCoord(x, y);
#else
    return DPoint(0,0);
#endif
}

void RasterNode::setWarpedVertexCoord(int x, int y, const DPoint& Vertex)
{
#ifdef AVG_ENABLE_GL
    OGLSurface * pOGLSurface = getOGLSurface();
    pOGLSurface->setWarpedVertexCoord(x, y, Vertex);
#endif
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

Node * RasterNode::getElementByPos (const DPoint & pos)
{
    // Node isn't pickable if it's tilted or warped.
    if (fabs(m_Angle)<0.0001 && m_MaxTileSize == IntPoint(-1, -1)) {
        return Node::getElementByPos(pos);
    } else {
        return 0;
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

#ifdef AVG_ENABLE_GL
OGLSurface * RasterNode::getOGLSurface()
{
    OGLSurface * pOGLSurface = dynamic_cast<OGLSurface *>(getSurface());
    if (pOGLSurface) {
        return pOGLSurface; 
    } else {
        AVG_TRACE(Logger::ERROR, 
                "OpenGL display engine needed for node " << getID() <<
                ". Aborting.");
        exit(-1);
    }
}
#endif

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

