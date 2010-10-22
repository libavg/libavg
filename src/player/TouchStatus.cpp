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

#include "TouchStatus.h"

namespace avg {

TouchStatus::TouchStatus(TouchEventPtr pEvent)
    : m_pEvent(pEvent),
      m_bFirstFrame(true),
      m_LastDownPos(pEvent->getPos()),
      m_CursorID(pEvent->getCursorID())
{
    pEvent->setLastDownPos(IntPoint(pEvent->getPos()));
}

TouchStatus::~TouchStatus()
{
}

const IntPoint& TouchStatus::getLastDownPos()
{
    return m_LastDownPos;
}

bool TouchStatus::isFirstFrame()
{
    return m_bFirstFrame;
}

void TouchStatus::updateEvent(TouchEventPtr pEvent)
{
    if (isFirstFrame()) {
        // Always send a cursordown event first.
        m_pEvent = boost::dynamic_pointer_cast<TouchEvent>(
                pEvent->cloneAs(Event::CURSORDOWN));
        if (pEvent->getType() == Event::CURSORUP) {
            // If we get a down and an up in the first frame, we delay the up to the
            // next frame.
            m_pUpEvent = pEvent;
            m_pUpEvent->setCursorID(m_CursorID);
        }
    } else {
        m_pEvent = pEvent;
    }
    m_pEvent->setCursorID(m_CursorID);
    m_pEvent->setLastDownPos(m_LastDownPos);
}

TouchEventPtr TouchStatus::getEvent()
{
    m_bFirstFrame = false;
    TouchEventPtr pEvent;
    if (!m_pEvent && m_pUpEvent) {
        // Special case: delayed up.
        pEvent = m_pUpEvent;
        m_pUpEvent = TouchEventPtr();
    } else {
        pEvent = m_pEvent;
        m_pEvent = TouchEventPtr();
    }
    return pEvent;
}

}

