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
#include "SDLDisplayEngine.h"
#include "Arg.h"
#include "Image.h"

#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/XMLHelper.h"
#include "../base/StringHelper.h"

#include <Magick++.h>

#include <iostream>

using namespace std;

namespace avg {

NodeDefinition Node::createDefinition()
{
    return NodeDefinition("node")
        .addArg(Arg<string>("id", "", false, offsetof(Node, m_ID)))
        .addArg(Arg<string>("oncursormove", ""))
        .addArg(Arg<string>("oncursorup", ""))
        .addArg(Arg<string>("oncursordown", ""))
        .addArg(Arg<string>("oncursorover", ""))
        .addArg(Arg<string>("oncursorout", ""))
        .addArg(Arg<bool>("active", true, false, offsetof(Node, m_bActive)))
        .addArg(Arg<bool>("sensitive", true, false, offsetof(Node, m_bSensitive)))
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
    EventHandlerMap::iterator it;
    for (it=m_EventHandlerMap.begin(); it != m_EventHandlerMap.end(); ++it) {
        Py_DECREF(it->second);
    }
}

void Node::setArgs(const ArgList& Args)
{
    addEventHandlers(Event::CURSORMOTION, Args.getArgVal<string> ("oncursormove"));
    addEventHandlers(Event::CURSORUP, Args.getArgVal<string> ("oncursorup"));
    addEventHandlers(Event::CURSORDOWN, Args.getArgVal<string> ("oncursordown"));
    addEventHandlers(Event::CURSOROVER, Args.getArgVal<string> ("oncursorover"));
    addEventHandlers(Event::CURSOROUT, Args.getArgVal<string> ("oncursorout"));
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
    m_pDisplayEngine = dynamic_cast<SDLDisplayEngine*>(pDisplayEngine);
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

bool Node::getActive() const 
{
    return m_bActive;
}

void Node::setActive(bool bActive)
{
    if (bActive != m_bActive) {
        m_bActive = bActive;
    }
}

bool Node::getSensitive() const 
{
    return m_bSensitive;
}

void Node::setSensitive(bool bSensitive)
{
    m_bSensitive = bSensitive;
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

void Node::setMouseEventCapture()
{
    setEventCapture(MOUSECURSORID);
}

void Node::releaseMouseEventCapture()
{
    releaseEventCapture(MOUSECURSORID);
}

void Node::setEventCapture(int cursorID) 
{
    Player::get()->setEventCapture(getThis(), cursorID);
}

void Node::releaseEventCapture(int cursorID) 
{
    Player::get()->releaseEventCapture(cursorID);
}

void Node::setEventHandler(Event::Type Type, int Sources, PyObject * pFunc)
{
    for (int i=0; i<4; ++i) {
        int source = int(pow(2.,i));
        if (source & Sources) {
            EventHandlerID ID(Type, (Event::Source)source);
            EventHandlerMap::iterator it = m_EventHandlerMap.find(ID);
            if (it != m_EventHandlerMap.end()) {
                Py_DECREF(it->second);
                m_EventHandlerMap.erase(it);
            }
            if (pFunc != Py_None) {
                Py_INCREF(pFunc);
                m_EventHandlerMap[ID] = pFunc;
            }
        }
    }
}

bool Node::reactsToMouseEvents()
{
    return m_bActive && m_bSensitive;
}

NodePtr Node::getElementByPos(const DPoint & pos)
{
    return NodePtr();
}

void Node::preRender()
{
    if (getParent()) {
        m_EffectiveOpacity = m_Opacity*getParent()->getEffectiveOpacity();
    } else {
        m_EffectiveOpacity = m_Opacity;
    }
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

bool Node::handleEvent(EventPtr pEvent)
{
    EventHandlerID ID(pEvent->getType(), pEvent->getSource());
    EventHandlerMap::iterator it = m_EventHandlerMap.find(ID);
    if (it!=m_EventHandlerMap.end()) {
        return callPython(it->second, pEvent);
    } else {
        return false;
    }
}

void Node::addEventHandlers(Event::Type EventType, const string& Code)
{
    addEventHandler(EventType, Event::MOUSE, Code);
    addEventHandler(EventType, Event::TOUCH, Code);
    addEventHandler(EventType, Event::TRACK, Code);
}

void Node::addEventHandler(Event::Type EventType, Event::Source Source, 
        const string& Code)
{
    PyObject * pFunc = findPythonFunc(Code);
    if (pFunc) {
        Py_INCREF(pFunc);
        EventHandlerID ID(EventType, Source);
        m_EventHandlerMap[ID] = pFunc;
    }
}

SDLDisplayEngine * Node::getDisplayEngine() const
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
    return m_EffectiveOpacity;
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
    if (sFilename != "") {
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

void Node::checkReload(const std::string& sHRef, ImagePtr& pImage)
{
    string sLastFilename = pImage->getFilename();
    string sFilename = sHRef;
    initFilename(sFilename);
    if (sLastFilename != sFilename) {
        try {
            pImage->setFilename(sFilename);
        } catch (Magick::Exception & ex) {
            pImage->setFilename("");
            if (getState() == Node::NS_CONNECTED) {
                AVG_TRACE(Logger::ERROR, ex.what());
            } else {
                AVG_TRACE(Logger::MEMORY, ex.what());
            }
        }
    }
}

bool Node::callPython(PyObject * pFunc, EventPtr pEvent)
{
    return boost::python::call<bool>(pFunc, pEvent);
}

PyObject * Node::findPythonFunc(const string& Code)
{
    if (Code.empty()) {
        return 0;
    } else {
        PyObject * pModule = PyImport_AddModule("__main__");
        if (!pModule) {
            cerr << "Could not find module __main__." << endl;
            exit(-1);
        }
        PyObject * pDict = PyModule_GetDict(pModule);
        PyObject * pFunc = PyDict_GetItemString(pDict, Code.c_str());
        if (!pFunc) {
            AVG_TRACE(Logger::ERROR, "Function \"" << Code << 
                    "\" not defined for node with id '"+getID()+"'. Aborting.");
            exit(-1);
        }
        return pFunc;
    }
}

Node::EventHandlerID::EventHandlerID(Event::Type EventType, Event::Source Source)
    : m_Type(EventType),
      m_Source(Source)
{
}

bool Node::EventHandlerID::operator < (const EventHandlerID& other) const 
{
    if (m_Type == other.m_Type) {
        return m_Source < other.m_Source;
    } else {
        return m_Type < other.m_Type;
    }
}

}
