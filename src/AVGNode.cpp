//
// $Id$
// 

#include "AVGEvent.h"
#include "AVGMouseEvent.h"
#include "AVGLogger.h"
#include "AVGContainer.h"
#include "AVGAVGNode.h"
#include "AVGJSScript.h"
#include "AVGPlayer.h"
#include "IAVGDisplayEngine.h"
#include "AVGOGLSurface.h"
#include "AVGJSPoint.h"

#include <paintlib/plpoint.h>
#include <paintlib/plrect.h>

#include <xpcom/nsMemory.h>

#include <iostream>
#include <sstream>

using namespace std;

NS_IMPL_ISUPPORTS1(AVGNode, IAVGNode);

AVGNode::AVGNode ()
    : m_pParent(0),
      m_pPlayer(0),
      m_pSurface(0),
      m_MaxTileSize(PLPoint(-1,-1))
{
    NS_INIT_ISUPPORTS();
}

AVGNode::~AVGNode()
{
    if (m_pSurface) {
        delete m_pSurface;
    }
}

NS_IMETHODIMP 
AVGNode::GetID(char **_retval)
{
    *_retval = (char*)nsMemory::Clone(m_ID.c_str(), m_ID.length()+1);
    return NS_OK;
}

NS_IMETHODIMP 
AVGNode::GetType(PRInt32 *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP 
AVGNode::GetNumChildren(PRInt32 *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP 
AVGNode::GetChild(PRInt32 i, IAVGNode **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP 
AVGNode::GetParent(IAVGNode **_retval)
{
    NS_IF_ADDREF(m_pParent);
    *_retval=m_pParent;
    return NS_OK;
}

/* long getXNumTiles (); */
NS_IMETHODIMP AVGNode::GetNumVerticesX(PRInt32 *_retval)
{
    AVGOGLSurface * pOGLSurface = getOGLSurface();
    *_retval = pOGLSurface->getNumVerticesX(); 
    return NS_OK;
}

NS_IMETHODIMP AVGNode::GetNumVerticesY(PRInt32 *_retval)
{
    AVGOGLSurface * pOGLSurface = getOGLSurface();
    *_retval = pOGLSurface->getNumVerticesY(); 
    return NS_OK;
}

/* IAVGJSPoint getOrigVertexCoord (in long x, in long y); */
NS_IMETHODIMP AVGNode::GetOrigVertexCoord(PRInt32 x, PRInt32 y, 
        IAVGJSPoint **_retval)
{
    nsresult rv;
    rv = nsComponentManager::CreateInstance (AVGJSPOINT_CONTRACTID, 0,
            NS_GET_IID(IAVGJSPoint), (void**)_retval);
    if (NS_FAILED(rv)) {
        std::cerr << "creating point failed: " << rv << std::endl;
        PLASSERT(false);
    }
    AVGJSPoint * pPoint = dynamic_cast<AVGJSPoint*>(*_retval);
    AVGOGLSurface * pOGLSurface = getOGLSurface();
    *pPoint = pOGLSurface->getOrigVertexCoord(x, y);
    return NS_OK;
}

/* IAVGJSPoint getWarpedVertexCoord (in long x, in long y); */
NS_IMETHODIMP AVGNode::GetWarpedVertexCoord(PRInt32 x, PRInt32 y, 
        IAVGJSPoint **_retval)
{
    nsresult rv;
    rv = nsComponentManager::CreateInstance (AVGJSPOINT_CONTRACTID, 0,
            NS_GET_IID(IAVGJSPoint), (void**)_retval);
    if (NS_FAILED(rv)) {
        std::cerr << "creating point failed: " << rv << std::endl;
        PLASSERT(false);
    }
    AVGJSPoint * pPoint = dynamic_cast<AVGJSPoint*>(*_retval);
    AVGOGLSurface * pOGLSurface = getOGLSurface();
    *pPoint = pOGLSurface->getWarpedVertexCoord(x, y);
    return NS_OK;
}

/* void setWarpedVertexCoord (in long x, in long y, in IAVGJSPoint Vertex); */
NS_IMETHODIMP AVGNode::SetWarpedVertexCoord(PRInt32 x, PRInt32 y, IAVGJSPoint *Vertex)
{
    AVGOGLSurface * pOGLSurface = getOGLSurface();
    AVGDPoint * pPoint = dynamic_cast<AVGDPoint*>(Vertex);
    pOGLSurface->setWarpedVertexCoord(x, y, *pPoint);
    return NS_OK;
}

NS_IMETHODIMP AVGNode::GetX(float *aX)
{
    *aX = m_RelViewport.tl.x;
    return NS_OK;
}

NS_IMETHODIMP AVGNode::SetX(float aX)
{
    invalidate();
    setViewport(aX, -32767, -32767, -32767);
    invalidate();
    return NS_OK;
}

NS_IMETHODIMP AVGNode::GetY(float *aY)
{
    *aY = m_RelViewport.tl.y;
    return NS_OK;
}

NS_IMETHODIMP AVGNode::SetY(float aY)
{
    invalidate();
    setViewport(-32767, aY, -32767, -32767);
    invalidate();
    return NS_OK;
}

/* attribute long z; */
NS_IMETHODIMP AVGNode::GetZ(PRInt32 *aZ)
{
    *aZ = m_z;
    return NS_OK;
}

NS_IMETHODIMP AVGNode::SetZ(PRInt32 aZ)
{
    m_z = aZ;
    if (getParent()) {
        getParent()->zorderChange(this);
    }
    invalidate();
    return NS_OK;
}

NS_IMETHODIMP AVGNode::GetWidth(float *aWidth)
{
    *aWidth = m_RelViewport.Width();
    return NS_OK;
}

NS_IMETHODIMP AVGNode::SetWidth(float aWidth)
{
    invalidate();
    setViewport(-32767, -32767, aWidth, -32767);
    invalidate();
    return NS_OK;
}

NS_IMETHODIMP AVGNode::GetHeight(float *aHeight)
{
    *aHeight = m_RelViewport.Height();
    return NS_OK;
}

NS_IMETHODIMP AVGNode::SetHeight(float aHeight)
{
    invalidate();
    setViewport(-32767, -32767, -32767, aHeight);
    invalidate();
    return NS_OK;
}

NS_IMETHODIMP AVGNode::GetOpacity(float *aOpacity)
{
    *aOpacity = m_Opacity;
    return NS_OK;
}

NS_IMETHODIMP AVGNode::SetOpacity(float aOpacity)
{
    m_Opacity = aOpacity;
    if (m_Opacity < 0.0) {
        m_Opacity = 0.0;
    }
    if (m_Opacity > 1.0) {
        m_Opacity = 1.0;
    }
    invalidate();
    return NS_OK;
}


NS_IMETHODIMP AVGNode::GetAngle(float *aAngle)
{
   *aAngle = m_Angle;
    return NS_OK;
}
NS_IMETHODIMP AVGNode::SetAngle(float aAngle)
{
    m_Angle = fmod(aAngle, 360);
    
    invalidate();
    return NS_OK;
}

NS_IMETHODIMP AVGNode::GetPivotx(float *aPivotx)
{
    *aPivotx = getPivot().x;
    return NS_OK;
}

NS_IMETHODIMP AVGNode::SetPivotx(float aPivotx)
{
    m_Pivot = getPivot();
    m_Pivot.x = aPivotx;
    m_bHasCustomPivot = true;
    return NS_OK;
}

NS_IMETHODIMP AVGNode::GetPivoty(float *aPivoty)
{
    *aPivoty = getPivot().y;
    return NS_OK;
}

NS_IMETHODIMP AVGNode::SetPivoty(float aPivoty)
{
    m_Pivot = getPivot();
    m_Pivot.y = aPivoty;
    m_bHasCustomPivot = true;
    return NS_OK;
}

void AVGNode::init(const string& id, IAVGDisplayEngine * pEngine, 
        AVGContainer * pParent, AVGPlayer * pPlayer)
{
    m_ID = id;
    m_pParent = pParent;
    m_pEngine = pEngine;
    m_pPlayer = pPlayer;
}


void AVGNode::initVisible(double x, double y, int z, 
        double width, double height, 
        double opacity, double angle, double pivotx, double pivoty,
        int maxTileWidth, int maxTileHeight)
{
    AVGDPoint PreferredSize = getPreferredMediaSize();
    if (width == 0) {
        width = PreferredSize.x;
    }
    if (height == 0) {
        height = PreferredSize.y;
    }
    m_RelViewport = AVGDRect(x, y, x+width, y+height);
    m_z = z;
    m_Opacity = opacity;
    m_Angle = fmod(angle, 360);
    AVGDPoint pos(0,0);
    if (m_pParent) {
        pos = m_pParent->getAbsViewport().tl;
    } 
    m_AbsViewport = AVGDRect (pos+getRelViewport().tl, pos+getRelViewport().br);

    m_Pivot = AVGDPoint(pivotx, pivoty);
    m_bHasCustomPivot = ((pivotx != -32767) && (pivoty != -32767));
    m_MaxTileSize = PLPoint(maxTileWidth, maxTileHeight);
     if (m_MaxTileSize != PLPoint(-1, -1)) {
        AVGOGLSurface * pOGLSurface = 
            dynamic_cast<AVGOGLSurface*>(m_pSurface);
        if (!pOGLSurface) {
            AVG_TRACE(AVGPlayer::DEBUG_WARNING, 
                    "Node "+m_ID+":"
                    "Custom tile sizes are only allowed when "
                    "the display engine is OpenGL. Ignoring.");
        } else {
           
            pOGLSurface->setMaxTileSize(m_MaxTileSize);
        }

    }
}

void AVGNode::InitEventHandlers
                (const string& MouseMoveHandler, 
                 const string& MouseButtonUpHandler, 
                 const string& MouseButtonDownHandler,
                 const string& MouseOverHandler, 
                 const string& MouseOutHandler)
{
    m_MouseMoveHandler = MouseMoveHandler;
    m_MouseButtonUpHandler = MouseButtonUpHandler;
    m_MouseButtonDownHandler = MouseButtonDownHandler;
    m_MouseOverHandler = MouseOverHandler;
    m_MouseOutHandler = MouseOutHandler;
}

AVGNode * AVGNode::getElementByPos (const AVGDPoint & pos)
{
    if (getVisibleRect().Contains(pos) && getEffectiveOpacity() > 0.01) {
        return this;
    } else {
        return 0;
    }
}

void AVGNode::prepareRender (int time, const AVGDRect& parent)
{
    calcAbsViewport();
}

void AVGNode::maybeRender (const AVGDRect& Rect)
{
    bool bVisible;
    if (dynamic_cast<AVGDivNode*>(this) != 0) {
        bVisible = getEngine()->pushClipRect(getVisibleRect(), true);
    } else {
        bVisible = getEngine()->pushClipRect(getVisibleRect(), false);
    }
    if (bVisible) {
        if (getEffectiveOpacity() > 0.01) {
            if (!getParent() || 
                !getParent()->obscures(getEngine()->getClipRect(), getZ())) {
                render(Rect);
            } 
        }
    }
    getEngine()->popClipRect();
}

void AVGNode::render (const AVGDRect& Rect)
{
}

bool AVGNode::obscures (const AVGDRect& Rect, int z)  
{
    return false;
}

void AVGNode::addDirtyRect(const AVGDRect& Rect)
{
    m_DirtyRegion.addRect(Rect);
}

void AVGNode::getDirtyRegion (AVGRegion& Region)
{
    Region.addRegion(m_DirtyRegion);
    m_DirtyRegion.clear();
}

void AVGNode::invalidate()
{
    addDirtyRect(getVisibleRect());
}

AVGDPoint AVGNode::getPivot()
{
    if (m_bHasCustomPivot) {
        return m_Pivot;
    } else {
        const AVGDRect& vpt = getRelViewport();
        return AVGDPoint (vpt.Width()/2, vpt.Height()/2);
    }
}

AVGPlayer * AVGNode::getPlayer()
{
    return m_pPlayer;
}

void AVGNode::setViewport (double x, double y, double width, double height)
{
    if (x == -32767) {
        x = getRelViewport().tl.x;
    }
    if (y == -32767) {
        y = getRelViewport().tl.y;
    }
    if (width == -32767) {
        width = getRelViewport().Width();
    }
    if (height == -32767) {
        height = getRelViewport().Height();
    }
    m_RelViewport = AVGDRect (x, y, x+width, y+height);
    calcAbsViewport();
}

const AVGDRect& AVGNode::getRelViewport ()
{
    return m_RelViewport;
}

const AVGDRect& AVGNode::getAbsViewport ()
{
    return m_AbsViewport;
}

AVGDRect AVGNode::getVisibleRect()
{
    AVGNode * pParent = getParent();
    AVGDRect visRect = getAbsViewport();
    if (pParent) {
        AVGDRect parent = getParent()->getVisibleRect();
        visRect.Intersect(parent);
    }
    return visRect;
}

void AVGNode::calcAbsViewport()
{
    AVGNode * pParent = getParent();
    if (pParent) {
        AVGDRect parent = pParent->getAbsViewport();
        m_AbsViewport = AVGDRect(parent.tl+getRelViewport().tl, 
                parent.tl+getRelViewport().br);
    } else {
        m_AbsViewport = getRelViewport();
    }
    if (m_AbsViewport.Width() < 0 || m_AbsViewport.Height() < 0) {
        m_AbsViewport.br=m_AbsViewport.tl;
    }
}

AVGOGLSurface * AVGNode::getOGLSurface()
{
    AVGOGLSurface * pOGLSurface = dynamic_cast<AVGOGLSurface *>(m_pSurface);
    if (pOGLSurface) {
        return pOGLSurface; 
    } else {
        AVG_TRACE(AVGPlayer::DEBUG_ERROR, 
                "OpenGL display engine needed for node " << m_ID << ". Aborting.");
        exit(-1);
    }
}

int AVGNode::getZ ()
{
    return m_z;
}

double AVGNode::getOpacity()
{
    return m_Opacity;
}

void AVGNode::setOpacity(double o)
{
    m_Opacity = o;
}

double AVGNode::getAngle()
{
    return m_Angle;
}

double AVGNode::getEffectiveOpacity()
{
    if (getParent()) {
        return m_Opacity*getParent()->getEffectiveOpacity();
    } else {
        return m_Opacity;
    }
}

IAVGDisplayEngine * AVGNode::getEngine()
{
    return m_pEngine;
}

IAVGSurface * AVGNode::getSurface()
{
    if (!m_pSurface) {
        m_pSurface = getEngine()->createSurface();
    }
    return m_pSurface;
}

string AVGNode::dump (int indent)
{
    string dumpStr = string(indent, ' ') + getTypeStr() + ": m_ID= " + m_ID;
    char sz[256];
    sprintf (sz, ", x=%i, y=%i, z=%i, width=%i, height=%i, opacity=%.2e\n",
            m_RelViewport.tl.x, m_RelViewport.tl.y, m_z, 
            m_RelViewport.Width(), m_RelViewport.Height(), m_Opacity);
    dumpStr += sz;
    sprintf (sz, "        Abs: (x=%i, y=%i, width=%i, height=%i)\n",
            m_AbsViewport.tl.x, m_AbsViewport.tl.y,  
            m_AbsViewport.Width(), m_AbsViewport.Height());
    dumpStr += sz;

    return dumpStr; 
}

string AVGNode::getTypeStr ()
{
    return "AVGNode";
}

const string& AVGNode::getID ()
{
    return m_ID;
}

AVGContainer * AVGNode::getParent()
{
    return m_pParent;
}

void AVGNode::handleMouseEvent (AVGMouseEvent* pEvent, JSContext * pJSContext)
{
    string Code;
    pEvent->setElement(this);
    int EventType;
    pEvent->GetType(&EventType);
    switch (EventType) {
        case IAVGEvent::MOUSEMOTION:
            Code = m_MouseMoveHandler;
            break;
        case IAVGEvent::MOUSEBUTTONDOWN:
            Code = m_MouseButtonDownHandler;
            break;
        case IAVGEvent::MOUSEBUTTONUP:
            Code = m_MouseButtonUpHandler;
            break;
        case IAVGEvent::MOUSEOVER:
            Code = m_MouseOverHandler;
            break;
        case IAVGEvent::MOUSEOUT:
            Code = m_MouseOutHandler;
            break;
         default:
            break;
    }
    if (!Code.empty()) {
        callJS(Code, pJSContext);
    } else {
        if (m_pParent) {
            m_pParent->handleMouseEvent (pEvent, pJSContext);
        }
    }
}

void AVGNode::callJS (const string& Code, JSContext * pJSContext)
{

    // TODO: precompile.
    AVGJSScript Script(Code, "EventScript", 0, pJSContext);
    Script.run();
}

