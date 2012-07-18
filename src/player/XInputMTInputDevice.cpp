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

#include "XInputMTInputDevice.h"

#include "Player.h"
#include "GDKDisplayEngine.h"

#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/StringHelper.h"

#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>

#include <boost/functional/hash.hpp>

using namespace std;

namespace avg {

Display* XInputMTInputDevice::s_pDisplay = 0;

XInputMTInputDevice::XInputMTInputDevice()
{
}

XInputMTInputDevice::~XInputMTInputDevice()
{
}

void XInputMTInputDevice::start()
{
    Status status;
    GDKDisplayEngine * pEngine = Player::get()->getDisplayEngine();
    s_pDisplay = gdk_x11_display_get_xdisplay(pEngine->getDisplay());

    // XInput Extension available?
    int event, error;
    bool bOk = XQueryExtension(s_pDisplay, "XInputExtension", &m_XIOpcode,
            &event, &error);
    if (!bOk) {
        throw Exception(AVG_ERR_MT_INIT,
                "XiInput mutiouch event source: X Input extension not avaiable.");
    }
    // Which version of XI2? 
    int major;
    int minor;
    status = XIQueryVersion(s_pDisplay, &major, &minor);
    if (status == BadRequest) {
        throw Exception(AVG_ERR_MT_INIT, 
                "XInput 2.1 multitouch event source: Server does not support XI2");
    }
    if (major < 2 || minor < 1) {
        throw Exception(AVG_ERR_MT_INIT, 
                "XInput multitouch event source: Supported version is "
                +toString(major)+"."+toString(minor)+". At least 2.1 is needed.");
    }

    findMTDevice();

    pEngine->setXIMTInputDevice(this);
    MultitouchInputDevice::start();
    AVG_TRACE(Logger::CONFIG, "XInput Multitouch event source created.");
}

std::vector<EventPtr> XInputMTInputDevice::pollEvents()
{
    return MultitouchInputDevice::pollEvents();
}

TouchStatusPtr XInputMTInputDevice::getTouchStatusViaSeq(GdkEventSequence* id)
{
    return getTouchStatus(boost::hash<GdkEventSequence*>()(id));
}

void XInputMTInputDevice::addTouchStatusViaSeq(GdkEventSequence* id, TouchEventPtr pInitialEvent)
{
    addTouchStatus(boost::hash<GdkEventSequence*>()(id), pInitialEvent);
}

void XInputMTInputDevice::removeTouchStatusViaSeq(GdkEventSequence* id)
{
    removeTouchStatus(boost::hash<GdkEventSequence*>()(id));
}

void XInputMTInputDevice::findMTDevice()
{
    int ndevices;
    XIDeviceInfo* pDevices;
    XIDeviceInfo* pDevice;

    pDevices = XIQueryDevice(s_pDisplay, XIAllDevices, &ndevices);

    XITouchClassInfo* pTouchClass = 0;
    int maxTouches;
    for (int i = 0; i < ndevices && !pTouchClass; ++i) {
        pDevice = &pDevices[i];
        if (pDevice->use == XISlavePointer || pDevice->use == XIFloatingSlave) {
            for (int j = 0; j < pDevice->num_classes; ++j) {
                XIAnyClassInfo * pClass = pDevice->classes[j];
                if (pClass->type == XITouchClass) {
                    XITouchClassInfo* pTempTouchClass = (XITouchClassInfo *)pClass;
                    if (pTempTouchClass->mode == XIDirectTouch) {
                        pTouchClass = pTempTouchClass;
                        m_sDeviceName = pDevice->name;
                        m_DeviceID = pDevice->deviceid;
                        if (pDevice->use == XISlavePointer) {
                            m_OldMasterDeviceID = pDevice->attachment;
                        } else {
                            m_OldMasterDeviceID = -1;
                        }
                        maxTouches = pTouchClass->num_touches;
                        break;
                    }
                }
            }
        }
    }
    if (pTouchClass) {
        AVG_TRACE(Logger::CONFIG, "Using multitouch input device " << m_sDeviceName 
                << ", max touches: " << maxTouches);
    } else {
        throw Exception(AVG_ERR_MT_INIT, 
                "XInput multitouch event source: No multitouch device found.");
    }
    XIFreeDeviceInfo(pDevices);
}

// From xinput/test_xi2.c
const char* cookieTypeToName(int evtype)
{
    const char *name;
    switch(evtype) {
        case XI_DeviceChanged:    name = "DeviceChanged";        break;
        case XI_KeyPress:         name = "KeyPress";             break;
        case XI_KeyRelease:       name = "KeyRelease";           break;
        case XI_ButtonPress:      name = "ButtonPress";          break;
        case XI_ButtonRelease:    name = "ButtonRelease";        break;
        case XI_Motion:           name = "Motion";               break;
        case XI_Enter:            name = "Enter";                break;
        case XI_Leave:            name = "Leave";                break;
        case XI_FocusIn:          name = "FocusIn";              break;
        case XI_FocusOut:         name = "FocusOut";             break;
        case XI_HierarchyChanged: name = "HierarchyChanged";     break;
        case XI_PropertyEvent:    name = "PropertyEvent";        break;
        case XI_RawKeyPress:      name = "RawKeyPress";          break;
        case XI_RawKeyRelease:    name = "RawKeyRelease";        break;
        case XI_RawButtonPress:   name = "RawButtonPress";       break;
        case XI_RawButtonRelease: name = "RawButtonRelease";     break;
        case XI_RawMotion:        name = "RawMotion";            break;
        case XI_TouchBegin:       name = "TouchBegin";           break;
        case XI_TouchEnd:         name = "TouchEnd";             break;
        case XI_TouchUpdate:      name = "TouchUpdate";          break;
#ifdef HAVE_XI2_1  
        case XI_TouchUpdateUnowned: name = "TouchUpdateUnowned"; break;
#endif
        default:                  name = "unknown event type";   break;
    }
    return name;
}

string xEventTypeToName(int evtype)
{
    switch(evtype) {
        case KeyPress:
            return "KeyPress";
        case KeyRelease:
            return "KeyRelease";
        case ButtonPress:
            return "ButtonPress";
        case ButtonRelease:
            return "ButtonRelease";
        case MotionNotify:
            return "MotionNotify";
        case EnterNotify:
            return "EnterNotify";
        case LeaveNotify:
            return "LeaveNotify";
        case FocusIn:
            return "FocusIn";
        case FocusOut:
            return "FocusOut";
        case KeymapNotify:
            return "KeymapNotify";
        case Expose:
            return "Expose";
        case GraphicsExpose:
            return "GraphicsExpose";
        case NoExpose:
            return "NoExpose";
        case VisibilityNotify:
            return "VisibilityNotify";
        case CreateNotify:
            return "CreateNotify";
        case DestroyNotify:
            return "DestroyNotify";
        case UnmapNotify:
            return "UnmapNotify";
        case MapNotify:
            return "MapNotify";
        case MapRequest:
            return "MapRequest";
        case ReparentNotify:
            return "ReparentNotify";
        case ConfigureNotify:
            return "ConfigureNotify";
        case ConfigureRequest:
            return "ConfigureRequest";
        case GravityNotify:
            return "GravityNotify";
        case ResizeRequest:
            return "ResizeRequest";
        case CirculateNotify:
            return "CirculateNotify";
        case CirculateRequest:
            return "CirculateRequest";
        case PropertyNotify:
            return "PropertyNotify";
        case SelectionClear:
            return "SelectionClear";
        case SelectionRequest:
            return "SelectionRequest";
        case SelectionNotify:
            return "SelectionNotify";
        case ColormapNotify:
            return "ColormapNotify";
        case ClientMessage:
            return "ClientMessage";
        case MappingNotify:
            return "MappingNotify";
        case GenericEvent:
            return "GenericEvent";
        default:
            return "Unknown event type";
    }
}

}
