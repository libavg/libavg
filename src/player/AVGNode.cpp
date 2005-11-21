//
// $Id$
// 

#include "AVGNode.h"

#include "../base/XMLHelper.h"

#include <iostream>

using namespace std;

namespace avg {

AVGNode::AVGNode()
    : m_bEnableCrop(true)
{
}

AVGNode::AVGNode (const xmlNodePtr xmlNode)
    : DivNode(xmlNode, 0)
{
    m_bEnableCrop = getDefaultedBoolAttr (xmlNode, "enablecrop", true);
    m_sKeyUpHandler = getDefaultedStringAttr (xmlNode, "onkeyup", "");
    m_sKeyDownHandler = getDefaultedStringAttr (xmlNode, "onkeydown", "");
}

AVGNode::~AVGNode()
{
}

string AVGNode::getTypeStr ()
{
    return "AVGNode";
}

void AVGNode::handleKeyEvent (KeyEvent* pEvent)
{
    string Code;
    int EventType = pEvent->getType();
    switch (EventType) {
        case Event::KEYDOWN:
            Code = m_sKeyDownHandler;
            break;
        case Event::KEYUP:
            Code = m_sKeyUpHandler;
            break;
         default:
            break;
    }
    if (!Code.empty()) {
        callPython(Code, *pEvent);
    } 
}

bool AVGNode::getCropSetting() {
    return m_bEnableCrop;
}

}
