//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2020 Ulrich von Zadow
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

#include "Node.h"
#include "Contact.h"

#include "../base/Exception.h"
#include "../base/Logger.h"

#include <iostream>

using namespace std;

namespace avg {

CursorEvent::CursorEvent(int id, Type eventType, const IntPoint& pos, Source source,
        int when)
    : Event(eventType, source, when),
      m_Pos(pos),
      m_ID(id),
      m_UserID(-1),
      m_JointID(-1),
      m_Speed(0,0)
{
}

CursorEvent::~CursorEvent()
{
}

CursorEventPtr CursorEvent::copy() const
{
    return CursorEventPtr(new CursorEvent(*this));
}

CursorEventPtr CursorEvent::cloneAs(Type eventType) const
{
    CursorEventPtr pClone = copy();
    if (eventType != UNKNOWN) {
        pClone->m_Type = eventType;
    }
    return pClone;
}

CursorEventPtr CursorEvent::cloneAs(Type eventType, const glm::vec2& pos) const
{
    CursorEventPtr pClone = cloneAs(eventType);
    pClone->m_Pos = IntPoint(pos);
    return pClone;
}

void CursorEvent::setUserID(int userID, int jointID)
{
    m_UserID = userID;
    m_JointID = jointID;
}

glm::vec2 CursorEvent::getPos() const
{
    return glm::vec2(m_Pos);
}

int CursorEvent::getXPosition() const
{
    return m_Pos.x;
}

int CursorEvent::getYPosition() const
{
    return m_Pos.y;
}

void CursorEvent::setCursorID(int id)
{
    m_ID = id;
}

int CursorEvent::getCursorID() const
{
    return m_ID;
}

int CursorEvent::getUserID() const
{
    return m_UserID;
}

int CursorEvent::getJointID() const
{
    return m_JointID;
}

void CursorEvent::setNode(NodePtr pNode)
{
    m_pNode = pNode;
}

void CursorEvent::clearNodeData()
{
    m_pNode = NodePtr();
}

NodePtr CursorEvent::getNode() const
{
    return m_pNode;
}
        
void CursorEvent::setSpeed(glm::vec2 speed)
{
    m_Speed = speed;
}

const glm::vec2& CursorEvent::getSpeed() const
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
    return (event1.m_Pos == event2.m_Pos && 
            event1.getWhen() == event2.getWhen()); 
}

void CursorEvent::trace()
{
    AVG_TRACE(Logger::category::EVENTS, Logger::severity::DEBUG,
            typeStr() << ", User ID: " << m_UserID << ", Joint ID: " << m_JointID);
}

}

