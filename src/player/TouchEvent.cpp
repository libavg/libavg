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

#include "TouchEvent.h"

#include "../graphics/Bitmap.h"
#include "../graphics/Filterfill.h"
#include "../graphics/Pixel8.h"

#include "../base/Logger.h"

using namespace std;

namespace avg {

TouchEvent::TouchEvent(int id, Type EventType, BlobPtr pBlob, const IntPoint& Pos, Source source)
    : CursorEvent(id, EventType, Pos, source),
      m_pBlob(pBlob)
{
}

TouchEvent::~TouchEvent()
{
}

CursorEventPtr TouchEvent::cloneAs(Type EventType) const
{
    TouchEventPtr pClone(new TouchEvent(*this));
    pClone->m_Type = EventType;
    return pClone;
}

const BlobPtr TouchEvent::getBlob() const
{
    return m_pBlob;
}

const DPoint & TouchEvent::getMajorAxis() const
{
    const DPoint & Axis0 = m_pBlob->getScaledBasis(0);
    const DPoint & Axis1 = m_pBlob->getScaledBasis(1);
    if (calcDist(Axis0, DPoint(0,0)) > calcDist(Axis1, DPoint(0,0))) {
        return Axis0;
    } else {
        return Axis1;
    }
}

const DPoint & TouchEvent::getMinorAxis() const
{
    const DPoint & Axis0 = m_pBlob->getScaledBasis(0);
    const DPoint & Axis1 = m_pBlob->getScaledBasis(1);
    if (calcDist(Axis0, DPoint(0,0)) > calcDist(Axis1, DPoint(0,0))) {
        return Axis1;
    } else {
        return Axis0;
    }
}

ContourSeq TouchEvent::getContour()
{
    return m_pBlob->getContour();
}

void TouchEvent::addRelatedEvent(TouchEventPtr pEvent)
{
    m_RelatedEvents.push_back(pEvent);
}

vector<TouchEventPtr> TouchEvent::getRelatedEvents() const
{
    vector<TouchEventPtr> pRelatedEvents;
    vector<TouchEventWeakPtr>::const_iterator it;
    for (it=m_RelatedEvents.begin(); it != m_RelatedEvents.end(); ++it) {
        pRelatedEvents.push_back((*it).lock());
    }
    return pRelatedEvents;
}

void TouchEvent::trace()
{
    Event::trace();
    AVG_TRACE(Logger::EVENTS2, "pos: " << m_Position 
            << ", ID: " << getCursorID()
            << ", Area: " << m_pBlob->getArea()
            << ", Eccentricity: " << m_pBlob->getEccentricity());
}
      
}

