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

#ifndef _TouchStatus_H_
#define _TouchStatus_H_

#include "TouchEvent.h"

namespace avg {

class TouchStatus {
public:
    TouchStatus(TouchEventPtr pEvent);
    virtual ~TouchStatus();

    const IntPoint& getLastDownPos();
    bool isFirstFrame();

    void updateEvent(TouchEventPtr pEvent);
    TouchEventPtr getEvent();
    Event::Type getEventType();

private:
    TouchEventPtr m_pEvent;
    TouchEventPtr m_pUpEvent;
    bool m_bFirstFrame;
    IntPoint m_LastDownPos;
    int m_CursorID;
};

typedef boost::shared_ptr<class TouchStatus> TouchStatusPtr;

}

#endif
