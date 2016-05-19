//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2014 Ulrich von Zadow
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

#ifndef _MouseWheelEvent_h_
#define _MouseWheelEvent_h_

#include "../api.h"

#include "CursorEvent.h"

#include "../base/GLMHelper.h"

namespace avg {

class AVG_API MouseWheelEvent : public CursorEvent {
    public:
        MouseWheelEvent(const IntPoint& pos, const glm::vec2& motion, int when=-1);
        virtual ~MouseWheelEvent();
        
        const glm::vec2& getMotion() const;
        virtual CursorEventPtr copy() const;
        virtual void trace();

    private:
        glm::vec2 m_Motion;
};

typedef boost::shared_ptr<class MouseWheelEvent> MouseWheelEventPtr;

}

#endif

