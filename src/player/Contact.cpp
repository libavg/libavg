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

#include "../base/Exception.h"
#include "CursorEvent.h"

namespace avg {

Contact::Contact(CursorEventPtr pEvent)
    : m_CursorID(pEvent->getCursorID())
{
    m_pNewEvents.push_back(pEvent);
}

Contact::~Contact()
{
}

void Contact::pushEvent(CursorEventPtr pEvent)
{
    AVG_ASSERT(pEvent);
    pEvent->setCursorID(m_CursorID);
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

}

