//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2011 Ulrich von Zadow
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

void Node::checkSetParentError(DivNodeWeakPtr pParent)
{
    if (getParent() && !!(pParent.lock())) {
        throw(Exception(AVG_ERR_UNSUPPORTED,
                string("Can't change parent of node (") + getID() + ")."));
    }
}

void Node::setParent(DivNodeWeakPtr pParent)
{
    checkSetParentError(pParent);
    m_pParent = pParent;
}

DivNodePtr Node::getParent() const
{
    if (m_pParent.expired()) {
        return DivNodePtr();
    } else {
        return m_pParent.lock();
    }
}

const string& Node::getID() const
{
    return m_ID;
}

void Node::setID(const std::string& id)
{
    m_ID = id;
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

}
