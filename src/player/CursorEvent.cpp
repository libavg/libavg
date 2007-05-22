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
//  Original author of this file is igor@c-base.org
//

#include "Node.h"
#include "CursorEvent.h"

namespace avg{

CursorEvent::CursorEvent(int id, Type eventType, IntPoint Position, Source source)
            :Event(eventType),
            m_Position(Position),
            m_ID(id),
            m_Source(source)
{
}

CursorEvent::~CursorEvent()
{
}

Event *CursorEvent::cloneAs(Type EventType)
{
    assert(false);
}

int CursorEvent::getXPosition() const
{
    return m_Position.x;
}

int CursorEvent::getYPosition() const
{
    return m_Position.y;
}

int CursorEvent::getCursorID() const
{
    return m_ID;
}

CursorEvent::Source CursorEvent::getSource() const
{
    return m_Source;
}

}

