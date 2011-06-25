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

#include "Player.h"
#include "AVGNode.h"

#include "../graphics/Bitmap.h"
#include "../graphics/Filterfill.h"
#include "../graphics/Pixel8.h"

#include "../base/Logger.h"

using namespace std;

namespace avg {

TouchEvent::TouchEvent(int id, Type eventType, BlobPtr pBlob, const IntPoint& pos, 
        Source source, const DPoint& speed)
    : CursorEvent(id, eventType, pos, source),
      m_pBlob(pBlob),
      m_bHasHandOrientation(false)
{
    setSpeed(speed);
    if (pBlob) {
        m_Orientation = pBlob->getOrientation();
        m_Area = pBlob->getArea();
        m_Center = pBlob->getCenter();
        m_Eccentricity = pBlob->getEccentricity();
        const DPoint& axis0 = m_pBlob->getScaledBasis(0);
        const DPoint& axis1 = m_pBlob->getScaledBasis(1);
        if (axis0.getNorm() > axis1.getNorm()) {
            m_MajorAxis = axis0;
            m_MinorAxis = axis1;
        } else {
            m_MajorAxis = axis1;
            m_MinorAxis = axis0;
        }
    } else {
        m_Orientation = 0;
        m_Area = 20;
        m_Center = DPoint(0, 0);
        m_Eccentricity = 0;
        m_MajorAxis = DPoint(5, 0);
        m_MinorAxis = DPoint(0, 5);
    }
}

TouchEvent::TouchEvent(int id, Type eventType, const IntPoint& pos, Source source, 
        const DPoint& speed, double orientation, double area, double eccentricity, 
        DPoint majorAxis, DPoint minorAxis)
    : CursorEvent(id, eventType, pos, source),
      m_Orientation(orientation),
      m_Area(area),
      m_Eccentricity(eccentricity),
      m_MajorAxis(majorAxis),
      m_MinorAxis(minorAxis)
{
    setSpeed(speed);
}

TouchEvent::TouchEvent(int id, Type eventType, const IntPoint& pos, Source source,
        const DPoint& speed)
    : CursorEvent(id, eventType, pos, source),
      m_Orientation(0),
      m_Area(20),
      m_Eccentricity(0),
      m_MajorAxis(5, 0),
      m_MinorAxis(0, 5)
{
    setSpeed(speed);
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

double TouchEvent::getOrientation() const 
{
    return m_Orientation;
}

double TouchEvent::getArea() const 
{
    return m_Area;
}

const DPoint & TouchEvent::getCenter() const 
{
    return m_Center;
}

double TouchEvent::getEccentricity() const 
{
    return m_Eccentricity;
}

const DPoint & TouchEvent::getMajorAxis() const
{
    return m_MajorAxis;
}

const DPoint & TouchEvent::getMinorAxis() const
{
    return m_MinorAxis;
}

const BlobPtr TouchEvent::getBlob() const
{
    return m_pBlob;
}

ContourSeq TouchEvent::getContour()
{
    if (m_pBlob) {
        return m_pBlob->getContour();
    } else {
        throw Exception(AVG_ERR_UNSUPPORTED, 
                "TouchEvent::getContour: No contour available.");
    }
}

double TouchEvent::getHandOrientation() const
{
    if (getSource() == Event::TOUCH) {
        if (m_bHasHandOrientation) {
            return m_HandOrientation;
        } else {
            DPoint screenCenter = Player::get()->getRootNode()->getSize()/2;
            return (getPos()-screenCenter).getAngle();
        }
    } else {
        throw Exception(AVG_ERR_UNSUPPORTED,
                "TouchEvent::getHandOrientation: Only supported for touch events.");
    }
}

void TouchEvent::addRelatedEvent(TouchEventPtr pEvent)
{
    m_RelatedEvents.push_back(pEvent);
    if (getSource() == Event::TOUCH && m_RelatedEvents.size() == 1) {
        TouchEventPtr pHandEvent = m_RelatedEvents.begin()->lock();
        m_HandOrientation = (pHandEvent->getPos()-getPos()).getAngle();
        m_bHasHandOrientation = true;
    }
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

void TouchEvent::removeBlob()
{
    m_pBlob = BlobPtr();
}

void TouchEvent::trace()
{
    CursorEvent::trace();
    AVG_TRACE(Logger::EVENTS2, "pos: " << getPos() 
            << ", ID: " << getCursorID()
            << ", Area: " << m_Area
            << ", Eccentricity: " << m_Eccentricity);
}
      
}

