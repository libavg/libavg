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

#include "AreaNode.h"

#include "Event.h"
#include "CursorEvent.h"
#include "MouseEvent.h"
#include "DivNode.h"
#include "Player.h"
#include "DisplayEngine.h"

#include "../base/MathHelper.h"
#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/ObjectCounter.h"

#include <object.h>
#include <compile.h>
#include <eval.h>
#include "BoostPython.h"

#include <iostream>

using namespace boost;
using namespace boost::python;

using namespace std;

namespace avg {

NodeDefinition AreaNode::createDefinition()
{
    return NodeDefinition("areanode")
        .extendDefinition(Node::createDefinition())
        .addArg(Arg<string>("oncursormove", ""))
        .addArg(Arg<string>("oncursorup", ""))
        .addArg(Arg<string>("oncursordown", ""))
        .addArg(Arg<string>("oncursorover", ""))
        .addArg(Arg<string>("oncursorout", ""))
        .addArg(Arg<double>("x", 0.0, false, offsetof(AreaNode, m_RelViewport.tl.x)))
        .addArg(Arg<double>("y", 0.0, false, offsetof(AreaNode, m_RelViewport.tl.y)))
        .addArg(Arg<double>("width", 0.0, false, offsetof(AreaNode, m_WantedSize.x)))
        .addArg(Arg<double>("height", 0.0, false, offsetof(AreaNode, m_WantedSize.y)))
        .addArg(Arg<double>("angle", 0.0, false, offsetof(AreaNode, m_Angle)))
        .addArg(Arg<double>("pivotx", -32767, false, offsetof(AreaNode, m_Pivot.x)))
        .addArg(Arg<double>("pivoty", -32767, false, offsetof(AreaNode, m_Pivot.y)))
        .addArg(Arg<bool>("active", true, false, offsetof(AreaNode, m_bActive)))
        .addArg(Arg<bool>("sensitive", true, false, offsetof(AreaNode, m_bSensitive)));
}

AreaNode::AreaNode()
    : m_RelViewport(0,0,0,0)
{
    ObjectCounter::get()->incRef(&typeid(*this));
}

AreaNode::~AreaNode()
{
    EventHandlerMap::iterator it;
    for (it=m_EventHandlerMap.begin(); it != m_EventHandlerMap.end(); ++it) {
        Py_DECREF(it->second);
    }
    ObjectCounter::get()->decRef(&typeid(*this));
}

void AreaNode::setArgs(const ArgList& Args)
{
    addEventHandlers(Event::CURSORMOTION, Args.getArgVal<string> ("oncursormove"));
    addEventHandlers(Event::CURSORUP, Args.getArgVal<string> ("oncursorup"));
    addEventHandlers(Event::CURSORDOWN, Args.getArgVal<string> ("oncursordown"));
    addEventHandlers(Event::CURSOROVER, Args.getArgVal<string> ("oncursorover"));
    addEventHandlers(Event::CURSOROUT, Args.getArgVal<string> ("oncursorout"));
    m_RelViewport.setWidth(m_WantedSize.x);
    m_RelViewport.setHeight(m_WantedSize.y);
}

void AreaNode::setRenderingEngines(DisplayEngine * pDisplayEngine, 
        AudioEngine * pAudioEngine)
{
    m_bHasCustomPivot = ((m_Pivot.x != -32767) && (m_Pivot.y != -32767));
    IntPoint PreferredSize = getMediaSize();
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
    Node::setRenderingEngines(pDisplayEngine, pAudioEngine);
}

DivNodePtr AreaNode::getDivParent() const
{
    return dynamic_pointer_cast<DivNode>(Node::getParent());
}

double AreaNode::getX() const 
{
    return m_RelViewport.tl.x;
}

void AreaNode::setX(double x) 
{
    setViewport(x, -32767, -32767, -32767);
}

double AreaNode::getY() const 
{
    return m_RelViewport.tl.y;
}

void AreaNode::setY(double y) 
{
    setViewport(-32767, y, -32767, -32767);
}

const DPoint& AreaNode::getPos() const
{
    return m_RelViewport.tl;
}

void AreaNode::setPos(const DPoint& pt)
{
    setViewport(pt.x, pt.y, -32767, -32767);
}

double AreaNode::getWidth() 
{
    return getRelViewport().width();
}

void AreaNode::setWidth(double width) 
{
    m_WantedSize.x = width;
    setViewport(-32767, -32767, width, -32767);
}

double AreaNode::getHeight()
{
    return getRelViewport().height();
}

void AreaNode::setHeight(double height) 
{
    m_WantedSize.y = height;
    setViewport(-32767, -32767, -32767, height);
}

DPoint AreaNode::getSize() const
{
    return getRelViewport().size();
}

void AreaNode::setSize(const DPoint& pt)
{
    setViewport(-32767, -32767, pt.x, pt.y);
}

double AreaNode::getAngle() const
{
    return m_Angle;
}

void AreaNode::setAngle(double Angle)
{
    m_Angle = fmod(Angle, 2*PI);
}

double AreaNode::getPivotX() const
{
    return m_Pivot.x;
}

void AreaNode::setPivotX(double Pivotx)
{
    m_Pivot = getPivot();
    m_Pivot.x = Pivotx;
    m_bHasCustomPivot = true;
}

double AreaNode::getPivotY() const
{
    return m_Pivot.y;
}

void AreaNode::setPivotY(double Pivoty)
{
    m_Pivot = getPivot();
    m_Pivot.y = Pivoty;
    m_bHasCustomPivot = true;
}

bool AreaNode::getActive() const 
{
    return m_bActive;
}

void AreaNode::setActive(bool bActive)
{
    if (bActive != m_bActive) {
        m_bActive = bActive;
    }
}

bool AreaNode::getSensitive() const 
{
    return m_bSensitive;
}

void AreaNode::setSensitive(bool bSensitive)
{
    m_bSensitive = bSensitive;
}

DPoint AreaNode::getRelPos(const DPoint& AbsPos) const 
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

DPoint AreaNode::getAbsPos(const DPoint& RelPos) const 
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

void AreaNode::setMouseEventCapture()
{
    setEventCapture(MOUSECURSORID);
}

void AreaNode::releaseMouseEventCapture()
{
    releaseEventCapture(MOUSECURSORID);
}

void AreaNode::setEventCapture(int cursorID) 
{
    Player::get()->setEventCapture(dynamic_pointer_cast<AreaNode>(getThis()), cursorID);
}

void AreaNode::releaseEventCapture(int cursorID) 
{
    Player::get()->releaseEventCapture(cursorID);
}

void AreaNode::setEventHandler(Event::Type Type, int Sources, PyObject * pFunc)
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

bool AreaNode::isActive()
{
    return m_bActive;
}

bool AreaNode::reactsToMouseEvents()
{
    return m_bActive && m_bSensitive;
}

AreaNodePtr AreaNode::getElementByPos(const DPoint & pos)
{
    DPoint relPos = toLocal(pos);
    if (relPos.x >= 0 && relPos.y >= 0 && 
            relPos.x < getSize().x && relPos.y < getSize().y &&
            reactsToMouseEvents())
    {
        return dynamic_pointer_cast<AreaNode>(getThis());
    } else {
        return AreaNodePtr();
    }
}

void AreaNode::maybeRender(const DRect& Rect)
{
    assert(getState() == NS_CANRENDER);
    if (m_bActive) {
        if (getEffectiveOpacity() > 0.01) {
            if (getID() != "") {
                AVG_TRACE(Logger::BLTS, "Rendering " << getTypeStr() << 
                        " with ID " << getID());
            } else {
                AVG_TRACE(Logger::BLTS, "Rendering " << getTypeStr()); 
            }
            getDisplayEngine()->pushTransform(getRelViewport().tl, getAngle(), getPivot());
            render(Rect);
            getDisplayEngine()->popTransform();
        }
    }
}

void AreaNode::setViewport (double x, double y, double width, double height)
{
    if (x == -32767) {
        x = getRelViewport().tl.x;
    }
    if (y == -32767) {
        y = getRelViewport().tl.y;
    }
    IntPoint MediaSize = getMediaSize();
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

const DRect& AreaNode::getRelViewport() const
{
//    cerr << "Node " << getID() << ": (" << m_RelViewport.tl.x << ", " 
//            << m_RelViewport.tl.y << ")" << endl;
    return m_RelViewport;
}

string AreaNode::dump(int indent)
{
    string dumpStr = Node::dump(indent); 
    char sz[256];
    sprintf (sz, ", x=%.1f, y=%.1f, width=%.1f, height=%.1f\n",
            m_RelViewport.tl.x, m_RelViewport.tl.y,
            m_RelViewport.width(), m_RelViewport.height());
    dumpStr += sz;

    return dumpStr; 
}

void AreaNode::handleEvent(EventPtr pEvent)
{
    EventHandlerID ID(pEvent->getType(), pEvent->getSource());
    EventHandlerMap::iterator it = m_EventHandlerMap.find(ID);
    if (it!=m_EventHandlerMap.end()) {
        callPython(it->second, pEvent);
    }
}

DPoint AreaNode::getPivot() const
{
    if (m_bHasCustomPivot) {
        return m_Pivot;
    } else {
        return getSize()/2;
    }
}

void AreaNode::callPython (PyObject * pFunc, EventPtr pEvent)
{
    boost::python::call<void>(pFunc, pEvent);
}

PyObject * AreaNode::findPythonFunc(const string& Code)
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

void AreaNode::addEventHandlers(Event::Type EventType, const string& Code)
{
    addEventHandler(EventType, Event::MOUSE, Code);
    addEventHandler(EventType, Event::TOUCH, Code);
    addEventHandler(EventType, Event::TRACK, Code);
}

void AreaNode::addEventHandler(Event::Type EventType, Event::Source Source, 
        const string& Code)
{
    PyObject * pFunc = findPythonFunc(Code);
    if (pFunc) {
        Py_INCREF(pFunc);
        EventHandlerID ID(EventType, Source);
        m_EventHandlerMap[ID] = pFunc;
    }
}

void AreaNode::initFilename(string& sFilename)
{
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

DPoint AreaNode::toLocal(const DPoint& globalPos) const
{
    DPoint localPos = globalPos-m_RelViewport.tl;
    return rotate(localPos, -getAngle(), getPivot());
}

DPoint AreaNode::toGlobal(const DPoint& localPos) const
{
    DPoint globalPos = rotate(localPos, getAngle(), getPivot());
    return globalPos+m_RelViewport.tl;
}

AreaNode::EventHandlerID::EventHandlerID(Event::Type EventType, Event::Source Source)
    : m_Type(EventType),
      m_Source(Source)
{
}

bool AreaNode::EventHandlerID::operator < (const EventHandlerID& other) const 
{
    if (m_Type == other.m_Type) {
        return m_Source < other.m_Source;
    } else {
        return m_Type < other.m_Type;
    }
}

}
