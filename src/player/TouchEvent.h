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
        TouchEvent(int id, Type EventType, BlobPtr pBlob, const IntPoint& Pos, 
                Source source, const DPoint& speed, const IntPoint& lastDownPos);
        virtual ~TouchEvent();
        virtual CursorEventPtr cloneAs(Type EventType) const;

        const DPoint& getSpeed() const;
        double getOrientation() const {return m_pBlob->getOrientation();};
        double getArea() const {return m_pBlob->getArea();};
        double getInertia() const {return m_pBlob->getInertia();};
        const DPoint & getCenter() const {return m_pBlob->getCenter();};
        double getEccentricity() const {return m_pBlob->getEccentricity();};
        const DPoint & getEigenValues() const {return m_pBlob->getEigenValues();};
        const BlobPtr getBlob() const;
        const DPoint & getMajorAxis() const;
        const DPoint & getMinorAxis() const;
        ContourSeq getContour();

        void addRelatedEvent(TouchEventPtr pEvent);
        std::vector<TouchEventPtr> getRelatedEvents() const;

        virtual void trace();
    
    private:
        BlobPtr m_pBlob;
        DPoint m_Speed;
        std::vector<TouchEventWeakPtr> m_RelatedEvents; 
};

}
#endif
