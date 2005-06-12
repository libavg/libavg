//
// $Id$
// 

#include "RasterNode.h"
#include "OGLSurface.h"
#include "MathHelper.h"
#include "IDisplayEngine.h"

#include "../base/Logger.h"

using namespace std;

namespace avg {

RasterNode::RasterNode ()
    : m_Angle(0),
      m_Pivot(-32767, -32767),
      m_MaxTileSize(PLPoint(-1,-1)),
      m_sBlendMode("blend"),
      m_pSurface(0)
{
}

RasterNode::~RasterNode()
{
    delete m_pSurface;
}

void RasterNode::initVisible()
{
    Node::initVisible();

    m_bHasCustomPivot = ((m_Pivot.x != -32767) && (m_Pivot.y != -32767));

    if (m_MaxTileSize != PLPoint(-1, -1)) {
        OGLSurface * pOGLSurface = 
            dynamic_cast<OGLSurface*>(m_pSurface);
        if (!pOGLSurface) {
            AVG_TRACE(Logger::WARNING, 
                    "Node "+getID()+":"
                    "Custom tile sizes are only allowed when "
                    "the display engine is OpenGL. Ignoring.");
        } else {
            pOGLSurface->setMaxTileSize(m_MaxTileSize);
        }
    }
    setBlendMode(m_sBlendMode);
}

int RasterNode::getNumVerticesX()
{
    OGLSurface * pOGLSurface = getOGLSurface();
    return pOGLSurface->getNumVerticesX(); 
}

int RasterNode::getNumVerticesY()
{
    OGLSurface * pOGLSurface = getOGLSurface();
    return pOGLSurface->getNumVerticesY(); 
}

DPoint RasterNode::getOrigVertexCoord(int x, int y)
{
    OGLSurface * pOGLSurface = getOGLSurface();
    return pOGLSurface->getOrigVertexCoord(x, y);
}

DPoint RasterNode::getWarpedVertexCoord(int x, int y) 
{
    OGLSurface * pOGLSurface = getOGLSurface();
    return pOGLSurface->getWarpedVertexCoord(x, y);
}

void RasterNode::setWarpedVertexCoord(int x, int y, const DPoint& Vertex)
{
    OGLSurface * pOGLSurface = getOGLSurface();
    pOGLSurface->setWarpedVertexCoord(x, y, Vertex);
}

void RasterNode::setAngle(double Angle)
{
    m_Angle = fmod(Angle, 2*PI);
    invalidate();
}

void RasterNode::setPivotX(double Pivotx)
{
    m_Pivot = getPivot();
    m_Pivot.x = Pivotx;
    m_bHasCustomPivot = true;
}

void RasterNode::setPivotY(double Pivoty)
{
    m_Pivot = getPivot();
    m_Pivot.y = Pivoty;
    m_bHasCustomPivot = true;
}

bool RasterNode::setBlendMode(const std::string& sBlendMode)
{
    m_sBlendMode = sBlendMode;
    if (m_sBlendMode == "blend") {
        m_BlendMode = IDisplayEngine::BLEND_BLEND;
    } else if (m_sBlendMode == "add") {
        m_BlendMode = IDisplayEngine::BLEND_ADD;
    } else if (m_sBlendMode == "min") {
        m_BlendMode = IDisplayEngine::BLEND_MIN;
    } else if (m_sBlendMode == "max") {
        m_BlendMode = IDisplayEngine::BLEND_MAX;
    } else {
        return false;
    }
    return true;
}

Node * RasterNode::getElementByPos (const DPoint & pos)
{
    // Node isn't pickable if it's tilted or warped.
    if (fabs(m_Angle)<0.0001 && m_MaxTileSize == PLPoint(-1, -1)) {
        return Node::getElementByPos(pos);
    } else {
        return 0;
    }
}

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
    OGLSurface * pOGLSurface = dynamic_cast<OGLSurface *>(m_pSurface);
    if (pOGLSurface) {
        return pOGLSurface; 
    } else {
        AVG_TRACE(Logger::ERROR, 
                "OpenGL display engine needed for node " << getID() <<
                ". Aborting.");
        exit(-1);
    }
}

IDisplayEngine::BlendMode RasterNode::getBlendMode()
{
    return m_BlendMode;
}

double RasterNode::getAngle()
{
    return m_Angle;
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

