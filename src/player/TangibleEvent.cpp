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

#include "TangibleEvent.h"

#include "../base/Exception.h"
#include "../base/Logger.h"

using namespace std;

namespace avg {

TangibleEvent::TangibleEvent(int id, int markerID, Type eventType, const IntPoint& pos, 
                const glm::vec2& speed, float orientation)
    : CursorEvent(id, eventType, pos, Event::TANGIBLE),
      m_MarkerID(markerID),
      m_Orientation(orientation)
{
    setSpeed(speed);
}

TangibleEvent::~TangibleEvent()
{
}

CursorEventPtr TangibleEvent::cloneAs(Type eventType) const
{
    TangibleEventPtr pClone(new TangibleEvent(*this));
    pClone->m_Type = eventType;
    return pClone;
}

int TangibleEvent::getMarkerID() const
{
    return m_MarkerID;
}

float TangibleEvent::getOrientation() const 
{
    return m_Orientation;
}

void TangibleEvent::trace()
{
    CursorEvent::trace();
    AVG_TRACE(Logger::category::EVENTS, Logger::severity::DEBUG, "pos: " << getPos() 
            << ", ID: " << getCursorID()
            << ", Marker ID: " << m_MarkerID);
}
      
}

