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

#include "TouchEvent.h"

#include "../graphics/Bitmap.h"
#include "../graphics/Filterfill.h"
#include "../graphics/Pixel8.h"

#include "../base/Logger.h"

using namespace std;

namespace avg {

TouchEvent::TouchEvent(int id, Type eventType, BlobPtr pBlob, const IntPoint& pos, 
        Source source, const DPoint& speed, const IntPoint& lastDownPos)
    : CursorEvent(id, eventType, pos, source),
      m_pBlob(pBlob),
      m_Speed(speed)
{
    setLastDownPos(lastDownPos);
}

TouchEvent::~TouchEvent()
{
}

CursorEventPtr TouchEvent::cloneAs(Type eventType) const
{
    TouchEventPtr pClone(new TouchEvent(*this));
    pClone->m_Type = eventType;
    return pClone;
}

const DPoint& TouchEvent::getSpeed() const
{
    return m_Speed;
}

double TouchEvent::getOrientation() const 
{
    return m_pBlob->getOrientation();
}

double TouchEvent::getArea() const 
{
    return m_pBlob->getArea();
}

double TouchEvent::getInertia() const
{
    return m_pBlob->getInertia();
}

const DPoint & TouchEvent::getCenter() const 
{
    return m_pBlob->getCenter();
}

double TouchEvent::getEccentricity() const 
{
    return m_pBlob->getEccentricity();
}

const BlobPtr TouchEvent::getBlob() const
{
    return m_pBlob;
}

const DPoint & TouchEvent::getMajorAxis() const
{
    const DPoint & axis0 = m_pBlob->getScaledBasis(0);
    const DPoint & axis1 = m_pBlob->getScaledBasis(1);
    if (calcDist(axis0, DPoint(0,0)) > calcDist(axis1, DPoint(0,0))) {
        return axis0;
    } else {
        return axis1;
    }
}

const DPoint & TouchEvent::getMinorAxis() const
{
    const DPoint & axis0 = m_pBlob->getScaledBasis(0);
    const DPoint & axis1 = m_pBlob->getScaledBasis(1);
    if (calcDist(axis0, DPoint(0,0)) > calcDist(axis1, DPoint(0,0))) {
        return axis1;
    } else {
        return axis0;
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
    for (it = m_RelatedEvents.begin(); it != m_RelatedEvents.end(); ++it) {
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

