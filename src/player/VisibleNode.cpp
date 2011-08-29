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

#include "VisibleNode.h"

#include "NodeDefinition.h"
#include "DivNode.h"
#include "Player.h"
#include "SDLDisplayEngine.h"
#include "Arg.h"
#include "Image.h"
#include "Canvas.h"
#include "MouseEvent.h"

#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/XMLHelper.h"
#include "../base/StringHelper.h"
#include "../base/ObjectCounter.h"
#include "../base/OSHelper.h"

#include <iostream>
#ifdef _WIN32
#include <Windows.h>
#endif

using namespace std;
using namespace boost;

namespace avg {

NodeDefinition VisibleNode::createDefinition()
{
    return NodeDefinition("visiblenode")
        .extendDefinition(Node::createDefinition())
        .addArg(Arg<string>("oncursormove", ""))
        .addArg(Arg<string>("oncursorup", ""))
        .addArg(Arg<string>("oncursordown", ""))
        .addArg(Arg<string>("oncursorover", ""))
        .addArg(Arg<string>("oncursorout", ""))
        .addArg(Arg<bool>("active", true, false, offsetof(VisibleNode, m_bActive)))
        .addArg(Arg<bool>("sensitive", true, false, offsetof(VisibleNode, m_bSensitive)))
        .addArg(Arg<double>("opacity", 1.0, false, offsetof(VisibleNode, m_Opacity)));
}

VisibleNode::VisibleNode()
    : m_pCanvas(),
      m_pDisplayEngine(0),
      m_pAudioEngine(0),
      m_State(NS_UNCONNECTED)
{
    ObjectCounter::get()->incRef(&typeid(*this));
}

VisibleNode::~VisibleNode()
{
    m_EventHandlerMap.clear();
    ObjectCounter::get()->decRef(&typeid(*this));
}

VisibleNodePtr VisibleNode::getVThis() const
{
    return dynamic_pointer_cast<VisibleNode>(getThis());
}

void VisibleNode::setArgs(const ArgList& args)
{
    addArgEventHandlers(Event::CURSORMOTION, args.getArgVal<string> ("oncursormove"));
    addArgEventHandlers(Event::CURSORUP, args.getArgVal<string> ("oncursorup"));
    addArgEventHandlers(Event::CURSORDOWN, args.getArgVal<string> ("oncursordown"));
    addArgEventHandlers(Event::CURSOROVER, args.getArgVal<string> ("oncursorover"));
    addArgEventHandlers(Event::CURSOROUT, args.getArgVal<string> ("oncursorout"));
}

void VisibleNode::setParent(DivNodeWeakPtr pParent, NodeState parentState,
        CanvasPtr pCanvas)
{
    AVG_ASSERT(getState() == NS_UNCONNECTED);
    Node::setParent(pParent);
    if (parentState != NS_UNCONNECTED) {
        connect(pCanvas);
    }
}

void VisibleNode::setRenderingEngines(DisplayEngine * pDisplayEngine,
        AudioEngine * pAudioEngine)
{
    AVG_ASSERT(getState() == NS_CONNECTED);
    m_pDisplayEngine = dynamic_cast<SDLDisplayEngine*>(pDisplayEngine);
    m_pAudioEngine = pAudioEngine;
    setState(NS_CANRENDER);
}

void VisibleNode::connect(CanvasPtr pCanvas)
{
    m_pCanvas = pCanvas;
    setState(NS_CONNECTED);
}

void VisibleNode::disconnect(bool bKill)
{
    AVG_ASSERT(getState() != NS_UNCONNECTED);
    if (getState() == NS_CANRENDER) {
        m_pDisplayEngine = 0;
        m_pAudioEngine = 0;
    }
    m_pCanvas.lock()->removeNodeID(getID());
    setState(NS_UNCONNECTED);
    if (bKill) {
        m_EventHandlerMap.clear();
    }
}

void VisibleNode::setID(const std::string& sID)
{
    if (getState() != NS_UNCONNECTED) {
        throw Exception(AVG_ERR_UNSUPPORTED, "Node with ID "+getID()
                +" is connected. setID invalid.");
    }
    Node::setID(sID);
}

double VisibleNode::getOpacity() const 
{
    return m_Opacity;
}

void VisibleNode::setOpacity(double opacity) 
{
    m_Opacity = opacity;
    if (m_Opacity < 0.0) {
        m_Opacity = 0.0;
    } else if (m_Opacity > 1.0) {
        m_Opacity = 1.0;
    }
}

bool VisibleNode::getActive() const 
{
    return m_bActive;
}

void VisibleNode::setActive(bool bActive)
{
    if (bActive != m_bActive) {
        m_bActive = bActive;
    }
}

bool VisibleNode::getSensitive() const 
{
    return m_bSensitive;
}

void VisibleNode::setSensitive(bool bSensitive)
{
    m_bSensitive = bSensitive;
}

DivNodePtr VisibleNode::getDivParent() const
{
    return dynamic_pointer_cast<DivNode>(getParent());
}

vector<VisibleNodeWeakPtr> VisibleNode::getParentChain() const
{
    vector<VisibleNodeWeakPtr> pNodes;
    VisibleNodePtr pCurNode = getVThis();
    while (pCurNode) {
        pNodes.push_back(pCurNode);
        pCurNode = pCurNode->getDivParent();
    }
    return pNodes;
}

void VisibleNode::unlink(bool bKill)
{
    DivNodePtr pParent = getDivParent();
    if (pParent != DivNodePtr()) {
        pParent->removeChild(getVThis(), bKill);
    }
}

void VisibleNode::setMouseEventCapture()
{
    setEventCapture(MOUSECURSORID);
}

void VisibleNode::releaseMouseEventCapture()
{
    releaseEventCapture(MOUSECURSORID);
}

void VisibleNode::setEventCapture(int cursorID) 
{
    Player::get()->setEventCapture(getVThis(), cursorID);
}

void VisibleNode::releaseEventCapture(int cursorID) 
{
    Player::get()->releaseEventCapture(cursorID);
}

void VisibleNode::setEventHandler(Event::Type type, int sources, PyObject * pFunc)
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

void VisibleNode::connectEventHandler(Event::Type type, int sources, 
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

void VisibleNode::disconnectEventHandler(PyObject * pObj, PyObject * pFunc)
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

bool VisibleNode::reactsToMouseEvents()
{
    return m_bActive && m_bSensitive;
}

DPoint VisibleNode::getRelPos(const DPoint& absPos) const 
{
    DPoint parentPos;
    DivNodePtr pParent = getDivParent();
    if (!pParent) {
        parentPos = absPos;
    } else {
        parentPos = pParent->getRelPos(absPos);
    }
    return toLocal(parentPos);
}

DPoint VisibleNode::getAbsPos(const DPoint& relPos) const 
{
    DPoint thisPos = toGlobal(relPos);
    DPoint parentPos;
    DivNodePtr pParent = getDivParent();
    if (!pParent) {
        parentPos = thisPos;
    } else {
        parentPos = pParent->getAbsPos(thisPos);
    }
    return parentPos;
}

DPoint VisibleNode::toLocal(const DPoint& globalPos) const
{
    return globalPos;
}

DPoint VisibleNode::toGlobal(const DPoint& localPos) const
{
    return localPos;
}

VisibleNodePtr VisibleNode::getElementByPos(const DPoint& pos)
{
    vector<VisibleNodeWeakPtr> elements;
    getElementsByPos(pos, elements);
    if (elements.empty()) {
        return VisibleNodePtr();
    } else {
        return elements[0].lock();
    }
}

void VisibleNode::getElementsByPos(const DPoint& pos, 
                vector<VisibleNodeWeakPtr>& pElements)
{
}

void VisibleNode::preRender()
{
    if (getParent()) {
        m_EffectiveOpacity = m_Opacity*getDivParent()->getEffectiveOpacity();
    } else {
        m_EffectiveOpacity = m_Opacity;
    }
}

VisibleNode::NodeState VisibleNode::getState() const
{
    return m_State;
}

CanvasPtr VisibleNode::getCanvas() const
{
    return m_pCanvas.lock();
}

bool VisibleNode::handleEvent(EventPtr pEvent)
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

void VisibleNode::addArgEventHandlers(Event::Type eventType, const string& sCode)
{
    addArgEventHandler(eventType, Event::MOUSE, sCode);
    addArgEventHandler(eventType, Event::TOUCH, sCode);
    addArgEventHandler(eventType, Event::TRACK, sCode);
}

void VisibleNode::addArgEventHandler(Event::Type eventType, Event::Source source, 
        const string& sCode)
{
    PyObject * pFunc = findPythonFunc(sCode);
    if (pFunc) {
        EventID id(eventType, source);
        connectOneEventHandler(id, Py_None, pFunc);
    }
}

SDLDisplayEngine * VisibleNode::getDisplayEngine() const
{
    return m_pDisplayEngine;
}

AudioEngine * VisibleNode::getAudioEngine() const
{
    return m_pAudioEngine;
}

double VisibleNode::getEffectiveOpacity() const
{
    return m_EffectiveOpacity;
}

string VisibleNode::dump(int indent)
{
    string dumpStr = string(indent, ' ') + getTypeStr() + ": m_ID=" + getID() +
            "m_Opacity=" + toString(m_Opacity);
    return dumpStr; 
}

void VisibleNode::setState(VisibleNode::NodeState state)
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
        
void VisibleNode::initFilename(string& sFilename)
{
    if (sFilename != "") {
        bool bAbsDir = sFilename[0] == '/';
#ifdef _WIN32
        if (!bAbsDir) {
            bAbsDir = (sFilename[0] == '\\' || sFilename[1] == ':');
        }
#endif
        if (!bAbsDir) {
            DivNodePtr pParent = getDivParent();
            if (!pParent) {
                sFilename = Player::get()->getRootMediaDir()+sFilename;
            } else {
                sFilename = pParent->getEffectiveMediaDir()+sFilename;
            }
        }
    }
}

void VisibleNode::checkReload(const std::string& sHRef, const ImagePtr& pImage,
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
            if (getState() != VisibleNode::NS_UNCONNECTED) {
                AVG_TRACE(Logger::ERROR, ex.getStr());
            } else {
                AVG_TRACE(Logger::MEMORY, ex.getStr());
            }
        }
    }
}

