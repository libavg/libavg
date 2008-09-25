//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2008 Ulrich von Zadow
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

#include "GroupNode.h"
#include "DisplayEngine.h"
#include "Player.h"
#include "NodeDefinition.h"

#include "../base/Point.h"
#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/StringHelper.h"

#include <iostream>
#include <sstream>

using namespace std;
using namespace boost;

namespace avg {

NodeDefinition GroupNode::createDefinition()
{
    return NodeDefinition("group")
        .extendDefinition(AreaNode::createDefinition())
        .addArg(Arg<bool>("crop", true, false, offsetof(GroupNode, m_bCrop)));
}

GroupNode::GroupNode()
    : AreaNode()
{
}

GroupNode::~GroupNode()
{
}

void GroupNode::setRenderingEngines(DisplayEngine * pDisplayEngine, 
        AudioEngine * pAudioEngine)
{
    AreaNode::setRenderingEngines(pDisplayEngine, pAudioEngine);
    for  (int i = 0; i<(int)m_Children.size(); ++i) {
        m_Children[i]->setRenderingEngines(pDisplayEngine, pAudioEngine);
    }
}

void GroupNode::connect()
{
    AreaNode::connect();
    for (int i = 0; i< (int)m_Children.size(); ++i) {
        m_Children[i]->connect();
    }
}

void GroupNode::disconnect()
{
    for  (int i = 0; i< (int)m_Children.size(); ++i) {
        m_Children[i]->disconnect();
    }
    AreaNode::disconnect();
}

bool GroupNode::getCrop() const
{
    return m_bCrop;
}

void GroupNode::setCrop(bool bCrop)
{
    m_bCrop = bCrop;
}

int GroupNode::getNumChildren()
{
    return int(m_Children.size());
}

NodePtr GroupNode::getChild(unsigned i)
{
    if (i >= m_Children.size()) {
        stringstream s;
        s << "Index " << i << " is out of range in GroupNode::getChild()";
        throw(Exception(AVG_ERR_OUT_OF_RANGE, s.str()));
    }
    return m_Children[i];
}

void GroupNode::appendChild(NodePtr pNewNode)
{
    insertChild(pNewNode, unsigned(m_Children.size()));
}

void GroupNode::insertChildBefore(NodePtr pNewNode, NodePtr pOldChild)
{
    if (!pOldChild) {
        throw Exception(AVG_ERR_NO_NODE,
                getID()+"::insertChildBefore called without a node.");
    }
    unsigned i = indexOf(pOldChild);
    insertChild(pNewNode, i);
}


void GroupNode::insertChild(NodePtr pNewNode, unsigned i)
{
    if (!pNewNode) {
        throw Exception(AVG_ERR_NO_NODE,
                getID()+"::insertChild called without a node.");
    }
    if (!isChildTypeAllowed(pNewNode->getTypeStr())) {
        throw(Exception(AVG_ERR_ALREADY_CONNECTED,
                "Can't insert a node of type "+pNewNode->getTypeStr()+
                " into a node of type "+getTypeStr()+"."));

    }
    if (pNewNode->getState() == NS_CONNECTED || pNewNode->getState() == NS_CANRENDER) 
    {
        throw(Exception(AVG_ERR_ALREADY_CONNECTED,
                "Can't connect node with id "+pNewNode->getID()+
                ": already connected."));
    }
    if (i>m_Children.size()) {
        throw(Exception(AVG_ERR_OUT_OF_RANGE,
                pNewNode->getID()+"::insertChild: index out of bounds."));
    }
    std::vector<NodePtr>::iterator Pos = m_Children.begin()+i;
    if (getState() == NS_CONNECTED || getState() == NS_CANRENDER) {
        Player::get()->registerNode(pNewNode);
    }
    m_Children.insert(Pos, pNewNode);
    GroupNodePtr Ptr = boost::dynamic_pointer_cast<GroupNode>(getThis());           
    pNewNode->setParent(Ptr, getState());
    if (getState() == NS_CANRENDER) {
        pNewNode->setRenderingEngines(getDisplayEngine(), getAudioEngine());
    }
    childrenChanged();
}

void GroupNode::removeChild(NodePtr pNode)
{
    int i = indexOf(pNode);
    pNode->removeParent();
    m_Children.erase(m_Children.begin()+i);
    childrenChanged();
}

void GroupNode::removeChild(unsigned i)
{
    if (i>m_Children.size()-1) {
        throw(Exception(AVG_ERR_OUT_OF_RANGE,
                getID()+"::removeChild: index "+toString(i)+" out of bounds."));
    }
    NodePtr pNode = getChild(i);
    pNode->removeParent();
    m_Children.erase(m_Children.begin()+i);
    childrenChanged();
}

void GroupNode::reorderChild(NodePtr pNode, unsigned j)
{
    if (j > m_Children.size()-1) {
        throw(Exception(AVG_ERR_OUT_OF_RANGE,
                getID()+"::reorderChild: index "+toString(j)+" out of bounds."));
    }
    int i = indexOf(pNode);
    m_Children.erase(m_Children.begin()+i);
    std::vector<NodePtr>::iterator Pos = m_Children.begin()+j;
    m_Children.insert(Pos, pNode);
    childrenChanged();
}

void GroupNode::reorderChild(unsigned i, unsigned j)
{
    if (i>m_Children.size()-1 || j > m_Children.size()-1) {
        throw(Exception(AVG_ERR_OUT_OF_RANGE,
                getID()+"::reorderChild: index out of bounds."));
    }
    NodePtr pNode = getChild(i);
    m_Children.erase(m_Children.begin()+i);
    std::vector<NodePtr>::iterator Pos = m_Children.begin()+j;
    m_Children.insert(Pos, pNode);
    childrenChanged();
}

int GroupNode::indexOf(NodePtr pChild)
{
    if (!pChild) {
        throw Exception(AVG_ERR_NO_NODE,
                getID()+"::indexOf called without a node.");
    }
    for  (int i = 0; i< (int)m_Children.size(); ++i) {
        if (m_Children[i] == pChild) {
            return i;
        }
    }
    throw(Exception(AVG_ERR_OUT_OF_RANGE,
            "indexOf: node '"+pChild->getID()+"' is not a child of node '"
            +getID()+"'"));
}

string GroupNode::dump (int indent)
{
    string dumpStr = AreaNode::dump () + "\n";
    vector<NodePtr>::iterator it;
    for (it=m_Children.begin(); it<m_Children.end(); it++) {
        dumpStr += (*it)->dump(indent+2)+"\n";
    }
    return dumpStr;
}

IntPoint GroupNode::getMediaSize()
{
    return IntPoint(10000,10000);
}

bool GroupNode::isChildTypeAllowed(const string& sType)
{
    return getDefinition()->isChildAllowed(sType);
}

}
