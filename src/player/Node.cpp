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
#include "Canvas.h"
#include "DivNode.h"
#include "Player.h"
#include "CursorEvent.h"

#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/ObjectCounter.h"
#include "../base/StringHelper.h"
#include "../base/OSHelper.h"

#include <string>

using namespace std;

namespace avg {

NodeDefinition Node::createDefinition()
{
    return NodeDefinition("node")
        .addArg(Arg<string>("id", "", false, offsetof(Node, m_ID)))
        .addArg(Arg<bool>("active", true, false, offsetof(Node, m_bActive)))
        .addArg(Arg<bool>("sensitive", true, false, offsetof(Node, m_bSensitive)))
        .addArg(Arg<float>("opacity", 1.0, false, offsetof(Node, m_Opacity)));
}

Node::Node()
    : m_pCanvas(),
      m_State(NS_UNCONNECTED)
      
{
    ObjectCounter::get()->incRef(&typeid(*this));
}

Node::~Node()
{
    m_EventHandlerMap.clear();
    ObjectCounter::get()->decRef(&typeid(*this));
}

void Node::setArgs(const ArgList& args)
{
}

void Node::setTypeInfo(const NodeDefinition * pDefinition)
{
    m_pDefinition = pDefinition;
}

void Node::checkSetParentError(DivNodeWeakPtr pParent)
{
    if (getParent() && !!(pParent.lock())) {
        throw(Exception(AVG_ERR_UNSUPPORTED,
                string("Can't change parent of node (") + getID() + ")."));
    }
}

void Node::setParent(DivNodeWeakPtr pParent, NodeState parentState,
        CanvasPtr pCanvas)
{
    AVG_ASSERT(getState() == NS_UNCONNECTED);
    checkSetParentError(pParent);
    m_pParent = pParent;
    if (parentState != NS_UNCONNECTED) {
        connect(pCanvas);
    }
}

void Node::removeParent()
{
    m_pParent = DivNodePtr();
}

DivNodePtr Node::getParent() const
{
    if (m_pParent.expired()) {
        return DivNodePtr();
    } else {
        return m_pParent.lock();
    }
}

vector<NodeWeakPtr> Node::getParentChain()
{
    vector<NodeWeakPtr> pNodes;
    boost::shared_ptr<Node> pCurNode = shared_from_this();
    while (pCurNode) {
        pNodes.push_back(pCurNode);
        pCurNode = pCurNode->getParent();
    }
    return pNodes;
}

void Node::connectDisplay()
{
    AVG_ASSERT(getState() == NS_CONNECTED);
    setState(NS_CANRENDER);
}

void Node::connect(CanvasPtr pCanvas)
{
    m_pCanvas = pCanvas;
    setState(NS_CONNECTED);
}

void Node::disconnect(bool bKill)
{
    AVG_ASSERT(getState() != NS_UNCONNECTED);
    m_pCanvas.lock()->removeNodeID(getID());
    setState(NS_UNCONNECTED);
    if (bKill) {
        m_EventHandlerMap.clear();
    }
}

void Node::unlink(bool bKill)
{
    DivNodePtr pParent = getParent();
    if (pParent != DivNodePtr()) {
        pParent->removeChild(shared_from_this(), bKill);
    }
}

const string& Node::getID() const
{
    return m_ID;
}

void Node::setID(const std::string& id)
{
    if (getState() != NS_UNCONNECTED) {
        throw Exception(AVG_ERR_UNSUPPORTED, "Node with ID "+getID()
                +" is connected. setID invalid.");
    }
    m_ID = id;
}

float Node::getOpacity() const 
{
    return m_Opacity;
}

void Node::setOpacity(float opacity) 
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
    Player::get()->setEventCapture(shared_from_this(), cursorID);
}

void Node::releaseEventCapture(int cursorID) 
{
    Player::get()->releaseEventCapture(cursorID);
}

void Node::setEventHandler(Event::Type type, int sources, PyObject * pFunc)
{
    for (int source = 1; source <= Event::NONE; source *= 2) {
        if (source & sources) {
            EventID id(type, (Event::Source)source);
            EventHandlerMap::iterator it = m_EventHandlerMap.find(id);
            if (it != m_EventHandlerMap.end()) {
                m_EventHandlerMap.erase(it);
            }
            if (pFunc != Py_None) {
                connectOneEventHandler(id, Py_None, pFunc);
            }
        }
    }
}

