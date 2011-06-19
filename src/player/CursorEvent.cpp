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
#include "Contact.h"

#include "../base/Exception.h"
#include "../base/Logger.h"

#include <iostream>

using namespace std;

namespace avg {

CursorEvent::CursorEvent(int id, Type eventType, const IntPoint& position, Source source,
        int when)
    : Event(eventType, source, when),
      m_Position(position),
      m_ID(id),
      m_Speed(0,0)
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

void CursorEvent::setPos(const DPoint& pos)
{
    m_Position = IntPoint(pos);
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

void CursorEvent::setNode(VisibleNodePtr pNode)
{
    m_pNode = pNode;
}

VisibleNodePtr CursorEvent::getNode() const
{
    return m_pNode;
}
        
void CursorEvent::setSpeed(DPoint speed)
{
    m_Speed = speed;
}

const DPoint& CursorEvent::getSpeed() const
{
    return m_Speed;
}

void CursorEvent::setContact(ContactPtr pContact)
{
    m_pContact = pContact;
}

ContactPtr CursorEvent::getContact() const
{
    return m_pContact.lock();
}

bool operator ==(const CursorEvent& event1, const CursorEvent& event2)
{
    return (event1.m_Position == event2.m_Position && 
            event1.getWhen() == event2.getWhen()); 
}

void CursorEvent::trace()
{
    string sType = typeStr();
    if (!m_pNode) {
        AVG_TRACE(Logger::EVENTS, sType); 
    } else {
        AVG_TRACE(Logger::EVENTS, m_pNode->getID()+", "+sType); 
    }
}

}

