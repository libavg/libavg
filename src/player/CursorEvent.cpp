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
//  Original author of this file is igor@c-base.org
//

#include "CursorEvent.h"

#include "VisibleNode.h"

#include "../base/Exception.h"

namespace avg {

CursorEvent::CursorEvent(int id, Type eventType, const IntPoint& position, Source source)
    : Event(eventType, source),
      m_Position(position),
      m_ID(id)
{
}

CursorEvent::~CursorEvent()
{
}

CursorEventPtr CursorEvent::cloneAs(Type eventType) const
{
    CursorEventPtr pClone(new CursorEvent(*this));
    pClone->m_Type = eventType;
    return pClone;
}

DPoint CursorEvent::getPos() const
{
    return DPoint(m_Position);
}

int CursorEvent::getXPosition() const
{
    return m_Position.x;
}

int CursorEvent::getYPosition() const
{
    return m_Position.y;
}

void CursorEvent::setCursorID(int id)
{
    m_ID = id;
}

int CursorEvent::getCursorID() const
{
    return m_ID;
}

bool operator ==(const CursorEvent& event1, const CursorEvent& event2)
{
    return (event1.m_Position == event2.m_Position && event1.m_When == event2.m_When); 
}

}

