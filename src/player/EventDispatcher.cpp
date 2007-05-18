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

#include "EventDispatcher.h"
#include "Event.h"

#include <string>

using namespace std;
using namespace boost;

namespace avg {

    EventDispatcher::EventDispatcher()
        : m_pLastMouseEvent(new MouseEvent(Event::CURSORMOTION, false, false, false, 
                IntPoint(0, 0), MouseEvent::NO_BUTTON))
    {
    }

    EventDispatcher::~EventDispatcher() 
    {
    }

    void EventDispatcher::dispatch() 
    {
        for (unsigned int i = 0; i<m_EventSources.size(); ++i) {
            vector<EventPtr> curEvents = m_EventSources[i]->pollEvents();
            for (unsigned int i= 0; i<curEvents.size(); i++) {
                m_Events.push(curEvents[i]);
            }
        }

        while (!m_Events.empty()) {
            EventPtr pCurEvent = m_Events.top();
            m_Events.pop();
            sendEvent(pCurEvent);
        }
    }

    const MouseEventPtr EventDispatcher::getLastMouseEvent() const 
    {
        return m_pLastMouseEvent;
    }
        
    void EventDispatcher::addSource(IEventSource * pSource)
    {
        m_EventSources.push_back(pSource);
        pSource->initEventSource();
    }

    void EventDispatcher::addSink(IEventSink * pSink)
    {
        m_EventSinks.push_back(pSink);
    }

    void EventDispatcher::sendEvent(EventPtr pEvent)
    {
        if (dynamic_pointer_cast<MouseEvent>(pEvent) != 0) {
            m_pLastMouseEvent = dynamic_pointer_cast<MouseEvent>(pEvent);
        }
        for (unsigned int i = 0; i < m_EventSinks.size(); ++i) {
            if (m_EventSinks[i]->handleEvent(pEvent)) {
                break;
            }
        }
    }

}

