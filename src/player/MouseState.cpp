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

#include "MouseState.h"

#include <iostream>

using namespace std;

namespace avg {
    MouseState::MouseState()
        : m_pLastMouseEvent(new MouseEvent(Event::CURSORMOTION, false, false, false, 
                IntPoint(0, 0), MouseEvent::NO_BUTTON))
    {
    }

    MouseState::~MouseState()
    {
    }


    MouseEventPtr MouseState::getLastEvent() const
    {
        return m_pLastMouseEvent;
    }

    void MouseState::setEvent(MouseEventPtr pEvent)
    {
        m_pLastMouseEvent = pEvent;
        if (pEvent->getType() == Event::CURSORDOWN) {
            m_LastDownPos[pEvent->getButton()] = 
                    IntPoint(pEvent->getXPosition(), pEvent->getYPosition());
        }
    }
    
    const IntPoint& MouseState::getLastDownPos(long button) const
    {
        return m_LastDownPos[button];
    }
}

