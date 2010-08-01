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

#include "Node.h"

#include "NodeDefinition.h"
#include "Arg.h"

#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/ObjectCounter.h"
#include "../base/StringHelper.h"

#include <string>

using namespace std;

namespace avg {

NodeDefinition Node::createDefinition()
{
    return NodeDefinition("node")
        .addArg(Arg<string>("id", "", false, offsetof(Node, m_ID)));
}

Node::Node()
    : m_This()
{
    ObjectCounter::get()->incRef(&typeid(*this));
}

Node::~Node()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void Node::setThis(NodeWeakPtr This, const NodeDefinition * pDefinition)
{
    m_This = This;
    m_pDefinition = pDefinition;
}

void Node::checkSetParentError(NodeWeakPtr pParent)
{
    if (getParent() && !!(pParent.lock())) {
        throw(Exception(AVG_ERR_UNSUPPORTED,
                string("Can't change parent of node (") + getID() + ")."));
    }
}

void Node::setParent(NodeWeakPtr pParent)
{
    checkSetParentError(pParent);
    m_pParent = pParent;
}

NodePtr Node::getParent() const
{
    if (m_pParent.expired()) {
        return NodePtr();
    } else {
        return m_pParent.lock();
    }
}

unsigned Node::getNumChildren()
{
    return m_Children.size();
}

const NodePtr& Node::getChild(unsigned i)
{
    if (i >= m_Children.size()) {
        stringstream s;
        s << "Index " << i << " is out of range in Node::getChild()";
        throw(Exception(AVG_ERR_OUT_OF_RANGE, s.str()));
    }
    return m_Children[i];
}

void Node::appendChild(NodePtr pNewNode)
{
    insertChild(pNewNode, unsigned(m_Children.size()));
}

void Node::insertChildBefore(NodePtr pNewNode, NodePtr pOldChild)
{
    if (!pOldChild) {
        throw Exception(AVG_ERR_NO_NODE,
                getID()+"::insertChildBefore called without a node.");
    }
    unsigned i = indexOf(pOldChild);
    insertChild(pNewNode, i);
}

void Node::insertChildAfter(NodePtr pNewNode, NodePtr pOldChild)
{
    if (!pOldChild) {
        throw Exception(AVG_ERR_NO_NODE,
                getID()+"::insertChildBefore called without a node.");
    }
    unsigned i = indexOf(pOldChild);
    insertChild(pNewNode, i+1);
}

void Node::insertChild(NodePtr pNewNode, unsigned i)
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
    if (i>m_Children.size()) {
        throw(Exception(AVG_ERR_OUT_OF_RANGE,
                pNewNode->getID()+"::insertChild: index out of bounds."));
    }
    std::vector<NodePtr>::iterator Pos = m_Children.begin()+i;
    m_Children.insert(Pos, pNewNode);
}

void Node::eraseChild(NodePtr pNode)
{
    unsigned i = indexOf(pNode);
    eraseChild(i);
}

void Node::eraseChild(unsigned i)
{
    if (i>m_Children.size()-1) {
        throw(Exception(AVG_ERR_OUT_OF_RANGE,
                getID()+"::removeChild: index "+toString(i)+" out of bounds."));
    }
    m_Children.erase(m_Children.begin()+i);
}

void Node::reorderChild(NodePtr pNode, unsigned j)
{
    if (j > m_Children.size()-1) {
        throw(Exception(AVG_ERR_OUT_OF_RANGE,
                getID()+"::reorderChild: index "+toString(j)+" out of bounds."));
    }
    int i = indexOf(pNode);
    m_Children.erase(m_Children.begin()+i);
    std::vector<NodePtr>::iterator Pos = m_Children.begin()+j;
    m_Children.insert(Pos, pNode);
}

void Node::reorderChild(unsigned i, unsigned j)
{
    if (i>m_Children.size()-1 || j > m_Children.size()-1) {
        throw(Exception(AVG_ERR_OUT_OF_RANGE,
                getID()+"::reorderChild: index out of bounds."));
    }
    NodePtr pNode = getChild(i);
    m_Children.erase(m_Children.begin()+i);
    std::vector<NodePtr>::iterator Pos = m_Children.begin()+j;
    m_Children.insert(Pos, pNode);
}

unsigned Node::indexOf(NodePtr pChild)
{
    if (!pChild) {
        throw Exception(AVG_ERR_NO_NODE,
                getID()+"::indexOf called without a node.");
    }
    for (unsigned i = 0; i<m_Children.size(); ++i) {
        if (m_Children[i] == pChild) {
            return i;
        }
    }
    throw(Exception(AVG_ERR_OUT_OF_RANGE,
            "indexOf: node '"+pChild->getID()+"' is not a child of node '"
            +getID()+"'"));
}

const string& Node::getID() const
{
    return m_ID;
}

void Node::setID(const std::string& ID)
{
    m_ID = ID;
}

bool Node::operator ==(const Node& other) const
{
    return m_This.lock() == other.m_This.lock();
}

bool Node::operator !=(const Node& other) const
{
    return m_This.lock() != other.m_This.lock();
}

long Node::getHash() const
{
    return long(&*m_This.lock());
}

const NodeDefinition* Node::getDefinition() const
{
    return m_pDefinition;
}

NodePtr Node::getThis() const
{
    return m_This.lock();
}

string Node::getTypeStr() const
{
    return m_pDefinition->getName();
}

bool Node::isChildTypeAllowed(const string& sType)
{
    return getDefinition()->isChildAllowed(sType);
}

}
