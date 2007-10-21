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
#include "DisplayEngine.h"
#include "Player.h"

#include "../base/Exception.h"
#include "../base/Logger.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

DivNode::DivNode (const xmlNodePtr xmlNode, Player * pPlayer)
    : Node(xmlNode, pPlayer)
{
    
}

DivNode::~DivNode()
{
}

void DivNode::setDisplayEngine(DisplayEngine * pEngine)
{
    Node::setDisplayEngine(pEngine);
    for  (int i = 0; i< (int)m_Children.size(); ++i) {
        m_Children[i]->setDisplayEngine(pEngine);
    }
}

void DivNode::disconnect()
{
    for  (int i = 0; i< (int)m_Children.size(); ++i) {
        m_Children[i]->disconnect();
    }
    Node::disconnect();
}

int DivNode::getNumChildren ()
{
    return m_Children.size();
}

NodePtr DivNode::getChild (unsigned i)
{
    if (i >= m_Children.size()) {
        stringstream s;
        s << "Index " << i << " is out of range in DivNode::getChild()";
        throw(Exception(AVG_ERR_OUT_OF_RANGE, s.str()));
    }
    return m_Children[i];
}

void DivNode::appendChild (NodePtr pNewNode)
{
    insertChild(pNewNode, m_Children.size());
}

void DivNode::insertChild(NodePtr pNewNode, unsigned i)
{
    if (!pNewNode) {
        throw Exception(AVG_ERR_NO_NODE,
                "insertChild called without a node.");
    }
    if (pNewNode->getState() == NS_CONNECTED) {
        throw(Exception(AVG_ERR_ALREADY_CONNECTED,
                "Can't connect node with id "+pNewNode->getID()+
                ": already connected."));
    }
    if (i>m_Children.size()) {
        throw(Exception(AVG_ERR_OUT_OF_RANGE,
                pNewNode->getID()+"::insertChild: index out of bounds."));
    }
    std::vector<NodePtr>::iterator Pos = m_Children.begin()+i;
    m_Children.insert(Pos, pNewNode);
    DivNodePtr Ptr = boost::dynamic_pointer_cast<DivNode>(getThis());
    pNewNode->setParent(Ptr);
    if (getState() == NS_CONNECTED) {
        getPlayer()->registerNode(pNewNode);
    }
    if (isDisplayAvailable()) {
        pNewNode->setDisplayEngine(getEngine());
    }
}

void DivNode::removeChild (unsigned i)
{
    NodePtr pNode = getChild(i);
    pNode->setParent(DivNodePtr());
    pNode->disconnect();
    m_Children.erase(m_Children.begin()+i);
}

void DivNode::reorderChild(unsigned i, unsigned j)
{
    NodePtr pNode = getChild(i);
    m_Children.erase(m_Children.begin()+i);
    if (i>m_Children.size()) {
        throw(Exception(AVG_ERR_OUT_OF_RANGE,
                pNode->getID()+"::reorderChild: index out of bounds."));
    }
    std::vector<NodePtr>::iterator Pos = m_Children.begin()+j;
    m_Children.insert(Pos, pNode);
}

int DivNode::indexOf(NodePtr pChild)
{
    for  (int i = 0; i< (int)m_Children.size(); ++i) {
        if (m_Children[i] == pChild) {
            return i;
        }
    }
    return -1;
}

NodePtr DivNode::getElementByPos (const DPoint & pos)
{
    if (!getVisibleRect().Contains(pos) || !reactsToMouseEvents()) {
        return NodePtr();
    }
    for (int i=getNumChildren()-1; i>=0; i--) {
        NodePtr pFoundNode = getChild(i)->getElementByPos(pos);
        if (pFoundNode) {
            return pFoundNode;
        }
    }
    return getThis(); // pos is in current node, but not in any child.
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

bool DivNode::obscures (const DRect& rect, int Child)
{
    if (!isActive()) {
        return false;
    }
    for (int i=Child+1; i<getNumChildren(); i++) {
        if (getChild(i)->obscures(rect, -1))
            return true;
    }
    return false;
 
}

string DivNode::getTypeStr ()
{
    return "DivNode";
}
string DivNode::dump (int indent)
{
    string dumpStr = Node::dump () + "\n";
    vector<NodePtr>::iterator it;
    for (it=m_Children.begin(); it<m_Children.end(); it++) {
        dumpStr += (*it)->dump(indent+2)+"\n";
    }
    return dumpStr;
}

DPoint DivNode::getPreferredMediaSize()
{
    return DPoint(10000,10000);
}

}
