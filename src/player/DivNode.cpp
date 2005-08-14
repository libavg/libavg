//
// $Id$
// 

#include "DivNode.h"
#include "Container.h"
#include "IDisplayEngine.h"

#include <iostream>

using namespace std;

namespace avg {

DivNode::DivNode()
{
}

DivNode::DivNode (const xmlNodePtr xmlNode, Container * pParent)
    : Container(xmlNode, pParent)
{
    
}

DivNode::~DivNode()
{
}

void DivNode::init(IDisplayEngine * pEngine, Container * pParent, 
        Player * pPlayer)
{
    Node::init(pEngine, pParent, pPlayer);
    Node::initVisible();
}

Node * DivNode::getElementByPos (const DPoint & pos)
{
    if (!getVisibleRect().Contains(pos)) {
        return 0; // pos is not in parent.
    }
    for (int i=getNumChildren()-1; i>=0; i--) {
        Node * pFoundNode = getChild(i)->getElementByPos(pos);
        if (pFoundNode) {
            return pFoundNode;
        }
    }
    return this; // pos is in current node, but not in any child.
}

void DivNode::prepareRender (int time, const DRect& parent)
{
    Node::prepareRender(time, parent);
    for (int i=0; i<getNumChildren(); i++){
        getChild(i)->prepareRender(time, getAbsViewport());
    }
}

void DivNode::render(const DRect& rect)
{
    for (int i=0; i<getNumChildren(); i++) {
        getChild(i)->maybeRender(rect);
    }
}

bool DivNode::obscures (const DRect& rect, int z)
{
    if (!isActive()) {
        return false;
    }
    for (int i=0; i<getNumChildren(); i++){
        if (getChild(i)->getZ() > z && getChild(i)->obscures(rect, 0))
            return true;
    }
    return false;
 
}

void DivNode::getDirtyRegion (Region& DirtyRegion)
{
    for (int i=0; i<getNumChildren(); i++){
        Region ChildRegion;
        getChild(i)->getDirtyRegion(ChildRegion);
        DirtyRegion.addRegion(ChildRegion);
    }
    Region myRegion;
    Node::getDirtyRegion(myRegion);
    DirtyRegion.addRegion(myRegion);
}

string DivNode::getTypeStr ()
{
    return "DivNode";
}

}
