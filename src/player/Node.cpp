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
#include "DivNode.h"

#include "Event.h"
#include "CursorEvent.h"
#include "MouseEvent.h"
#include "DivNode.h"
#include "Player.h"
#include "DisplayEngine.h"

#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/XMLHelper.h"
#include "../base/ObjectCounter.h"

#include <Python.h>
#include <object.h>
#include <compile.h>
#include <eval.h>
#include <boost/python.hpp>

#include <iostream>

using namespace boost::python;

using namespace std;

namespace avg {

Node::Node ()
    : m_pParent(),
      m_This(),
      m_pEngine(0),
      m_pPlayer(0),
      m_ID(""),
      m_EventHandlerMap(),
      m_RelViewport(0,0,0,0),
      m_AbsViewport(0,0,0,0),
      m_Opacity(1.0),
      m_bActive(true),
      m_bSensitive(true),
      m_WantedSize(0, 0)
{
    ObjectCounter::get()->incRef(&typeid(*this));
    setState(NS_UNCONNECTED);
}

Node::Node (const xmlNodePtr xmlNode, Player * pPlayer)
    : m_pParent(),
      m_This(),
      m_pEngine(0),
      m_pPlayer(pPlayer),
      m_RelViewport(0,0,0,0),
      m_AbsViewport(0,0,0,0)
{
    ObjectCounter::get()->incRef(&typeid(*this));
    m_ID = getDefaultedStringAttr (xmlNode, "id", "");
    m_EventHandlerMap[Event::CURSORMOTION] = getDefaultedStringAttr (xmlNode, "oncursormove", "");
    m_EventHandlerMap[Event::CURSORUP] = getDefaultedStringAttr (xmlNode, "oncursorup", "");
    m_EventHandlerMap[Event::CURSORDOWN] = getDefaultedStringAttr (xmlNode, "oncursordown", "");
    m_EventHandlerMap[Event::CURSOROVER] = getDefaultedStringAttr (xmlNode, "oncursorover", "");
    m_EventHandlerMap[Event::CURSOROUT] = getDefaultedStringAttr (xmlNode, "oncursorout", "");
    m_RelViewport.tl.x = getDefaultedDoubleAttr (xmlNode, "x", 0.0);
    m_RelViewport.tl.y = getDefaultedDoubleAttr (xmlNode, "y", 0.0);
    m_WantedSize.x = getDefaultedDoubleAttr (xmlNode, "width", 0.0);
    m_WantedSize.y = getDefaultedDoubleAttr (xmlNode, "height", 0.0);
    m_Opacity = getDefaultedDoubleAttr (xmlNode, "opacity", 1.0);
    m_bActive = getDefaultedBoolAttr (xmlNode, "active", true);
    m_bSensitive = getDefaultedBoolAttr (xmlNode, "sensitive", true);
    setState(NS_UNCONNECTED);
}

Node::~Node()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void Node::setThis(NodeWeakPtr This)
{
    m_This = This;
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

void Node::setDisplayEngine(DisplayEngine * pEngine)
{
    DPoint PreferredSize = getPreferredMediaSize();
    if (m_WantedSize.x == 0.0) {
        m_RelViewport.SetWidth(PreferredSize.x);
    } else {
        m_RelViewport.SetWidth(m_WantedSize.x);
    }
    if (m_WantedSize.y == 0.0) {
        m_RelViewport.SetHeight(PreferredSize.y);
    } else {
        m_RelViewport.SetHeight(m_WantedSize.y);
    } 
    m_pEngine = pEngine;
    DPoint pos(0,0);
    if (getParent()) {
        pos = getParent()->getAbsViewport().tl;
    }
    m_AbsViewport = DRect (pos+getRelViewport().tl, pos+getRelViewport().br);
}

void Node::disconnect()
{
    m_pEngine = 0;
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

double Node::getX() const {
    return m_RelViewport.tl.x;
}

void Node::setX(double x) {
    setViewport(x, -32767, -32767, -32767);
}

double Node::getY() const {
    return m_RelViewport.tl.y;
}

void Node::setY(double y) {
    setViewport(-32767, y, -32767, -32767);
}

double Node::getWidth() const {
    return getRelViewport().Width();
}

void Node::setWidth(double width) {
    m_WantedSize.x = width;
    setViewport(-32767, -32767, width, -32767);
}

double Node::getHeight() const {
    return getRelViewport().Height();
}

void Node::setHeight(double height) {
    m_WantedSize.y = height;
    setViewport(-32767, -32767, -32767, height);
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
    if (m_bActive) {
        invalidate();
    }
}

bool Node::getActive() const {
    return m_bActive;
}

void Node::setActive(bool bActive)
{
    if (bActive != m_bActive) {
        m_bActive = bActive;
        invalidate();
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

double Node::getRelXPos(double x) 
{
    DRect VP = getAbsViewport();
    return x-VP.tl.x;
}

double Node::getRelYPos(double y)
{
    DRect VP = getAbsViewport();
    return y-VP.tl.y;
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
    m_pPlayer->releaseEventCapture(m_This, cursorID);
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
    if (getVisibleRect().Contains(pos) && reactsToMouseEvents()) {
        return m_This.lock();
    } else {
        return NodePtr();
    }
}

void Node::prepareRender (int time, const DRect& parent)
{
    calcAbsViewport();
}

void Node::maybeRender (const DRect& Rect)
{
    if (m_bActive) {
        bool bVisible;
        // FIXME
        if (dynamic_cast<DivNode*>(this) != 0) {
            bVisible = getEngine()->pushClipRect(getVisibleRect(), true);
        } else {
            bVisible = getEngine()->pushClipRect(getVisibleRect(), false);
        }
        if (bVisible && getEffectiveOpacity() > 0.01) {
            if (!getParent() || !getParent()->obscures(getEngine()->getClipRect(), 
                    getParent()->indexOf(m_This.lock())))
            {
                if (m_ID != "") {
                    AVG_TRACE(Logger::BLTS, "Rendering " << getTypeStr() << 
                            " with ID " << m_ID);
                } else {
                    AVG_TRACE(Logger::BLTS, "Rendering " << getTypeStr()); 
                }
                render(Rect);
            }
        }
        getEngine()->popClipRect();
    }
}

void Node::render (const DRect& Rect)
{
}

bool Node::obscures (const DRect& Rect, int Child)  
{
    return false;
}

void Node::addDirtyRect(const DRect& Rect)
{
    m_DirtyRegion.addRect(Rect);
}

void Node::getDirtyRegion (Region& Region)
{
    Region.addRegion(m_DirtyRegion);
    m_DirtyRegion.clear();
}

void Node::invalidate()
{
    if (isDisplayAvailable()) {
        addDirtyRect(getVisibleRect());
    }
}

Node::NodeState Node::getState() const
{
    return m_State;
}

bool Node::isDisplayAvailable() const
{
    return (getState() == NS_CONNECTED) && m_pEngine;
}


Player * Node::getPlayer() const
{
    return m_pPlayer;
}

void Node::setViewport (double x, double y, double width, double height)
{
    if (m_bActive) {
        invalidate();
    }
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
    calcAbsViewport();
    if (m_bActive) {
        invalidate();
    }
}

const DRect& Node::getRelViewport () const
{
//    cerr << "Node " << m_ID << ": (" << m_RelViewport.tl.x << ", " 
//            << m_RelViewport.tl.y << ")" << endl;
    return m_RelViewport;
}

const DRect& Node::getAbsViewport () const
{
    return m_AbsViewport;
}

DRect Node::getVisibleRect() const
{
    DRect visRect = getAbsViewport();
    NodePtr pParent = getParent();
    if (pParent) {
        DRect parent = pParent->getVisibleRect();
        visRect.Intersect(parent);
    }
    return visRect;
}

void Node::calcAbsViewport()
{
    NodePtr pParent = getParent();
    if (pParent) {
        DRect parent = pParent->getAbsViewport();
        m_AbsViewport = DRect(parent.tl+getRelViewport().tl, 
                parent.tl+getRelViewport().br);
    } else {
        m_AbsViewport = getRelViewport();
    }
    if (m_AbsViewport.Width() < 0 || m_AbsViewport.Height() < 0) {
        m_AbsViewport.br=m_AbsViewport.tl;
    }
}

double Node::getEffectiveOpacity()
{
    if (getParent()) {
        return m_Opacity*getParent()->getEffectiveOpacity();
    } else {
        return m_Opacity;
    }
}

DisplayEngine * Node::getEngine() const
{
    return m_pEngine;
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
            m_RelViewport.Width(), m_RelViewport.Height(), m_Opacity);
    dumpStr += sz;
    sprintf (sz, "        Abs: (x=%.1f, y=%.1f, width=%.1f, height=%.1f)\n",
            m_AbsViewport.tl.x, m_AbsViewport.tl.y,  
            m_AbsViewport.Width(), m_AbsViewport.Height());
    dumpStr += sz;

    return dumpStr; 
}

string Node::getTypeStr () const 
{
    return "Node";
}

void Node::handleEvent (Event* pEvent)
{
    Event::Type EventType = pEvent->getType();
    map<Event::Type, string>::iterator it = m_EventHandlerMap.find(EventType);
    assert(it!=m_EventHandlerMap.end());
    if (!(it->second.empty())) {
        pEvent->setElement(m_This.lock());
        callPython(it->second, *pEvent);
    }
    if (getParent()) {
        getParent()->handleEvent (pEvent);
    }
}

void Node::callPython (const string& Code, const Event& Event)
{
    // TODO:
    //   - Pass Event to python.
    //   - Handle python return code/exception and pass to caller.
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
    PyObject * pArgList = Py_BuildValue("()");
    PyObject * pResult = PyObject_CallObject(pFunc, pArgList);
    if (!pResult) {
        throw error_already_set();
    }
    Py_DECREF(pArgList);
}

void Node::initFilename(Player * pPlayer, string& sFilename)
{
    if (sFilename[0] != '/') {
        sFilename = pPlayer->getCurDirName() + sFilename;
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

}

