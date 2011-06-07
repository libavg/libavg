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

#include "Contact.h"

#include "CursorEvent.h"
#include "BoostPython.h"
#include "Player.h"

#include "../base/Exception.h"
#include "../base/StringHelper.h"

#include <iostream>

using namespace std;

namespace avg {

int Contact::s_LastListenerID = 0;

Contact::Contact(CursorEventPtr pEvent, bool bProcessEvents)
    : m_bProcessEvents(bProcessEvents),
      m_bFirstFrame(true),
      m_bSendingEvents(false),
      m_CursorID(pEvent->getCursorID()),
      m_DistanceTravelled(0)
{
    if (bProcessEvents) {
        m_pNewEvents.push_back(pEvent);
    }
    m_pFirstEvent = pEvent;
    m_pLastEvent = pEvent;
}

Contact::~Contact()
{
}

void Contact::disconnectEverything()
{
    m_ListenerMap.clear();
    m_pNewEvents.clear();
    m_pFirstEvent = CursorEventPtr();
    m_pLastEvent = CursorEventPtr();
    Player::get()->deregisterContact(getThis());
}

void Contact::setThis(ContactWeakPtr This)
{
    m_This = This;
    m_pFirstEvent->setContact(getThis());
    Player::get()->registerContact(getThis());
}

ContactPtr Contact::getThis() const
{
    return m_This.lock();
}
    
int Contact::connectListener(PyObject* pMotionCallback, PyObject* pUpCallback)
{
    if (Player::get()->isCaptured(m_CursorID)) {
        throw Exception(AVG_ERR_INVALID_CAPTURE, 
                "Attempted to connect listener to cursor id " + toString(m_CursorID) +
                ", but the cursor was already captured.");
    }
    s_LastListenerID++;
    pair<int, Listener> val = 
            pair<int, Listener>(s_LastListenerID, Listener(pMotionCallback, pUpCallback));
    m_ListenerMap.insert(val);
    return s_LastListenerID;
}

void Contact::disconnectListener(int id)
{
    bool bFound = false;
    map<int, Listener>::iterator it = m_ListenerMap.find(id);
    if (it == m_ListenerMap.end() || m_DeadListeners.count(id) != 0) {
        throw Exception(AVG_ERR_INVALID_ARGS, 
                "Contact.disconnectListener: id " + toString(id) + " is not connected.");
    }
    if (m_bSendingEvents) {
        m_DeadListeners.insert(id);
    } else {
        m_ListenerMap.erase(it);
    }
}

long long Contact::getAge() const
{
    return m_pLastEvent->getWhen() - m_pFirstEvent->getWhen();
}

double Contact::getDistanceFromStart() const
{
    return getMotionVec().getNorm();
}

double Contact::getMotionAngle() const
{
    DPoint motion = getMotionVec();
    if (motion == DPoint(0,0)) {
        return 0;
    } else {
        return motion.getAngle();
    }
}

DPoint Contact::getMotionVec() const
{
    return m_pLastEvent->getPos() - m_pFirstEvent->getPos();
}

double Contact::getDistanceTravelled() const
{
    return m_DistanceTravelled;
}

void Contact::pushEvent(CursorEventPtr pEvent, bool bCheckMotion)
{
    AVG_ASSERT(m_bProcessEvents);
    AVG_ASSERT(pEvent);
    pEvent->setCursorID(m_CursorID);
    pEvent->setContact(getThis());

    if (m_bFirstFrame) {
        // Ignore unless cursorup.
        if (pEvent->getType() == Event::CURSORUP) {
            // Down and up in the first frame. To avoid inconsistencies, both
            // messages must be delivered. This is the only time that m_pNewEvents
            // has more than one entry.
            m_pNewEvents.push_back(pEvent);
        }
    } else {
        if (bCheckMotion && pEvent->getType() == Event::CURSORMOTION && 
                getLastEvent()->getPos() == pEvent->getPos())
        {
            // Ignore motion events without motion.
            return;
        } else {
            if (m_pNewEvents.empty()) {
                // No pending events: schedule for delivery.
                m_pNewEvents.push_back(pEvent);
            } else {
                // More than one event per poll: Deliver only the last one.
                m_pNewEvents[0] = pEvent;
            }
        }
    }
}

void Contact::addEvent(CursorEventPtr pEvent)
{
    AVG_ASSERT(!m_bProcessEvents);
    pEvent->setCursorID(m_CursorID);
    pEvent->setContact(getThis());
    calcSpeed(pEvent, m_pLastEvent);
    updateDistanceTravelled(m_pLastEvent, pEvent);
    m_pLastEvent = pEvent;
}

CursorEventPtr Contact::pollEvent()
{
    AVG_ASSERT(m_bProcessEvents);
    if (m_pNewEvents.empty()) {
        return CursorEventPtr();
    } else {
        CursorEventPtr pEvent = m_pNewEvents[0];
        m_pNewEvents.erase(m_pNewEvents.begin());
        m_bFirstFrame = false;
        calcSpeed(pEvent, m_pLastEvent);
        updateDistanceTravelled(m_pLastEvent, pEvent);
        m_pLastEvent = pEvent;
        AVG_ASSERT(pEvent->getContact() == getThis());
        return pEvent;
    }
}

CursorEventPtr Contact::getLastEvent()
{
    if (m_pNewEvents.empty()) {
        AVG_ASSERT(m_pLastEvent);
        return m_pLastEvent;
    } else {
        return m_pNewEvents.back();
    }
}

bool Contact::hasListeners() const
{
    return !(m_ListenerMap.size() == m_DeadListeners.size());
}

void Contact::sendEventToListeners(CursorEventPtr pCursorEvent)
{
    m_bSendingEvents = true;
    AVG_ASSERT(pCursorEvent->getContact() == getThis());
    EventPtr pEvent = boost::dynamic_pointer_cast<Event>(pCursorEvent);
    for (map<int, Listener>::iterator it = m_ListenerMap.begin(); 
            it != m_ListenerMap.end(); ++it)
    {
        Listener listener = it->second;
        switch (pCursorEvent->getType()) {
            case Event::CURSORMOTION:
                if (listener.m_pMotionCallback != Py_None) {
                    boost::python::call<void>(listener.m_pMotionCallback, pEvent);
                }
                break;
            case Event::CURSORUP:
                if (listener.m_pUpCallback != Py_None) {
                    boost::python::call<void>(listener.m_pUpCallback, pEvent);
                }
                break;
            default:
                AVG_ASSERT(false);
        }
        pCursorEvent->setNode(VisibleNodePtr());
    }
    m_bSendingEvents = false;
    m_DeadListeners.clear();
}

int Contact::getID() const
{
    return m_CursorID;
}

void Contact::calcSpeed(CursorEventPtr pEvent, CursorEventPtr pOldEvent)
{
    if (pEvent->getSpeed() == DPoint(0,0)) {
        DPoint posDiff = pEvent->getPos() - pOldEvent->getPos();
        long long timeDiff = pEvent->getWhen() - pOldEvent->getWhen();
        if (timeDiff != 0) {
            pEvent->setSpeed(posDiff/double(timeDiff));
        }
    }
}
void Contact::updateDistanceTravelled(CursorEventPtr pEvent1, CursorEventPtr pEvent2)
{
    double dist = (pEvent2->getPos() - pEvent1->getPos()).getNorm();
    m_DistanceTravelled += dist;
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

