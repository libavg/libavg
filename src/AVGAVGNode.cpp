//
// $Id$
// 

#include "AVGAVGNode.h"
#include "AVGContainer.h"
#include "IAVGDisplayEngine.h"

#include <paintlib/plpoint.h>
#include <nsMemory.h>
#include <xpcom/nsComponentManagerUtils.h>

#include <iostream>

using namespace std;

NS_IMPL_ISUPPORTS1_CI(AVGAVGNode, IAVGNode);

AVGAVGNode * AVGAVGNode::create()
{
    return createNode<AVGAVGNode>("@c-base.org/avgavgnode;1");
}       

AVGAVGNode::AVGAVGNode()
{
}

AVGAVGNode::~AVGAVGNode()
{
}

NS_IMETHODIMP 
AVGAVGNode::GetType(PRInt32 *_retval)
{
    *_retval = NT_AVG;
    return NS_OK;
}

void AVGAVGNode::initKeyEventHandlers (const string& sKeyDownHandler, 
        const string& sKeyUpHandler)
{
    m_sKeyUpHandler = sKeyUpHandler;
    m_sKeyDownHandler = sKeyDownHandler;
}

string AVGAVGNode::getTypeStr ()
{
    return "AVGAVGNode";
}

void AVGAVGNode::handleKeyEvent (AVGKeyEvent* pEvent, JSContext * pJSContext)
{
    string Code;
    int EventType;
    pEvent->GetType(&EventType);
    switch (EventType) {
        case IAVGEvent::KEYDOWN:
            Code = m_sKeyDownHandler;
            break;
        case IAVGEvent::KEYUP:
            Code = m_sKeyUpHandler;
            break;
         default:
            break;
    }
    if (!Code.empty()) {
        callJS(Code, pJSContext);
    } 
}
