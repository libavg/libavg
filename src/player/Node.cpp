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
#include "DivNode.h"
#include "Player.h"
#include "DisplayEngine.h"
#include "Arg.h"

#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/XMLHelper.h"
#include "../base/StringHelper.h"

#include <iostream>

using namespace std;

namespace avg {

NodeDefinition Node::createDefinition()
{
    return NodeDefinition("node")
        .addArg(Arg<string>("id", "", false, offsetof(Node, m_ID)))
        .addArg(Arg<double>("opacity", 1.0, false, offsetof(Node, m_Opacity)));
}

Node::Node ()
    : m_pParent(),
      m_This(),
      m_pDisplayEngine(0),
      m_pAudioEngine(0),
      m_State(NS_UNCONNECTED)
{
}

Node::~Node()
{
}

void Node::setThis(NodeWeakPtr This, const NodeDefinition * pDefinition)
{
    m_This = This;
    m_pDefinition = pDefinition;
}

void Node::setParent(DivNodeWeakPtr pParent, NodeState parentState)
{
    assert(getState() == NS_UNCONNECTED);
    if (getParent() && !!(pParent.lock())) {
        throw(Exception(AVG_ERR_UNSUPPORTED, 
                string("Can't change parent of node (") + m_ID + ")."));
    }
    m_pParent = pParent;
    if (parentState != NS_UNCONNECTED) {
        connect();
    }
}

void Node::removeParent()
{
    m_pParent = DivNodePtr();
    if (getState() != NS_UNCONNECTED) {
        disconnect();
    }
}

void Node::setRenderingEngines(DisplayEngine * pDisplayEngine, AudioEngine * pAudioEngine)
{
    assert(getState() == NS_CONNECTED);
    m_pDisplayEngine = pDisplayEngine;
    m_pAudioEngine = pAudioEngine;
    setState(NS_CANRENDER);
}

void Node::connect()
{
    setState(NS_CONNECTED);
}

void Node::disconnect()
{
    assert(getState() != NS_UNCONNECTED);
    if (getState() == NS_CANRENDER) {
        m_pDisplayEngine = 0;
        m_pAudioEngine = 0;
    }
    Player::get()->removeNodeID(m_ID);
    setState(NS_UNCONNECTED);
}

const string& Node::getID() const
{
    return m_ID;
}

void Node::setID(const std::string& ID)
{
    if (getState() != NS_UNCONNECTED) {
        throw(Exception(AVG_ERR_UNSUPPORTED, "Node with ID "+m_ID
                +" is connected. setID invalid."));
    }
    m_ID = ID;
}

double Node::getOpacity() const 
{
    return m_Opacity;
}

void Node::setOpacity(double opacity) 
{
    m_Opacity = opacity;
    if (m_Opacity < 0.0) {
        m_Opacity = 0.0;
    } else if (m_Opacity > 1.0) {
        m_Opacity = 1.0;
    }
}

DivNodePtr Node::getParent() const
{
    if (m_pParent.expired()) {
        return DivNodePtr();
    } else {
        return m_pParent.lock();
    }
}

void Node::unlink()
{
    if (m_pParent.expired()) {
        throw(Exception(AVG_ERR_UNSUPPORTED, "Node with ID "+m_ID
                +" has no parent. unlink invalid."));
    }
    DivNodePtr pParent = m_pParent.lock();
    pParent->removeChild(pParent->indexOf(getThis()));
}

Node::NodeState Node::getState() const
{
    return m_State;
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

DisplayEngine * Node::getDisplayEngine() const
{
    return m_pDisplayEngine;
}

AudioEngine * Node::getAudioEngine() const
{
    return m_pAudioEngine;
}

NodePtr Node::getThis() const
{
    return m_This.lock();
}

double Node::getEffectiveOpacity()
{
    if (getParent()) {
        return m_Opacity*getParent()->getEffectiveOpacity();
    } else {
        return m_Opacity;
    }
}

string Node::dump(int indent)
{
    string dumpStr = string(indent, ' ') + getTypeStr() + ": m_ID=" + m_ID + 
            "m_Opacity=" + toString(m_Opacity);
    return dumpStr; 
}

string Node::getTypeStr() const 
{
    return m_pDefinition->getName();
}

void Node::setState(Node::NodeState State)
{
/*    
    cerr << m_ID << " state: ";
    switch(State) {
        case NS_UNCONNECTED:
            cerr << "unconnected" << endl;
            break;
        case NS_CONNECTED:
            cerr << "connected" << endl;
            break;
        case NS_CANRENDER:
            cerr << "canrender" << endl;
            break;
    }
*/
    if (m_State == NS_UNCONNECTED) {
        assert(State != NS_CANRENDER);
    }
    if (m_State == NS_CANRENDER) {
        assert(State != NS_CONNECTED);
    }

    m_State = State;
}
        
void Node::initFilename(string& sFilename)
{
    bool bAbsDir = sFilename[0] == '/';
#ifdef _WIN32
    if (!bAbsDir) {
        bAbsDir = (sFilename[0] == '\\' || sFilename[1] == ':');
    }
#endif
    if (!bAbsDir) {
        DivNodePtr pParent = getParent();
        if (!pParent) {
            sFilename = Player::get()->getRootMediaDir()+sFilename;
        } else {
            sFilename = pParent->getEffectiveMediaDir()+sFilename;
        }
    }
}

}
