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

#include "EventDispatcher.h"
#include "Event.h"
#include "Player.h"
#include "Contact.h"
#include "CursorEvent.h"

#include "../base/Exception.h"
#include "../base/OSHelper.h"

#include <string>

using namespace std;
using namespace boost;

namespace avg {

EventDispatcher::EventDispatcher(Player* pPlayer, bool bMouseEnabled)
    : m_pPlayer(pPlayer),
      m_NumMouseButtonsDown(0),
      m_bMouseEnabled(bMouseEnabled)
{
}

EventDispatcher::~EventDispatcher() 
{
}

void EventDispatcher::dispatch() 
{
    vector<EventPtr> events;

    for (unsigned int i = 0; i < m_InputDevices.size(); ++i) {
        IInputDevicePtr pCurInputDevice = m_InputDevices[i];

        vector<EventPtr> curEvents = pCurInputDevice->pollEvents();
        vector<EventPtr>::iterator eventIt = curEvents.begin();
        events.insert(events.end(), curEvents.begin(), curEvents.end());

        for ( ; eventIt != curEvents.end(); eventIt++) {
            (*eventIt)->setInputDevice(pCurInputDevice);
        }
    }

    vector<EventPtr>::iterator it;
    for (it = events.begin(); it != events.end(); ++it) {
        EventPtr pEvent = *it;
        bool bHookEatsEvent = processEventHook(pEvent);
        if (!(!m_bMouseEnabled && pEvent->getSource() == Event::MOUSE) &&
                !bHookEatsEvent) {
            testAddContact(pEvent);
            handleEvent(*it);
            testRemoveContact(pEvent);
        }
    }
}

void EventDispatcher::addInputDevice(IInputDevicePtr pInputDevice)
{
    m_InputDevices.push_back(pInputDevice);
}

void EventDispatcher::sendEvent(EventPtr pEvent)
{
    handleEvent(pEvent);
}

void EventDispatcher::enableMouse(bool bEnabled)
{
    m_bMouseEnabled = bEnabled;
}

ContactPtr EventDispatcher::getContact(int id)
{
    std::map<int, ContactPtr>::iterator it = m_ContactMap.find(id);
    if (it == m_ContactMap.end()) {
        return ContactPtr();
    } else {
        return it->second;
    }
}

void EventDispatcher::handleEvent(EventPtr pEvent)
{
    m_pPlayer->handleEvent(pEvent);
}

bool EventDispatcher::processEventHook(EventPtr pEvent)
{
    PyObject * pEventHook = m_pPlayer->getEventHook();
    if (pEventHook != Py_None) {
        // If the hook returns true, stop processing the event
        if (py::call<bool>(pEventHook, pEvent)) {
            return true;
        }
    }
    return false;
}

void EventDispatcher::testAddContact(EventPtr pEvent)
{
    ContactPtr pContact;
    CursorEventPtr pCursorEvent = dynamic_pointer_cast<CursorEvent>(pEvent);
    if (pCursorEvent) {
        switch (pCursorEvent->getType()) {
            case Event::CURSOR_DOWN:
                if (pCursorEvent->getSource() == Event::MOUSE) {
                    m_NumMouseButtonsDown++;
                    if (m_NumMouseButtonsDown == 1) {
                        AVG_ASSERT(!getContact(MOUSECURSORID));
                        pContact = ContactPtr(new Contact(pCursorEvent));
                        m_ContactMap[MOUSECURSORID] = pContact;
                    }
                } else {
                    pContact = ContactPtr(new Contact(pCursorEvent));
                    m_ContactMap[pCursorEvent->getCursorID()] = pContact;
                }
                break;
            case Event::CURSOR_MOTION:
            case Event::CURSOR_UP: {
                    pContact = getContact(pCursorEvent->getCursorID());
                    AVG_ASSERT(pContact || (
                            pCursorEvent->getSource() == Event::MOUSE && 
                            m_NumMouseButtonsDown == 0));
                    if (pContact) {
                        pContact->addEvent(pCursorEvent);
                    }
                }
                break;
            case Event::CUSTOM_EVENT:
                break;
            default:
                cerr << pCursorEvent->typeStr() << endl;
                AVG_ASSERT(false);
                break;
        }
        if (pContact) {
            pCursorEvent->setContact(pContact);
        }
    }
}

void EventDispatcher::testRemoveContact(EventPtr pEvent)
{
    if (pEvent->getType() == Event::CURSOR_UP) {
        if (pEvent->getSource() == Event::MOUSE) {
            // The following if is only false if the CURSOR_DOWN wasn't registered because
            // it was outside the application window (or on the window border).
            if (m_NumMouseButtonsDown > 0) {
                m_NumMouseButtonsDown--;
                if (m_NumMouseButtonsDown == 0) {
                    int rc = m_ContactMap.erase(MOUSECURSORID);
                    AVG_ASSERT(rc == 1);
                }
            }
        } else {
            int rc = m_ContactMap.erase(
                    dynamic_pointer_cast<CursorEvent>(pEvent)->getCursorID());
            AVG_ASSERT(rc == 1);
        }
    }
}

}
