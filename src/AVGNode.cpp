//
// $Id$
// 

#include "AVGEvent.h"
#include "AVGContainer.h"
#include "IJSEvalKruecke.h"

#include <paintlib/plpoint.h>

#include <xpcom/nsMemory.h>

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
    cerr << "Error: Request for unknown int attribute " << name << endl;
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP 
AVGNode::GetFloatAttr(const char *name, float *_retval)
{
    cerr << "Error: Request for unknown float attribute " << name << endl;
    return NS_ERROR_NOT_IMPLEMENTED;
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
    cerr << "Error: Setting unknown int attribute " << name << endl;
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP 
AVGNode::SetFloatAttr(const char *name, float value)
{
    cerr << "Error: Setting unknown float attribute " << name << endl;
    return NS_ERROR_NOT_IMPLEMENTED;
}

void AVGNode::init(const string& id, AVGContainer * pParent)
{
    m_ID = id;
    m_pParent = pParent;
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
    return 0;
}

void AVGNode::update (int time, const PLPoint& pos)
{
}

void AVGNode::render ()
{
}

void AVGNode::getDirtyRect ()
{
}

string AVGNode::dump (int indent)
{
    
    return string(indent, ' ') + getTypeStr() + ": m_ID= " + m_ID; 
}

string AVGNode::getTypeStr ()
{
    return "AVGNode";
}

const string& AVGNode::getID ()
{
    return m_ID;
}

void AVGNode::handleEvent (AVGEvent* pEvent, IJSEvalKruecke * pKruecke)
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
    callJS(Code, pKruecke);
}

void AVGNode::callJS (const string& Code, IJSEvalKruecke * pKruecke)
{
    if (!Code.empty()) {
        char * pResult;
        pKruecke->CallEval(Code.c_str(), &pResult);
    }
}

