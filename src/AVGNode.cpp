//
// $Id$
// 

#include "AVGEvent.h"
#include "AVGContainer.h"
#include "IJSEvalKruecke.h"

#include <paintlib/plpoint.h>

AVGNode::AVGNode (const std::string& id, AVGContainer * pParent)
    : m_ID(id),
      m_pParent(pParent)
{
}

AVGNode::AVGNode ()
{
}

AVGNode::~AVGNode()
{
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
        default:
            break;
    }
    callJS(Code, pKruecke);
}

void AVGNode::onMouseOver (IJSEvalKruecke * pKruecke)
{
    callJS(m_MouseOverHandler, pKruecke);
}

void AVGNode::onMouseOut (IJSEvalKruecke * pKruecke)
{
    callJS(m_MouseOutHandler, pKruecke);
}

void AVGNode::callJS (const string& Code, IJSEvalKruecke * pKruecke)
{
    if (!Code.empty()) {
        char * pResult;
        pKruecke->CallEval(Code.c_str(), &pResult);
    }
}

