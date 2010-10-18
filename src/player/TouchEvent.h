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

#ifndef _TouchEvent_H_
#define _TouchEvent_H_

#include "../api.h"
#include "CursorEvent.h"
#include "VisibleNode.h"

#include "../imaging/Blob.h"
#include "../graphics/Bitmap.h"
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
                Source source, const DPoint& speed, const IntPoint& lastDownPos);
        virtual ~TouchEvent();
        virtual CursorEventPtr cloneAs(Type eventType) const;

        const DPoint& getSpeed() const;
        double getOrientation() const;
        double getArea() const;
        double getInertia() const;
        const DPoint & getCenter() const;
        double getEccentricity() const;
        const DPoint & getMajorAxis() const;
        const DPoint & getMinorAxis() const;

        const BlobPtr getBlob() const;
        ContourSeq getContour();

        void addRelatedEvent(TouchEventPtr pEvent);
        std::vector<TouchEventPtr> getRelatedEvents() const;

        virtual void trace();
    
    private:
        BlobPtr m_pBlob;
        DPoint m_Speed;
        double m_Orientation;
        double m_Area;
        double m_Inertia;
        DPoint m_Center;
        double m_Eccentricity;
        DPoint m_MajorAxis;
        DPoint m_MinorAxis;
        std::vector<TouchEventWeakPtr> m_RelatedEvents; 
};

}
#endif
