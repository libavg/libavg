//
// $Id$
// 

#include "AVGEvent.h"
#include "AVGContainer.h"
#include "AVGAVGNode.h"
#include "AVGJSScript.h"
#include "AVGDFBDisplayEngine.h"

#include <paintlib/plpoint.h>

#include <xpcom/nsMemory.h>

#include <iostream>

using namespace std;

NS_IMPL_ISUPPORTS1(AVGNode, IAVGNode);

AVGNode::AVGNode ()
    : m_pParent(0)
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

NS_IMETHODIMP 
AVGNode::GetStringAttr(const char *name, char **_retval)
{
    cerr << "Error: Request for unknown string attribute " << name << endl;
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP 
AVGNode::GetIntAttr(const char *name, PRInt32 *_retval)
{
    if (!strcmp(name, "Left")) {
        *_retval = m_RelViewport.tl.x;
    } else if (!strcmp(name, "Top")) {
        *_retval = m_RelViewport.tl.y;
    } else if (!strcmp(name, "Width")) {
        *_retval = m_RelViewport.Width();
    } else if (!strcmp(name, "Height")) {
        *_retval = m_RelViewport.Height();
    } else if (!strcmp(name, "Z")) {
        *_retval = m_z;
    } else {
        cerr << "Error: Request for unknown int attribute " << name << endl;
        return NS_ERROR_NOT_IMPLEMENTED;
    }
    return NS_OK;
}

NS_IMETHODIMP 
AVGNode::GetFloatAttr(const char *name, float *_retval)
{
    if (!strcmp(name, "Opacity")) {
        *_retval = m_Opacity;
    } else {
        cerr << "Error: Request for unknown float attribute " << name << endl;
        return NS_ERROR_NOT_IMPLEMENTED;
    }
    return NS_OK;
}

NS_IMETHODIMP 
AVGNode::SetStringAttr(const char *name, const char *value)
{
    cerr << "Error: Setting unknown string attribute " << name << endl;
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP 
AVGNode::SetIntAttr(const char *name, PRInt32 value)
{
    if (!strcmp(name, "Left")) {
        invalidate();
        setViewport(value, -1, -1, -1);
        invalidate();
    } else if (!strcmp(name, "Top")) {
        invalidate();
        setViewport(-1, value, -1, -1);
        invalidate();
    } else if (!strcmp(name, "Width")) {
        invalidate();
        setViewport(-1, -1, value, -1);
        invalidate();
    } else if (!strcmp(name, "Height")) {
        invalidate();
        setViewport(-1, -1, -1, value);
        invalidate();
    } else if (!strcmp(name, "Z")) {
        m_z = value;
        if (getParent()) {
            getParent()->zorderChange(this);
        }
        invalidate();
    } else {
        cerr << "Error: Setting unknown int attribute " << name << endl;
        return NS_ERROR_NOT_IMPLEMENTED;
    }
    return NS_OK;
}

NS_IMETHODIMP 
AVGNode::SetFloatAttr(const char *name, float value)
{
    if (!strcmp(name, "Opacity")) {
        m_Opacity = value;
        if (m_Opacity < 0.0) {
            m_Opacity = 0.0;
        }
        if (m_Opacity > 1.0) {
            m_Opacity = 1.0;
        }
        invalidate();
    } else {
        cerr << "Error: Setting unknown float attribute " << name << endl;
        return NS_ERROR_NOT_IMPLEMENTED;
    }
    return NS_OK;
}

void AVGNode::init(const string& id, AVGDFBDisplayEngine * pEngine, AVGContainer * pParent)
{
    m_ID = id;
    m_pParent = pParent;
    m_pEngine = pEngine;
}


void AVGNode::initVisible(int x, int y, int z, int width, int height, double opacity)
{
    m_RelViewport = PLRect(x, y, x+width, y+height);
    m_z = z;
    m_Opacity = opacity;
    PLPoint pos(0,0);
    if (m_pParent) {
        pos = m_pParent->getAbsViewport().tl;
    } 
    m_AbsViewport = PLRect (pos+m_RelViewport.tl, pos+m_RelViewport.br);    
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
    if (m_AbsViewport.Contains(pos) && getEffectiveOpacity() > 0.01) {
        return this;
    } else {
        return 0;
    }
}

void AVGNode::prepareRender (int time, const PLRect& parent)
{
    m_AbsViewport = PLRect(parent.tl+m_RelViewport.tl, parent.tl+m_RelViewport.br);
    m_AbsViewport.Intersect(parent);
}

void AVGNode::maybeRender (const PLRect& Rect)
{
    bool bVisible = getEngine()->setClipRect(getAbsViewport());
    if (bVisible) {
        if (getEffectiveOpacity() > 0.01) {
            AVGNode* pParent = getParent(); 
            if (!pParent || !pParent->obscures(getEngine()->getClipRect(), getZ())) {
                render(Rect);
            } 
        }
    }
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
    addDirtyRect(m_AbsViewport);
}

bool AVGNode::isVisibleNode()
{
    return true;
}

void AVGNode::setViewport (int x, int y, int width, int height)
{
    if (x == -1) {
        x = m_RelViewport.tl.x;
    }
    if (y == -1) {
        y = m_RelViewport.tl.y;
    }
    if (width == -1) {
        width = m_RelViewport.Width();
    }
    if (height == -1) {
        height = m_RelViewport.Height();
    }
    PLPoint pos = m_AbsViewport.tl-m_RelViewport.tl;
    m_RelViewport = PLRect (x, y, x+width, y+height);
    m_AbsViewport = PLRect (pos+m_RelViewport.tl, pos+m_RelViewport.br);
}

const PLRect& AVGNode::getRelViewport ()
{
    return m_RelViewport;
}

const PLRect& AVGNode::getAbsViewport ()
{
    return m_AbsViewport;
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

double AVGNode::getEffectiveOpacity()
{
    if (getParent()) {
        return m_Opacity*getParent()->getEffectiveOpacity();
    } else {
        return m_Opacity;
    }
}

AVGDFBDisplayEngine * AVGNode::getEngine()
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

bool AVGNode::handleEvent (AVGEvent* pEvent, JSContext * pJSContext)
{
    string Code;
    pEvent->setNode(this);
    int EventType;
    pEvent->GetType(&EventType);
    switch (EventType) {
        case AVGEvent::MOUSEMOVE:
            Code = m_MouseMoveHandler;
            break;
        case AVGEvent::MOUSEDOWN:
            Code = m_MouseButtonDownHandler;
            break;
        case AVGEvent::MOUSEUP:
            Code = m_MouseButtonUpHandler;
            break;
        case AVGEvent::MOUSEOVER:
            Code = m_MouseOverHandler;
            break;
        case AVGEvent::MOUSEOUT:
            Code = m_MouseOutHandler;
            break;
         default:
            break;
    }
    if (!Code.empty()) {
        callJS(Code, pJSContext);
    } else {
        if (m_pParent) {
            m_pParent->handleEvent (pEvent, pJSContext);
        }
    }
}

void AVGNode::callJS (const string& Code, JSContext * pJSContext)
{

    // TODO: Move this to a separate class and precompile.
    AVGJSScript Script(Code, "EventScript", 0, pJSContext);
    Script.run();
}

