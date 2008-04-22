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

#include "Node.h"

#include "NodeDefinition.h"
#include "Event.h"
#include "CursorEvent.h"
#include "MouseEvent.h"
#include "DivNode.h"
#include "Player.h"
#include "DisplayEngine.h"
#include "Arg.h"
#include "WrapPython.h" 

#include "../base/MathHelper.h"
#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/XMLHelper.h"
#include "../base/ObjectCounter.h"

#include <object.h>
#include <compile.h>
#include <eval.h>
#include "BoostPython.h"

#include <iostream>

using namespace boost::python;

using namespace std;

namespace avg {


NodeDefinition Node::getNodeDefinition()
{
    return NodeDefinition("node")
        .addArg(Arg<string>("id", "", false, offsetof(Node, m_ID)))
        .addArg(Arg<string>("oncursormove", ""))
        .addArg(Arg<string>("oncursorup", ""))
        .addArg(Arg<string>("oncursordown", ""))
        .addArg(Arg<string>("oncursorover", ""))
        .addArg(Arg<string>("oncursorout", ""))
        .addArg(Arg<double>("x", 0.0, false, offsetof(Node, m_RelViewport.tl.x)))
        .addArg(Arg<double>("y", 0.0, false, offsetof(Node, m_RelViewport.tl.y)))
        .addArg(Arg<double>("width", 0.0, false, offsetof(Node, m_WantedSize.x)))
        .addArg(Arg<double>("height", 0.0, false, offsetof(Node, m_WantedSize.y)))
        .addArg(Arg<double>("angle", 0.0, false, offsetof(Node, m_Angle)))
        .addArg(Arg<double>("pivotx", -32767, false, offsetof(Node, m_Pivot.x)))
        .addArg(Arg<double>("pivoty", -32767, false, offsetof(Node, m_Pivot.y)))
        .addArg(Arg<double>("opacity", 1.0, false, offsetof(Node, m_Opacity)))
        .addArg(Arg<bool>("active", true, false, offsetof(Node, m_bActive)))
        .addArg(Arg<bool>("sensitive", true, false, offsetof(Node, m_bSensitive)));
}

Node::Node (Player * pPlayer)
    : m_pParent(),
      m_This(),
      m_pDisplayEngine(0),
      m_pAudioEngine(0),
      m_pPlayer(pPlayer),
      m_RelViewport(0,0,0,0)
{
    ObjectCounter::get()->incRef(&typeid(*this));
    setState(NS_UNCONNECTED);
}

Node::~Node()
{
    EventHandlerMap::iterator it;
    for (it=m_EventHandlerMap.begin(); it != m_EventHandlerMap.end(); ++it) {
        Py_DECREF(it->second);
    }
    ObjectCounter::get()->decRef(&typeid(*this));
}

void Node::setThis(NodeWeakPtr This)
{
    m_This = This;
}

void Node::setArgs(const ArgList& Args)
{
    addEventHandlers(Event::CURSORMOTION, Args.getArgVal<string> ("oncursormove"));
    addEventHandlers(Event::CURSORUP, Args.getArgVal<string> ("oncursorup"));
    addEventHandlers(Event::CURSORDOWN, Args.getArgVal<string> ("oncursordown"));
    addEventHandlers(Event::CURSOROVER, Args.getArgVal<string> ("oncursorover"));
    addEventHandlers(Event::CURSOROUT, Args.getArgVal<string> ("oncursorout"));
    m_RelViewport.setWidth(m_WantedSize.x);
    m_RelViewport.setHeight(m_WantedSize.y);
}

void Node::setParent(DivNodeWeakPtr pParent)
{
    if (getParent() && !!(pParent.lock())) {
        throw(Exception(AVG_ERR_UNSUPPORTED, 
                string("Can't change parent of node (") + m_ID + ")."));
    }
    m_pParent = pParent;
    setState(NS_CONNECTED);
}

void Node::setRenderingEngines(DisplayEngine * pDisplayEngine, AudioEngine * pAudioEngine)
{
    m_bHasCustomPivot = ((m_Pivot.x != -32767) && (m_Pivot.y != -32767));
    DPoint PreferredSize = getPreferredMediaSize();
    if (m_WantedSize.x == 0.0) {
        m_RelViewport.setWidth(PreferredSize.x);
    } else {
        m_RelViewport.setWidth(m_WantedSize.x);
    }
    if (m_WantedSize.y == 0.0) {
        m_RelViewport.setHeight(PreferredSize.y);
    } else {
        m_RelViewport.setHeight(m_WantedSize.y);
    } 
    m_pDisplayEngine = pDisplayEngine;
    m_pAudioEngine = pAudioEngine;
}

void Node::disconnect()
{
    m_pDisplayEngine = 0;
    m_pAudioEngine = 0;
    getPlayer()->removeNodeID(m_ID);
    setState(NS_UNCONNECTED);
}

const string& Node::getID () const
{
    return m_ID;
}

void Node::setID(const std::string& ID)
{
    if (getState() == NS_CONNECTED) {
        throw(Exception(AVG_ERR_UNSUPPORTED, "Node with ID "+m_ID
                +" is connected. setID invalid."));
    }
    m_ID = ID;
}

double Node::getX() const 
{
    return m_RelViewport.tl.x;
}

void Node::setX(double x) 
{
    setViewport(x, -32767, -32767, -32767);
}

double Node::getY() const 
{
    return m_RelViewport.tl.y;
}

void Node::setY(double y) 
{
    setViewport(-32767, y, -32767, -32767);
}

double Node::getWidth() 
{
    return getRelViewport().width();
}

void Node::setWidth(double width) 
{
    m_WantedSize.x = width;
    setViewport(-32767, -32767, width, -32767);
}

double Node::getHeight()
{
    return getRelViewport().height();
}

void Node::setHeight(double height) 
{
    m_WantedSize.y = height;
    setViewport(-32767, -32767, -32767, height);
}

DPoint Node::getRelSize() const
{
    return getRelViewport().size();
}

double Node::getAngle() const
{
    return m_Angle;
}

void Node::setAngle(double Angle)
{
    m_Angle = fmod(Angle, 2*PI);
}

double Node::getPivotX() const
{
    return m_Pivot.x;
}

void Node::setPivotX(double Pivotx)
{
    m_Pivot = getPivot();
    m_Pivot.x = Pivotx;
    m_bHasCustomPivot = true;
}

double Node::getPivotY() const
{
    return m_Pivot.y;
}

void Node::setPivotY(double Pivoty)
{
    m_Pivot = getPivot();
    m_Pivot.y = Pivoty;
    m_bHasCustomPivot = true;
}

double Node::getOpacity() const {
    return m_Opacity;
}

void Node::setOpacity(double opacity) {
    m_Opacity = opacity;
    if (m_Opacity < 0.0) {
        m_Opacity = 0.0;
    } else if (m_Opacity > 1.0) {
        m_Opacity = 1.0;
    }
}

bool Node::getActive() const {
    return m_bActive;
}

void Node::setActive(bool bActive)
{
    if (bActive != m_bActive) {
        m_bActive = bActive;
    }
}

bool Node::getSensitive() const {
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

DPoint Node::getRelPos(const DPoint& AbsPos) const 
{
    DPoint parentPos;
    if (m_pParent.expired()) {
        parentPos = AbsPos;
    } else {
        parentPos = m_pParent.lock()->getRelPos(AbsPos);
    }
    return toLocal(parentPos);
}

void Node::setMouseEventCapture()
{
    setEventCapture(MOUSECURSORID);
}

void Node::releaseMouseEventCapture()
{
    releaseEventCapture(MOUSECURSORID);
}

void Node::setEventCapture(int cursorID) {
    m_pPlayer->setEventCapture(m_This, cursorID);
}

void Node::releaseEventCapture(int cursorID) {
    m_pPlayer->releaseEventCapture(cursorID);
}

void Node::setEventHandler(Event::Type Type, Event::Source Source, PyObject * pFunc)
{
    EventHandlerID ID(Type, Source);
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

bool Node::isActive()
{
    return m_bActive;
}

bool Node::reactsToMouseEvents()
{
    return m_bActive && m_bSensitive;
}

NodePtr Node::getElementByPos (const DPoint & pos)
{
    DPoint relPos = toLocal(pos);
    if (relPos.x >= 0 && relPos.y >= 0 && 
            relPos.x < getRelSize().x && relPos.y < getRelSize().y &&
            reactsToMouseEvents())
    {
        return m_This.lock();
    } else {
        return NodePtr();
    }
}

void Node::maybeRender (const DRect& Rect)
{
    if (m_bActive) {
        getDisplayEngine()->pushTransform(getRelViewport().tl, getAngle(), getPivot());
        if (getEffectiveOpacity() > 0.01) {
            if (m_ID != "") {
                AVG_TRACE(Logger::BLTS, "Rendering " << getTypeStr() << 
                        " with ID " << m_ID);
            } else {
                AVG_TRACE(Logger::BLTS, "Rendering " << getTypeStr()); 
            }
            render(Rect);
        }
        getDisplayEngine()->popTransform();
    }
}

void Node::render (const DRect& Rect)
{
}

Node::NodeState Node::getState() const
{
    return m_State;
}

bool Node::isDisplayAvailable() const
{
    return (getState() == NS_CONNECTED) && m_pDisplayEngine;
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

DPoint Node::getPivot() const
{
    if (m_bHasCustomPivot) {
        return m_Pivot;
    } else {
        const DPoint& pt = getRelSize();
        return DPoint (pt.x/2, pt.y/2);
    }
}

Player * Node::getPlayer() const
{
    return m_pPlayer;
}

void Node::setViewport (double x, double y, double width, double height)
{
    if (x == -32767) {
        x = getRelViewport().tl.x;
    }
    if (y == -32767) {
        y = getRelViewport().tl.y;
    }
    DPoint MediaSize = getPreferredMediaSize();
    if (width == -32767) {
        if (m_WantedSize.x == 0.0) {
            width = MediaSize.x;
        } else {
            width = m_WantedSize.x;
        } 
    }
    if (height == -32767) {
        if (m_WantedSize.y == 0.0) {
            height = MediaSize.y;
        } else {
            height = m_WantedSize.y;
        } 
    }
    m_RelViewport = DRect (x, y, x+width, y+height);
}

const DRect& Node::getRelViewport () const
{
//    cerr << "Node " << m_ID << ": (" << m_RelViewport.tl.x << ", " 
//            << m_RelViewport.tl.y << ")" << endl;
    return m_RelViewport;
}

double Node::getEffectiveOpacity()
{
    if (getParent()) {
        return m_Opacity*getParent()->getEffectiveOpacity();
    } else {
        return m_Opacity;
    }
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

string Node::dump (int indent)
{
    string dumpStr = string(indent, ' ') + getTypeStr() + ": m_ID=" + m_ID;
    char sz[256];
    sprintf (sz, ", x=%.1f, y=%.1f, width=%.1f, height=%.1f, opacity=%.2f\n",
            m_RelViewport.tl.x, m_RelViewport.tl.y,
            m_RelViewport.width(), m_RelViewport.height(), m_Opacity);
    dumpStr += sz;

    return dumpStr; 
}

string Node::getTypeStr () const 
{
    return "Node";
}

Node::EventHandlerID::EventHandlerID(Event::Type EventType, Event::Source Source)
    : m_Type(EventType),
      m_Source(Source)
{
}

bool Node::EventHandlerID::operator < (const EventHandlerID& other) const {
    if (m_Type == other.m_Type) {
        return m_Source < other.m_Source;
    } else {
        return m_Type < other.m_Type;
    }
}

void Node::handleEvent (Event* pEvent)
{
    EventHandlerID ID(pEvent->getType(), pEvent->getSource());
    EventHandlerMap::iterator it = m_EventHandlerMap.find(ID);
    if (it!=m_EventHandlerMap.end()) {
        callPython(it->second, pEvent);
    }
}

void Node::callPython (PyObject * pFunc, Event *pEvent)
{
    boost::python::call<void>(pFunc, boost::python::ptr(pEvent));
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
                    "\" not defined for node with id '"+m_ID+"'. Aborting.");
            exit(-1);
        }
        return pFunc;
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

void Node::initFilename(Player * pPlayer, string& sFilename)
{
    bool bAbsDir = sFilename[0] == '/';
#ifdef _WIN32
    if (!bAbsDir) {
        bAbsDir = (sFilename[0] == '\\' || sFilename[1] == ':');
    }
#endif
    if (!bAbsDir) {
        if (m_pParent.expired()) {
            sFilename = pPlayer->getCurDirName()+sFilename;
        } else {
            sFilename = m_pParent.lock()->getEffectiveMediaDir()+sFilename;
        }
    }
}

void Node::setState(Node::NodeState State)
{
/*
    cerr << m_ID << "state: ";
    switch(State) {
        case NS_UNCONNECTED:
            cerr << "unconnected" << endl;
            break;
        case NS_CONNECTED:
            cerr << "connected" << endl;
            break;
        case NS_DISABLED:
            cerr << "disabled" << endl;
            break;
    }
*/
    m_State = State;
}

DPoint Node::toLocal(const DPoint& globalPos) const
{
    DPoint localPos = globalPos-m_RelViewport.tl;
    return rotate(localPos, -m_Angle, getPivot());
}

DPoint Node::toGlobal(const DPoint& localPos) const
{
    DPoint globalPos = rotate(localPos, m_Angle, getPivot());
    return globalPos+m_RelViewport.tl;
}

}