bool VisibleNode::isVisible() const
{
    return getEffectiveActive() && getEffectiveOpacity() > 0.01;
}

bool VisibleNode::getEffectiveActive() const
{
    DivNodePtr pParent = dynamic_pointer_cast<DivNode>(getParent());
    if (pParent) {
        return m_bActive && pParent->getEffectiveActive();
    } else {
        return m_bActive;
    }
}

void VisibleNode::connectOneEventHandler(const EventID& id, PyObject * pObj, 
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

void VisibleNode::dumpEventHandlers()
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

PyObject * VisibleNode::findPythonFunc(const string& sCode)
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

bool VisibleNode::callPython(PyObject * pFunc, EventPtr pEvent)
{
    bool bOk = boost::python::call<bool>(pFunc, pEvent);
    return bOk;
}

VisibleNode::EventID::EventID(Event::Type eventType, Event::Source source)
    : m_Type(eventType),
      m_Source(source)
{
}

bool VisibleNode::EventID::operator < (const EventID& other) const 
{
    if (m_Type == other.m_Type) {
        return m_Source < other.m_Source;
    } else {
        return m_Type < other.m_Type;
    }
}

VisibleNode::EventHandler::EventHandler(PyObject * pObj, PyObject * pMethod)
{
    Py_INCREF(pObj);
    m_pObj = pObj;
    Py_INCREF(pMethod);
    m_pMethod = pMethod;
}

VisibleNode::EventHandler::EventHandler(const EventHandler& other)
{
    Py_INCREF(other.m_pObj);
    m_pObj = other.m_pObj;
    Py_INCREF(other.m_pMethod);
    m_pMethod = other.m_pMethod;
}

VisibleNode::EventHandler::~EventHandler()
{
    Py_DECREF(m_pObj);
    Py_DECREF(m_pMethod);
}

}
