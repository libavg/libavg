//
// $Id$
// 

#include "AVGAVGNode.h"
#include "AVGContainer.h"
#include "AVGVisibleNode.h"
#include "AVGSDLDisplayEngine.h"

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

AVGNode * AVGAVGNode::getElementByPos (const PLPoint & pos)
{
    if (!getAbsViewport().Contains(pos)) {
        return 0; // pos is not in parent.
    }
    for (int i=getNumChildren()-1; i>=0; i--) {
        AVGNode * pFoundNode = getChild(i)->getElementByPos(pos);
        if (pFoundNode) {
            return pFoundNode;
        }
    }
    return this; // pos is in parent, but not in any child.
}

void AVGAVGNode::update (int time, const PLPoint& pos)
{
    AVGVisibleNode::update(time, pos);
    for (int i=0; i<getNumChildren(); i++){
        getChild(i)->update(time, getAbsViewport().tl);
    }
}

void AVGAVGNode::render ()
{
    getEngine()->setClipRect(getAbsViewport());
    for (int i=0; i<getNumChildren(); i++){
        getChild(i)->render();
    }
}

void AVGAVGNode::getDirtyRect ()
{

}

string AVGAVGNode::dump (int indent)
{
    // Messy duplicated code because of multiple inheritance.
    string dumpStr = AVGVisibleNode::dump(indent);
    for (int i=0; i<getNumChildren(); i++){
        dumpStr += getChild(i)->dump(indent+2);
    }

    return dumpStr;
}

string AVGAVGNode::getTypeStr ()
{
    return "AVGAVGNode";
}

