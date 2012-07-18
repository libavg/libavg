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

#ifndef _XInputMTInputDevice_H_
#define _XInputMTInputDevice_H_

#include "../api.h"
#include "../avgconfig.h"

#include "MultitouchInputDevice.h"

#include <gdk/gdk.h>

#include <X11/Xlib.h>

namespace avg {

class AVG_API XInputMTInputDevice: public MultitouchInputDevice
{
public:
    XInputMTInputDevice();
    virtual ~XInputMTInputDevice();
    virtual void start();
    std::vector<EventPtr> pollEvents();
    
    TouchStatusPtr getTouchStatusViaSeq(GdkEventSequence* id);
    void addTouchStatusViaSeq(GdkEventSequence* id, TouchEventPtr pInitialEvent);
    void removeTouchStatusViaSeq(GdkEventSequence* id);

private:
    void findMTDevice();

    static Display* s_pDisplay;

    int m_XIOpcode;

    std::string m_sDeviceName;
    int m_DeviceID;

    int m_OldMasterDeviceID;
};

typedef boost::shared_ptr<XInputMTInputDevice> XInputMTInputDevicePtr;

}

#endif

