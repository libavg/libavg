//
// $Id$
// 

#include "AVGDivNode.h"
#include "AVGContainer.h"
#include "IAVGDisplayEngine.h"

#include <paintlib/plpoint.h>
#include <nsMemory.h>
#include <xpcom/nsComponentManagerUtils.h>

#include <iostream>

using namespace std;

NS_IMPL_ISUPPORTS1_CI(AVGDivNode, IAVGNode);

AVGDivNode * AVGDivNode::create()
{
    return createNode<AVGDivNode>("@c-base.org/avgdivnode;1");
}       

AVGDivNode::AVGDivNode()
{
}

AVGDivNode::~AVGDivNode()
{
}

NS_IMETHODIMP 
AVGDivNode::GetType(PRInt32 *_retval)
{
    *_retval = NT_DIV;
    return NS_OK;
}

AVGNode * AVGDivNode::getElementByPos (const AVGPoint<double> & pos)
{
    if (!getVisibleRect().Contains(pos)) {
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

void AVGDivNode::prepareRender (int time, const AVGRect<double>& parent)
{
    AVGNode::prepareRender(time, parent);
    for (int i=0; i<getNumChildren(); i++){
        getChild(i)->prepareRender(time, getAbsViewport());
    }
}

void AVGDivNode::render(const AVGRect<double>& rect)
{
    for (int i=0; i<getNumChildren(); i++) {
        getChild(i)->maybeRender(rect);
    }
}

bool AVGDivNode::obscures (const AVGRect<double>& rect, int z)
{
    for (int i=0; i<getNumChildren(); i++){
        if (getChild(i)->getZ() > z && getChild(i)->obscures(rect, 0))
            return true;
    }
    return false;
 
}

void AVGDivNode::getDirtyRegion (AVGRegion& Region)
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

string AVGDivNode::getTypeStr ()
{
    return "AVGDivNode";
}


