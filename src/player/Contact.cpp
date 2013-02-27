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

#include "Contact.h"

#include "CursorEvent.h"
#include "BoostPython.h"
#include "Player.h"
#include "PublisherDefinition.h"

#include "../base/Exception.h"
#include "../base/StringHelper.h"
#include "../base/Logger.h"

#include <iostream>

using namespace std;

namespace avg {

int Contact::s_LastListenerID = 0;

void Contact::registerType()
{
    PublisherDefinitionPtr pPubDef = PublisherDefinition::create("Contact");
    pPubDef->addMessage("CURSOR_MOTION");
    pPubDef->addMessage("CURSOR_UP");
}

Contact::Contact(CursorEventPtr pEvent)
    : Publisher("Contact"),
      m_bSendingEvents(false),
      m_bCurListenerIsDead(false),
      m_CursorID(pEvent->getCursorID()),
      m_DistanceTravelled(0)
{
    m_Events.push_back(pEvent);
}

Contact::~Contact()
{
}

int Contact::connectListener(PyObject* pMotionCallback, PyObject* pUpCallback)
{
    avgDeprecationWarning("1.8", "Contact.connectListener()", "Contact.subscribe()");
    s_LastListenerID++;
    pair<int, Listener> val = 
            pair<int, Listener>(s_LastListenerID, Listener(pMotionCallback, pUpCallback));
    m_ListenerMap.insert(val);
    return s_LastListenerID;
}

void Contact::disconnectListener(int id)
{
    avgDeprecationWarning("1.8", "Contact.disconnectListener()", 
            "Contact.unsubscribe()");
    map<int, Listener>::iterator it = m_ListenerMap.find(id);
    if (it == m_ListenerMap.end() || (m_CurListenerID == id && m_bCurListenerIsDead)) {
        throw Exception(AVG_ERR_INVALID_ARGS, 
                "Contact.disconnectListener: id " + toString(id) + " is not connected.");
    }
    if (m_bSendingEvents && m_CurListenerID == id) {
        m_bCurListenerIsDead = true;
    } else {
        m_ListenerMap.erase(it);
    }
}

long long Contact::getAge() const
{
    return m_Events.back()->getWhen() - m_Events[0]->getWhen();
}

float Contact::getDistanceFromStart() const
{
    return glm::length(getMotionVec());
}

float Contact::getMotionAngle() const
{
    glm::vec2 motion = getMotionVec();
    if (motion == glm::vec2(0,0)) {
        return 0;
    } else {
        return getAngle(motion);
    }
}

glm::vec2 Contact::getMotionVec() const
{
    return m_Events.back()->getPos() - m_Events[0]->getPos();
}

float Contact::getDistanceTravelled() const
{
    return m_DistanceTravelled;
}

vector<CursorEventPtr> Contact::getEvents() const
{
    return m_Events;
}

void Contact::addEvent(CursorEventPtr pEvent)
{
    pEvent->setCursorID(m_CursorID);
    pEvent->setContact(boost::dynamic_pointer_cast<Contact>(shared_from_this()));
    calcSpeed(pEvent, m_Events.back());
    updateDistanceTravelled(m_Events.back(), pEvent);
    m_Events.back()->removeBlob();
    m_Events.back()->setNode(NodePtr());
    m_Events.push_back(pEvent);
}

void Contact::sendEventToListeners(CursorEventPtr pCursorEvent)
{
    switch (pCursorEvent->getType()) {
        case Event::CURSOR_DOWN:
            break;
        case Event::CURSOR_MOTION:
            notifySubscribers("CURSOR_MOTION", pCursorEvent);
            break;
        case Event::CURSOR_UP:
            notifySubscribers("CURSOR_UP", pCursorEvent);
            removeSubscribers();
            break;
        default:
            AVG_ASSERT_MSG(false, pCursorEvent->typeStr().c_str());
    }
    m_bSendingEvents = true;
    AVG_ASSERT(pCursorEvent->getContact() == shared_from_this());
    m_bCurListenerIsDead = false;
    for (map<int, Listener>::iterator it = m_ListenerMap.begin(); 
            it != m_ListenerMap.end();)
    {
        Listener listener = it->second;
        m_CurListenerID = it->first;
        m_bCurListenerIsDead = false;
        switch (pCursorEvent->getType()) {
            case Event::CURSOR_MOTION:
                if (listener.m_pMotionCallback != Py_None) {
                    py::call<void>(listener.m_pMotionCallback, pCursorEvent);
                }
                break;
            case Event::CURSOR_UP:
                if (listener.m_pUpCallback != Py_None) {
                    py::call<void>(listener.m_pUpCallback, pCursorEvent);
                }
                break;
            default:
                AVG_ASSERT(false);
        }
        map<int, Listener>::iterator lastIt = it;
        ++it;
        if (m_bCurListenerIsDead) {
            m_ListenerMap.erase(lastIt);
            m_bCurListenerIsDead = false;
        }
    }
    m_bSendingEvents = false;
}

int Contact::getID() const
{
    return m_CursorID;
}

void Contact::calcSpeed(CursorEventPtr pEvent, CursorEventPtr pOldEvent)
{
    if (pEvent->getSpeed() == glm::vec2(0,0)) {
        glm::vec2 posDiff = pEvent->getPos() - pOldEvent->getPos();
        long long timeDiff = pEvent->getWhen() - pOldEvent->getWhen();
        if (timeDiff != 0) {
            pEvent->setSpeed(posDiff/float(timeDiff));
        }
    }
}

void Contact::updateDistanceTravelled(CursorEventPtr pEvent1, CursorEventPtr pEvent2)
{
    float dist = glm::length(pEvent2->getPos() - pEvent1->getPos());
    m_DistanceTravelled += dist;
}

void Contact::dumpListeners(string sFuncName)
{
    cerr << "  " << sFuncName << ": ";
    for (map<int, Listener>::iterator it = m_ListenerMap.begin(); 
            it != m_ListenerMap.end(); ++it)
    {
        cerr << it->first << ", ";
    }
    cerr << endl;
}

Contact::Listener::Listener(PyObject * pMotionCallback, PyObject * pUpCallback)
{
    Py_INCREF(pMotionCallback);
    m_pMotionCallback = pMotionCallback;
    Py_INCREF(pUpCallback);
    m_pUpCallback = pUpCallback;
}

Contact::Listener::Listener(const Listener& other)
{
    Py_INCREF(other.m_pMotionCallback);
    m_pMotionCallback = other.m_pMotionCallback;
    Py_INCREF(other.m_pUpCallback);
    m_pUpCallback = other.m_pUpCallback;
}

Contact::Listener::~Listener()
{
    Py_DECREF(m_pMotionCallback);
    Py_DECREF(m_pUpCallback);
}


}

