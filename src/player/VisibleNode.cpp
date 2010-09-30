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

#include "VisibleNode.h"

#include "NodeDefinition.h"
#include "DivNode.h"
#include "Player.h"
#include "SDLDisplayEngine.h"
#include "Arg.h"
#include "Image.h"
#include "Canvas.h"

#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/XMLHelper.h"
#include "../base/StringHelper.h"
#include "../base/ObjectCounter.h"
#include "../base/OSHelper.h"
#include <Magick++.h>

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
    EventHandlerMap::iterator it;
    for (it=m_EventHandlerMap.begin(); it != m_EventHandlerMap.end(); ++it) {
        Py_DECREF(it->second);
    }
    ObjectCounter::get()->decRef(&typeid(*this));
}

VisibleNodePtr VisibleNode::getVThis() const
{
    return dynamic_pointer_cast<VisibleNode>(getThis());
}

void VisibleNode::setArgs(const ArgList& Args)
{
    addEventHandlers(Event::CURSORMOTION, Args.getArgVal<string> ("oncursormove"));
    addEventHandlers(Event::CURSORUP, Args.getArgVal<string> ("oncursorup"));
    addEventHandlers(Event::CURSORDOWN, Args.getArgVal<string> ("oncursordown"));
    addEventHandlers(Event::CURSOROVER, Args.getArgVal<string> ("oncursorover"));
    addEventHandlers(Event::CURSOROUT, Args.getArgVal<string> ("oncursorout"));
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
        EventHandlerMap::iterator it;
        for (it=m_EventHandlerMap.begin(); it != m_EventHandlerMap.end(); ++it) {
            Py_DECREF(it->second);
        }
        m_EventHandlerMap.clear();
    }
}

void VisibleNode::setID(const std::string& ID)
{
    if (getState() != NS_UNCONNECTED) {
        throw(Exception(AVG_ERR_UNSUPPORTED, "Node with ID "+getID()
                +" is connected. setID invalid."));
    }
    Node::setID(ID);
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

void VisibleNode::setEventHandler(Event::Type Type, int Sources, PyObject * pFunc)
{
    for (int source=1; source<=Event::NONE; source*=2) {
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

bool VisibleNode::reactsToMouseEvents()
{
    return m_bActive && m_bSensitive;
}

DPoint VisibleNode::getRelPos(const DPoint& AbsPos) const 
{
    DPoint parentPos;
    DivNodePtr pParent = getDivParent();
    if (!pParent) {
        parentPos = AbsPos;
    } else {
        parentPos = pParent->getRelPos(AbsPos);
    }
    return toLocal(parentPos);
}

DPoint VisibleNode::getAbsPos(const DPoint& RelPos) const 
{
    DPoint thisPos = toGlobal(RelPos);
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

VisibleNodePtr VisibleNode::getElementByPos(const DPoint & pos)
{
    return VisibleNodePtr();
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
    EventHandlerID ID(pEvent->getType(), pEvent->getSource());
    EventHandlerMap::iterator it = m_EventHandlerMap.find(ID);
    if (it!=m_EventHandlerMap.end()) {
        return callPython(it->second, pEvent);
    } else {
        return false;
    }
}

void VisibleNode::addEventHandlers(Event::Type EventType, const string& Code)
{
    addEventHandler(EventType, Event::MOUSE, Code);
    addEventHandler(EventType, Event::TOUCH, Code);
    addEventHandler(EventType, Event::TRACK, Code);
}

void VisibleNode::addEventHandler(Event::Type EventType, Event::Source Source, 
        const string& Code)
{
    PyObject * pFunc = findPythonFunc(Code);
    if (pFunc) {
        Py_INCREF(pFunc);
        EventHandlerID ID(EventType, Source);
        m_EventHandlerMap[ID] = pFunc;
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

double VisibleNode::getEffectiveOpacity()
{
    return m_EffectiveOpacity;
}

string VisibleNode::dump(int indent)
{
    string dumpStr = string(indent, ' ') + getTypeStr() + ": m_ID=" + getID() +
            "m_Opacity=" + toString(m_Opacity);
    return dumpStr; 
}

void VisibleNode::setState(VisibleNode::NodeState State)
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
        AVG_ASSERT(State != NS_CANRENDER);
    }
    if (m_State == NS_CANRENDER) {
        AVG_ASSERT(State != NS_CONNECTED);
    }

    m_State = State;
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
        } catch (Magick::Exception & ex) {
            pImage->setEmpty();
            if (getState() != VisibleNode::NS_UNCONNECTED) {
                AVG_TRACE(Logger::ERROR, ex.what());
            } else {
                AVG_TRACE(Logger::MEMORY, ex.what());
            }
        }
    }
}

bool VisibleNode::callPython(PyObject * pFunc, EventPtr pEvent)
{
    return boost::python::call<bool>(pFunc, pEvent);
}

PyObject * VisibleNode::findPythonFunc(const string& Code)
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

VisibleNode::EventHandlerID::EventHandlerID(Event::Type EventType, Event::Source Source)
    : m_Type(EventType),
      m_Source(Source)
{
}

bool VisibleNode::EventHandlerID::operator < (const EventHandlerID& other) const 
{
    if (m_Type == other.m_Type) {
        return m_Source < other.m_Source;
    } else {
        return m_Type < other.m_Type;
    }
}

}
