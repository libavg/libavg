//
// $Id$
// 

#include "AVGAVGNode.h"
#include "AVGContainer.h"
#include "AVGDFBDisplayEngine.h"

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
    return this; // pos is in current node, but not in any child.
}

void AVGAVGNode::prepareRender (int time, const PLRect& parent)
{
    AVGNode::prepareRender(time, parent);
    for (int i=0; i<getNumChildren(); i++){
        getChild(i)->prepareRender(time, getAbsViewport());
    }
}

void AVGAVGNode::render(const PLRect& rect)
{
    for (int i=0; i<getNumChildren(); i++) {
        getChild(i)->maybeRender(rect);
    }
}

bool AVGAVGNode::obscures (const PLRect& rect, int z)
{
    for (int i=0; i<getNumChildren(); i++){
        if (getChild(i)->getZ() > z && getChild(i)->obscures(rect, 0))
            return true;
    }
    return false;
 
}

void AVGAVGNode::getDirtyRegion (AVGRegion& Region)
{
    for (int i=0; i<getNumChildren(); i++){
        AVGRegion ChildRegion;
        getChild(i)->getDirtyRegion(ChildRegion);
        Region.addRegion(ChildRegion);
    }
    AVGRegion myRegion;
    AVGNode::getDirtyRegion(myRegion);
    Region.addRegion(myRegion);
}

string AVGAVGNode::getTypeStr ()
{
    return "AVGAVGNode";
}


