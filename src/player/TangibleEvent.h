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

#ifndef _TangibleEvent_H_
#define _TangibleEvent_H_

#include "../api.h"
#include "CursorEvent.h"

#include <boost/weak_ptr.hpp>

namespace avg {

class TangibleEvent;
typedef boost::shared_ptr<class TangibleEvent> TangibleEventPtr;
typedef boost::weak_ptr<class TangibleEvent> TangibleEventWeakPtr;

class AVG_API TangibleEvent: public CursorEvent 
{
    public:
        TangibleEvent(int id, int markerID, Type eventType, const IntPoint& pos, 
                const glm::vec2& speed, float orientation);
        virtual ~TangibleEvent();
        virtual CursorEventPtr cloneAs(Type eventType) const;

        int getMarkerID() const;
        float getOrientation() const;

        virtual void trace();

    private:
        int m_MarkerID;
        float m_Orientation;
};

}
#endif
