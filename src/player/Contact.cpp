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

#include "../base/Exception.h"

#include <iostream>

using namespace std;

namespace avg {

Contact::Contact(CursorEventPtr pEvent)
    : m_CursorID(pEvent->getCursorID())
{
    m_pNewEvents.push_back(pEvent);
}

Contact::~Contact()
{
}

void Contact::setThis(ContactWeakPtr This)
{
    m_This = This;
    m_pNewEvents[0]->setContact(getThis());
}

ContactPtr Contact::getThis() const
{
    return m_This.lock();
}
    
void Contact::connectListener(PyObject* pListener)
{
    Py_INCREF(pListener);
    m_pListeners.push_back(pListener);
}

void Contact::disconnectListener(PyObject* pListener)
{
    bool bFound = false;
    vector<PyObject*>::iterator it = m_pListeners.begin();
    while (it != m_pListeners.end() && !bFound) {
        if (PyObject_RichCompareBool(*it, pListener, Py_EQ)) {
            Py_DECREF(*it);
            it = m_pListeners.erase(it);
            bFound = true;
        } else {
            it++;
        }
    }
    if (!bFound) {
        throw Exception(AVG_ERR_INVALID_ARGS, 
                "Contact.disconnectListener: Not connected.");
    }
}

void Contact::pushEvent(CursorEventPtr pEvent)
{
    AVG_ASSERT(pEvent);
    pEvent->setCursorID(m_CursorID);
    pEvent->setContact(getThis());
    if (m_pEvents.empty()) {
        // This is the first frame. Ignore unless cursorup.
        if (pEvent->getType() == Event::CURSORUP) {
            // Down and up in the first frame. To avoid inconsistencies, both
            // messages must be delivered. This is the only time that m_pNewEvents
            // has more than one entry.
            m_pNewEvents.push_back(pEvent);
        }
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

CursorEventPtr Contact::pollEvent()
{
    if (m_pNewEvents.empty()) {
        return CursorEventPtr();
    } else {
        CursorEventPtr pEvent = m_pNewEvents[0];
        m_pNewEvents.erase(m_pNewEvents.begin());
        m_pEvents.push_back(pEvent);
        if (pEvent->getType() == Event::CURSORUP) {
            // Last event that this contact will ever receive.
            disconnectAllListeners();
        }
        return pEvent;
    }
}

CursorEventPtr Contact::getLastEvent()
{
    if (m_pNewEvents.empty()) {
        AVG_ASSERT(!m_pEvents.empty());
        return m_pEvents.back();
    } else {
        return m_pNewEvents.back();
    }
}

bool Contact::hasListeners() const
{
    return !m_pListeners.empty();
}

void Contact::sendEventToListeners(CursorEventPtr pEvent)
{
    for (int i = 0; i < m_pListeners.size(); ++i) {
        boost::python::call<void>(m_pListeners[i], 
                boost::dynamic_pointer_cast<Event>(pEvent));
    }
}

int Contact::getID() const
{
    return m_CursorID;
}

void Contact::disconnectAllListeners()
{
    for (int i = 0; i < m_pListeners.size(); ++i) {
        Py_DECREF(m_pListeners[i]);
    }
    m_pListeners.clear();
}

}

