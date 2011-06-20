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

Contact::Contact(CursorEventPtr pEvent)
    : m_bSendingEvents(false),
      m_bCurListenerIsDead(false),
      m_CursorID(pEvent->getCursorID()),
      m_DistanceTravelled(0)
{
    m_pFirstEvent = pEvent;
    m_pLastEvent = pEvent;
}

Contact::~Contact()
{
}

int Contact::connectListener(PyObject* pMotionCallback, PyObject* pUpCallback)
{
    s_LastListenerID++;
    pair<int, Listener> val = 
            pair<int, Listener>(s_LastListenerID, Listener(pMotionCallback, pUpCallback));
    m_ListenerMap.insert(val);
    return s_LastListenerID;
}

void Contact::disconnectListener(int id)
{
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

void Contact::addEvent(CursorEventPtr pEvent)
{
    pEvent->setCursorID(m_CursorID);
    pEvent->setContact(shared_from_this());
    calcSpeed(pEvent, m_pLastEvent);
    updateDistanceTravelled(m_pLastEvent, pEvent);
    m_pLastEvent = pEvent;
}

bool Contact::hasListeners() const
{
    return !(m_ListenerMap.empty() || 
            (m_ListenerMap.size() == 1 && m_bCurListenerIsDead));
}

void Contact::sendEventToListeners(CursorEventPtr pCursorEvent)
{
    m_bSendingEvents = true;
    AVG_ASSERT(pCursorEvent->getContact() == shared_from_this());
    EventPtr pEvent = boost::dynamic_pointer_cast<Event>(pCursorEvent);
    m_bCurListenerIsDead = false;
    for (map<int, Listener>::iterator it = m_ListenerMap.begin(); 
            it != m_ListenerMap.end();)
    {
        Listener listener = it->second;
        m_CurListenerID = it->first;
        m_bCurListenerIsDead = false;
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
        map<int, Listener>::iterator lastIt = it;
        ++it;
        if (m_bCurListenerIsDead) {
            m_ListenerMap.erase(lastIt);
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

