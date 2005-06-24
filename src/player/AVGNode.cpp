//
// $Id$
// 

#include "AVGNode.h"
#include "IDisplayEngine.h"

#include "../base/XMLHelper.h"

#include <paintlib/plpoint.h>

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
}

AVGNode::~AVGNode()
{
}

string AVGNode::getTypeStr ()
{
    return "AVGNode";
}
/*
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
        callJS(Code, pJSContext);
    } 
}
*/
bool AVGNode::getCropSetting() {
    return m_bEnableCrop;
}

}
