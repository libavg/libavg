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

#ifndef _XInput21MTInputDevice_H_
#define _XInput21MTInputDevice_H_

#include "../api.h"
#include "MultitouchInputDevice.h"

#include "../base/Point.h"

#include <X11/Xlib.h>
#include <vector>
#include <string>

union SDL_Event;

namespace avg {

class AVG_API XInput21MTInputDevice: public MultitouchInputDevice
{
public:
    XInput21MTInputDevice();
    virtual ~XInput21MTInputDevice();
    virtual void start();

    void handleXIEvent(const XEvent& xEvent);
    std::vector<EventPtr> pollEvents();
    
private:
    void findMTDevice();
    TouchEventPtr createEvent(int id, Event::Type type, IntPoint pos);

    static int filterEvent(const SDL_Event * pEvent);

    int m_LastID;

    static Display* s_pDisplay;
    void (*m_SDLLockFunc)(void);
    void (*m_SDLUnlockFunc)(void);

    int m_XIOpcode;

    std::string m_sDeviceName;
    int m_DeviceID;

    int m_OldMasterDeviceID;
};

typedef boost::shared_ptr<XInput21MTInputDevice> XInput21MTInputDevicePtr;

}

#endif

