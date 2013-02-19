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

#include "TypeDefinition.h"
#include "Arg.h"
#include "Canvas.h"
#include "DivNode.h"
#include "Player.h"
#include "CursorEvent.h"
#include "PublisherDefinition.h"

#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/ObjectCounter.h"
#include "../base/StringHelper.h"
#include "../base/OSHelper.h"

#include <string>

using namespace std;
using namespace boost;

namespace avg {

void Node::registerType()
{
    PublisherDefinitionPtr pPubDef = PublisherDefinition::create("Node");
    pPubDef->addMessage("CURSOR_DOWN");
    pPubDef->addMessage("CURSOR_MOTION");
    pPubDef->addMessage("CURSOR_UP");
    pPubDef->addMessage("CURSOR_OVER");
    pPubDef->addMessage("CURSOR_OUT");
    pPubDef->addMessage("HOVER_DOWN");
    pPubDef->addMessage("HOVER_MOTION");
    pPubDef->addMessage("HOVER_UP");
    pPubDef->addMessage("HOVER_OVER");
    pPubDef->addMessage("HOVER_OUT");
    pPubDef->addMessage("END_OF_FILE");
    pPubDef->addMessage("SIZE_CHANGED");

    TypeDefinition def = TypeDefinition("node")
        .addArg(Arg<string>("id", "", false, offsetof(Node, m_ID)))
        .addArg(Arg<bool>("active", true, false, offsetof(Node, m_bActive)))
        .addArg(Arg<bool>("sensitive", true, false, offsetof(Node, m_bSensitive)))
        .addArg(Arg<float>("opacity", 1.0, false, offsetof(Node, m_Opacity)));
    TypeRegistry::get()->registerType(def);
}

Node::Node(const std::string& sPublisherName)
    : Publisher(sPublisherName),
      m_pParent(0),
      m_pCanvas(),
      m_State(NS_UNCONNECTED)
{
    ObjectCounter::get()->incRef(&typeid(*this));
}

Node::~Node()
{
    m_EventHandlerMap.clear();
    ObjectCounter::get()->decRef(&typeid(*this));
}

void Node::registerInstance(PyObject* pSelf, const DivNodePtr& pParent)
{
    ExportedObject::registerInstance(pSelf);
    if (pParent) {
        pParent->appendChild(getSharedThis());
    }
}

void Node::checkSetParentError(DivNode* pParent)
{
    if (getParent() && pParent != 0) {
        throw(Exception(AVG_ERR_UNSUPPORTED,
                string("Can't change parent of node (") + getID() + ")."));
    }
    if (getSharedThis() == NodePtr()) {
        throw(Exception(AVG_ERR_UNSUPPORTED, 
                "Node not registered. Please use Node.registerInstance() when deriving from libavg Nodes in python."));
    }
}

void Node::setParent(DivNode* pParent, NodeState parentState, CanvasPtr pCanvas)
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
    m_pParent = 0;
}

DivNodePtr Node::getParent() const
{
    if (m_pParent == 0) {
        return DivNodePtr();
    } else {
        NodePtr pParent = m_pParent->getSharedThis();
        return boost::dynamic_pointer_cast<DivNode>(pParent);
    }
}

vector<NodePtr> Node::getParentChain()
{
    vector<NodePtr> pNodes;
    NodePtr pCurNode = getSharedThis();
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
        pParent->removeChild(getSharedThis(), bKill);
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
    Player::get()->setEventCapture(getSharedThis(), cursorID);
}

void Node::releaseEventCapture(int cursorID) 
{
    Player::get()->releaseEventCapture(cursorID);
}

void Node::setEventHandler(Event::Type type, int sources, PyObject * pFunc)
{
    avgDeprecationWarning("1.7", "Node.setEventHandler()", "Node.subscribe()");
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
    avgDeprecationWarning("1.8", "Node.connectEventHandler()", "Node.subscribe()");
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
    avgDeprecationWarning("1.8", "Node.disconnectEventHandler()", "Node.unsubscribe()");
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
    if (m_pParent == 0) {
        parentPos = absPos;
    } else {
        parentPos = m_pParent->getSharedThis()->getRelPos(absPos);
    }
    return toLocal(parentPos);
}

glm::vec2 Node::getAbsPos(const glm::vec2& relPos) const 
{
    glm::vec2 thisPos = toGlobal(relPos);
    glm::vec2 parentPos;
    if (m_pParent == 0) {
        parentPos = thisPos;
    } else {
        parentPos = m_pParent->getSharedThis()->getAbsPos(thisPos);
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
    vector<NodePtr> elements;
    getElementsByPos(pos, elements);
    if (elements.empty()) {
        return NodePtr();
    } else {
        return elements[0];
    }
}

void Node::getElementsByPos(const glm::vec2& pos, vector<NodePtr>& pElements)
{
}

void Node::preRender(const VertexArrayPtr& pVA, bool bIsParentActive, 
        float parentEffectiveOpacity)
{
    m_EffectiveOpacity = m_Opacity*parentEffectiveOpacity;
    m_bEffectiveActive = bIsParentActive && m_bActive;
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
    if (pEvent->getSource() != Event::NONE && pEvent->getSource() != Event::CUSTOM) {
        string messageID = getEventMessageID(pEvent);
        notifySubscribers(messageID, pEvent);
    }

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

bool Node::checkReload(const std::string& sHRef, const ImagePtr& pImage,
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
                AVG_LOG_ERROR(ex.getStr());
            } else {
                AVG_TRACE(Logger::category::MEMORY, Logger::severity::INFO, ex.getStr());
            }
        }
        return true;
    } else {
        return false;
    }
}

bool Node::isVisible() const
{
    return getEffectiveActive() && getEffectiveOpacity() > 0.01;
}

bool Node::getEffectiveActive() const
{
    return m_bEffectiveActive;
}

NodePtr Node::getSharedThis()
{
    return dynamic_pointer_cast<Node>(ExportedObject::getSharedThis());
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

string Node::getEventMessageID(const EventPtr& pEvent)
{
    Event::Source source = pEvent->getSource();
    if (source == Event::MOUSE || source == Event::TOUCH) {
        switch (pEvent->getType()) {
            case Event::CURSOR_DOWN:
                return "CURSOR_DOWN";
            case Event::CURSOR_MOTION:
                return "CURSOR_MOTION";
            case Event::CURSOR_UP:
                return "CURSOR_UP";
            case Event::CURSOR_OVER:
                return "CURSOR_OVER";
            case Event::CURSOR_OUT:
                return "CURSOR_OUT";
            default:
                AVG_ASSERT_MSG(false, 
                        (string("Unknown message type ")+pEvent->typeStr()).c_str());
                return "";
        }
    } else {
        switch (pEvent->getType()) {
            case Event::CURSOR_DOWN:
                return "HOVER_DOWN";
            case Event::CURSOR_MOTION:
                return "HOVER_MOTION";
            case Event::CURSOR_UP:
                return "HOVER_UP";
            case Event::CURSOR_OVER:
                return "HOVER_OVER";
            case Event::CURSOR_OUT:
                return "HOVER_OUT";
            default:
                AVG_ASSERT_MSG(false, 
                        (string("Unknown message type ")+pEvent->typeStr()).c_str());
                return "";
        }
    }
}

bool Node::callPython(PyObject * pFunc, EventPtr pEvent)
{
    bool bOk = py::call<bool>(pFunc, pEvent);
    return bOk;
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
