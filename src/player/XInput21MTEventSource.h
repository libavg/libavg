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

#ifndef _XInput21MTEventSource_H_
#define _XInput21MTEventSource_H_

#include "../api.h"
#include "MultitouchEventSource.h"

#include "../base/Point.h"

#include <X11/Xlib.h>
#include <vector>

struct mtdev;

namespace avg {

class AVG_API XInput21MTEventSource: public MultitouchEventSource
{
public:
    XInput21MTEventSource();
    virtual ~XInput21MTEventSource();
    virtual void start();

    void handleXIEvent(const XEvent& xEvent);
    std::vector<EventPtr> pollEvents();
    
private:
    TouchEventPtr createEvent(int id, Event::Type type, IntPoint pos);
    void dumpEvent(const XEvent& xEvent);

    int m_LastID;

    Display* m_pDisplay;
    Window m_Win;
    int m_XIOpcode;
};

typedef boost::shared_ptr<XInput21MTEventSource> XInput21MTEventSourcePtr;

}

#endif