void Node::connectEventHandler(Event::Type type, int sources, 
        PyObject * pObj, PyObject * pFunc)
{
    for (int source = 1; source <= Event::NONE; source *= 2) {
        if (source & sources) {
            EventID id(type, (Event::Source)source);
            connectOneEventHandler(id, pObj, pFunc);
        }
    }
//    cerr << "connectEventHandler" << endl;
//    dumpEventHandlers();
}

void Node::disconnectEventHandler(PyObject * pObj, PyObject * pFunc)
{
    int numDisconnected = 0;
    EventHandlerMap::iterator it;
    for (it = m_EventHandlerMap.begin(); it != m_EventHandlerMap.end();) {
        EventHandlerArrayPtr pEventHandlers = it->second;
        EventHandlerArray::iterator listIt;
        for (listIt = pEventHandlers->begin(); listIt != pEventHandlers->end();)
        {
            EventHandler& eventHandler = *listIt;
            if (eventHandler.m_pObj == pObj &&
                    (pFunc == 0 || 
                     PyObject_RichCompareBool(eventHandler.m_pMethod, pFunc, Py_EQ)))
            {
                listIt = pEventHandlers->erase(listIt);
                numDisconnected++;
            } else {
                ++listIt;
            }
        }
        
        if (pEventHandlers->empty()) {
            EventHandlerMap::iterator itErase = it;
            ++it;
            m_EventHandlerMap.erase(itErase);
        } else {
            ++it;
        }
    }
//    cerr << "disconnectEventHandler" << endl;
//    dumpEventHandlers();
}

bool Node::reactsToMouseEvents()
{
    return m_bActive && m_bSensitive;
}

glm::vec2 Node::getRelPos(const glm::vec2& absPos) const 
{
    glm::vec2 parentPos;
    if (m_pParent.expired()) {
        parentPos = absPos;
    } else {
        parentPos = m_pParent.lock()->getRelPos(absPos);
    }
    return toLocal(parentPos);
}

glm::vec2 Node::getAbsPos(const glm::vec2& relPos) const 
{
    glm::vec2 thisPos = toGlobal(relPos);
    glm::vec2 parentPos;
    if (m_pParent.expired()) {
        parentPos = thisPos;
    } else {
        parentPos = m_pParent.lock()->getAbsPos(thisPos);
    }
    return parentPos;
}

glm::vec2 Node::toLocal(const glm::vec2& globalPos) const
{
    return globalPos;
}

glm::vec2 Node::toGlobal(const glm::vec2& localPos) const
{
    return localPos;
}

NodePtr Node::getElementByPos(const glm::vec2& pos)
{
    vector<NodeWeakPtr> elements;
    getElementsByPos(pos, elements);
    if (elements.empty()) {
        return NodePtr();
    } else {
        return elements[0].lock();
    }
}

