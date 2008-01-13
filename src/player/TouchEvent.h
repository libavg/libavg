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

#ifndef _TouchEvent_H_
#define _TouchEvent_H_

#include "CursorEvent.h"
#include "Node.h"

#include "../imaging/Blob.h"
#include "../graphics/Bitmap.h"
#include "../base/Point.h"

#include <math.h>

namespace avg {

class TouchEvent: public CursorEvent 
{
    public:
        TouchEvent(int id, Type EventType, BlobPtr pBlob, IntPoint& Pos, Source source);
        virtual ~TouchEvent();
        virtual Event* cloneAs(Type EventType);

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

        void addRelatedEvent(TouchEvent * pEvent);
        std::vector<TouchEvent *> getRelatedEvents() const;

        virtual void trace();
    
    private:
        BlobPtr m_pBlob;
        std::vector<TouchEvent *> m_RelatedEvents; 
};

}
#endif
