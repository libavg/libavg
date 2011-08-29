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
//  Original author of this file is igor@c-base.org
//

#ifndef _TouchEvent_H_
#define _TouchEvent_H_

#include "../api.h"
#include "CursorEvent.h"

#include "../imaging/Blob.h"
#include "../base/Point.h"

#include <math.h>
#include <boost/weak_ptr.hpp>

namespace avg {

class TouchEvent;
typedef boost::shared_ptr<class TouchEvent> TouchEventPtr;
typedef boost::weak_ptr<class TouchEvent> TouchEventWeakPtr;

class AVG_API TouchEvent: public CursorEvent 
{
    public:
        TouchEvent(int id, Type eventType, BlobPtr pBlob, const IntPoint& pos, 
                Source source, const DPoint& speed=DPoint(0,0));
        TouchEvent(int id, Type eventType, const IntPoint& pos, Source source, 
                const DPoint& speed, double orientation, double area, double eccentricity,
                DPoint majorAxis, DPoint minorAxis);
        TouchEvent(int id, Type eventType, const IntPoint& pos, Source source,
                const DPoint& speed=DPoint(0, 0));
        virtual ~TouchEvent();
        virtual CursorEventPtr cloneAs(Type eventType) const;

        double getOrientation() const;
        double getArea() const;
        const DPoint & getCenter() const;
        double getEccentricity() const;
        const DPoint & getMajorAxis() const;
        const DPoint & getMinorAxis() const;

        const BlobPtr getBlob() const;
        ContourSeq getContour();
        double getHandOrientation() const;

        void addRelatedEvent(TouchEventPtr pEvent);
        std::vector<TouchEventPtr> getRelatedEvents() const;

        void removeBlob();

        virtual void trace();

    private:
        BlobPtr m_pBlob;
        double m_Orientation;
        double m_Area;
        DPoint m_Center;
        double m_Eccentricity;
        DPoint m_MajorAxis;
        DPoint m_MinorAxis;
        std::vector<TouchEventWeakPtr> m_RelatedEvents;
        bool m_bHasHandOrientation;
        double m_HandOrientation; 
};

}
#endif
