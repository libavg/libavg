//
// $Id$
// 

#include "AVGEvent.h"
#include "AVGMouseEvent.h"
#include "AVGContainer.h"
#include "AVGAVGNode.h"
#include "AVGJSScript.h"
#include "AVGPlayer.h"
#include "IAVGDisplayEngine.h"

#include <paintlib/plpoint.h>
#include <paintlib/plrect.h>

#include <xpcom/nsMemory.h>

#include <iostream>

using namespace std;

NS_IMPL_ISUPPORTS1(AVGNode, IAVGNode);

AVGNode::AVGNode ()
    : m_pParent(0),
      m_pPlayer(0)
{
    NS_INIT_ISUPPORTS();
}

AVGNode::~AVGNode()
{
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

/* attribute long x; */
NS_IMETHODIMP AVGNode::GetX(PRInt32 *aX)
{
    *aX = m_RelViewport.tl.x;
    return NS_OK;
}

NS_IMETHODIMP AVGNode::SetX(PRInt32 aX)
{
    invalidate();
    setViewport(aX, -32767, -32767, -32767);
    invalidate();
    return NS_OK;
}

/* attribute long y; */
NS_IMETHODIMP AVGNode::GetY(PRInt32 *aY)
{
    *aY = m_RelViewport.tl.y;
    return NS_OK;
}

NS_IMETHODIMP AVGNode::SetY(PRInt32 aY)
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

/* attribute long width; */
NS_IMETHODIMP AVGNode::GetWidth(PRInt32 *aWidth)
{
    *aWidth = m_RelViewport.Width();
    return NS_OK;
}

NS_IMETHODIMP AVGNode::SetWidth(PRInt32 aWidth)
{
    invalidate();
    setViewport(-32767, -32767, aWidth, -32767);
    invalidate();
    return NS_OK;
}

/* attribute long height; */
NS_IMETHODIMP AVGNode::GetHeight(PRInt32 *aHeight)
{
    *aHeight = m_RelViewport.Height();
    return NS_OK;
}

NS_IMETHODIMP AVGNode::SetHeight(PRInt32 aHeight)
{
    invalidate();
    setViewport(-32767, -32767, -32767, aHeight);
    invalidate();
    return NS_OK;
}

/* attribute float opacity; */
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

NS_IMETHODIMP AVGNode::GetPivotx(PRInt32 *aPivotx)
{
    *aPivotx = getPivot().x;
    return NS_OK;
}

NS_IMETHODIMP AVGNode::SetPivotx(PRInt32 aPivotx)
{
    m_Pivot = getPivot();
    m_Pivot.x = aPivotx;
    m_bHasCustomPivot = true;
    return NS_OK;
}

NS_IMETHODIMP AVGNode::GetPivoty(PRInt32 *aPivoty)
{
    *aPivoty = getPivot().y;
    return NS_OK;
}

NS_IMETHODIMP AVGNode::SetPivoty(PRInt32 aPivoty)
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


void AVGNode::initVisible(int x, int y, int z, int width, int height, 
        double opacity, double angle, int pivotx, int pivoty)
{
    PLPoint PreferredSize = getPreferredMediaSize();
    if (width == 0) {
        width = PreferredSize.x;
    }
    if (height == 0) {
        height = PreferredSize.y;
    }
    m_RelViewport = PLRect(x, y, x+width, y+height);
    m_z = z;
    m_Opacity = opacity;
    m_Angle = fmod(angle, 360);
    PLPoint pos(0,0);
    if (m_pParent) {
        pos = m_pParent->getAbsViewport().tl;
    } 
    m_AbsViewport = PLRect (pos+getRelViewport().tl, pos+getRelViewport().br);
    m_Pivot = PLPoint(pivotx, pivoty);
    m_bHasCustomPivot = ((pivotx != -32767) && (pivoty != -32767));
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

AVGNode * AVGNode::getElementByPos (const PLPoint & pos)
{
    if (getVisibleRect().Contains(pos) && getEffectiveOpacity() > 0.01) {
        return this;
    } else {
        return 0;
    }
}

void AVGNode::prepareRender (int time, const PLRect& parent)
{
    calcAbsViewport();
}

void AVGNode::maybeRender (const PLRect& Rect)
{
    bool bVisible;
    if (dynamic_cast<AVGAVGNode*>(this) != 0) {
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

void AVGNode::render (const PLRect& Rect)
{
}

bool AVGNode::obscures (const PLRect& Rect, int z)  
{
    return false;
}

void AVGNode::addDirtyRect(const PLRect& Rect)
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

PLPoint AVGNode::getPivot()
{
    if (m_bHasCustomPivot) {
        return m_Pivot;
    } else {
        const PLRect& vpt = getRelViewport();
        return PLPoint (vpt.Width()/2, vpt.Height()/2);
    }
}

AVGPlayer * AVGNode::getPlayer()
{
    return m_pPlayer;
}

void AVGNode::setViewport (int x, int y, int width, int height)
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
    m_RelViewport = PLRect (x, y, x+width, y+height);
    calcAbsViewport();
}

const PLRect& AVGNode::getRelViewport ()
{
    return m_RelViewport;
}

const PLRect& AVGNode::getAbsViewport ()
{
    return m_AbsViewport;
}

PLRect AVGNode::getVisibleRect()
{
    AVGNode * pParent = getParent();
    PLRect visRect = getAbsViewport();
    if (pParent) {
        PLRect parent = getParent()->getAbsViewport();
        visRect.Intersect(parent);
    }
    return visRect;
}

void AVGNode::calcAbsViewport()
{
    AVGNode * pParent = getParent();
    if (pParent) {
        PLRect parent = pParent->getAbsViewport();
        m_AbsViewport = PLRect(parent.tl+getRelViewport().tl, 
                parent.tl+getRelViewport().br);
    } else {
        m_AbsViewport = getRelViewport();
    }
    if (m_AbsViewport.Width() < 0 || m_AbsViewport.Height() < 0) {
        m_AbsViewport.br=m_AbsViewport.tl;
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

bool AVGNode::handleMouseEvent (AVGMouseEvent* pEvent, JSContext * pJSContext)
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

