//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  Current versions can be found at www.libavg.de
//

#include "DivNode.h"
#include "Container.h"
#include "DisplayEngine.h"

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

void DivNode::init(DisplayEngine * pEngine, Container * pParent, 
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