void Node::getElementsByPos(const glm::vec2& pos, 
                vector<NodeWeakPtr>& pElements)
{
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

CanvasPtr Node::getCanvas() const
{
    return m_pCanvas.lock();
}

bool Node::handleEvent(EventPtr pEvent)
{
    EventID id(pEvent->getType(), pEvent->getSource());
    EventHandlerMap::iterator it = m_EventHandlerMap.find(id);
    if (it != m_EventHandlerMap.end()) {
        bool bHandled = false;
        // We need to copy the array because python code in callbacks can 
        /// connect and disconnect event handlers.
        EventHandlerArray eventHandlers = *(it->second);
        EventHandlerArray::iterator listIt;
        for (listIt = eventHandlers.begin(); listIt != eventHandlers.end(); ++listIt) {
            bHandled = callPython(listIt->m_pMethod, pEvent);
        }
        return bHandled;
    } else {
        return false;
    }
}

float Node::getEffectiveOpacity() const
{
    return m_EffectiveOpacity;
}

string Node::dump(int indent)
{
    string dumpStr = string(indent, ' ') + getTypeStr() + ": m_ID=" + getID() +
            "m_Opacity=" + toString(m_Opacity);
    return dumpStr; 
}

void Node::setState(Node::NodeState state)
{
/*    
    cerr << m_ID << " state: ";
    switch(state) {
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
        AVG_ASSERT(state != NS_CANRENDER);
    }
    if (m_State == NS_CANRENDER) {
        AVG_ASSERT(state != NS_CONNECTED);
    }

    m_State = state;
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

void Node::checkReload(const std::string& sHRef, const ImagePtr& pImage,
        Image::TextureCompression comp)
{
    string sLastFilename = pImage->getFilename();
    string sFilename = sHRef;
    initFilename(sFilename);
    if (sLastFilename != sFilename) {
        try {
            sFilename = convertUTF8ToFilename(sFilename);
            if (sHRef == "") {
                pImage->setEmpty();
            } else {
                pImage->setFilename(sFilename, comp);
            }
        } catch (Exception& ex) {
            pImage->setEmpty();
            if (getState() != Node::NS_UNCONNECTED) {
                AVG_TRACE(Logger::ERROR, ex.getStr());
            } else {
                AVG_TRACE(Logger::MEMORY, ex.getStr());
            }
        }
    }
}

bool Node::isVisible() const
{
    return getEffectiveActive() && getEffectiveOpacity() > 0.01;
}

bool Node::getEffectiveActive() const
{
    if (getParent()) {
        return m_bActive && getParent()->getEffectiveActive();
    } else {
        return m_bActive;
    }
}

void Node::connectOneEventHandler(const EventID& id, PyObject * pObj, 
        PyObject * pFunc)
{
    EventHandlerMap::iterator it = m_EventHandlerMap.find(id);
    EventHandlerArrayPtr pEventHandlers;
    if (it == m_EventHandlerMap.end()) {
        pEventHandlers = EventHandlerArrayPtr(new EventHandlerArray);
        m_EventHandlerMap[id] = pEventHandlers;
    } else {
        pEventHandlers = it->second;
    }
    pEventHandlers->push_back(EventHandler(pObj, pFunc));
}

void Node::dumpEventHandlers()
{
    EventHandlerMap::iterator it;
    cerr << "-----" << endl;
    for (it = m_EventHandlerMap.begin(); it != m_EventHandlerMap.end(); ++it) {
        EventID id = it->first;
        EventHandlerArrayPtr pEventHandlers = it->second;
        cerr << "type: " << id.m_Type << ", source: " << id.m_Source << endl;
        EventHandlerArray::iterator listIt;
        for (listIt = pEventHandlers->begin(); listIt != pEventHandlers->end(); ++listIt)
        {
            EventHandler& handler = *listIt;
            cerr << "  " << handler.m_pObj << ", " << handler.m_pMethod
                    << endl;
        }
    }
    cerr << "-----" << endl;
}

PyObject * Node::findPythonFunc(const string& sCode)
{
    if (sCode.empty()) {
        return 0;
    } else {
        PyObject * pModule = PyImport_AddModule("__main__");
        if (!pModule) {
            cerr << "Could not find module __main__." << endl;
            exit(-1);
        }
        PyObject * pDict = PyModule_GetDict(pModule);
        PyObject * pFunc = PyDict_GetItemString(pDict, sCode.c_str());
        if (!pFunc) {
            AVG_TRACE(Logger::ERROR, "Function \"" << sCode << 
                    "\" not defined for node with id '"+getID()+"'. Aborting.");
            exit(-1);
        }
        return pFunc;
    }
}

bool Node::callPython(PyObject * pFunc, EventPtr pEvent)
{
    bool bOk = boost::python::call<bool>(pFunc, pEvent);
    return bOk;
}

bool Node::operator ==(const Node& other) const
{
    return this == &other;
}

bool Node::operator !=(const Node& other) const
{
    return this != &other;
}

long Node::getHash() const
{
    return long(this);
}

const NodeDefinition* Node::getDefinition() const
{
    return m_pDefinition;
}

string Node::getTypeStr() const
{
    return m_pDefinition->getName();
}

Node::EventID::EventID(Event::Type eventType, Event::Source source)
    : m_Type(eventType),
      m_Source(source)
{
}

bool Node::EventID::operator < (const EventID& other) const 
{
    if (m_Type == other.m_Type) {
        return m_Source < other.m_Source;
    } else {
        return m_Type < other.m_Type;
    }
}

Node::EventHandler::EventHandler(PyObject * pObj, PyObject * pMethod)
{
    Py_INCREF(pObj);
    m_pObj = pObj;
    Py_INCREF(pMethod);
    m_pMethod = pMethod;
}

Node::EventHandler::EventHandler(const EventHandler& other)
{
    Py_INCREF(other.m_pObj);
    m_pObj = other.m_pObj;
    Py_INCREF(other.m_pMethod);
    m_pMethod = other.m_pMethod;
}

Node::EventHandler::~EventHandler()
{
    Py_DECREF(m_pObj);
    Py_DECREF(m_pMethod);
}

}
