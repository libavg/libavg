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

#include "AppleTrackpadInputDevice.h"
#include "TouchEvent.h"
#include "Player.h"
#include "AVGNode.h"
#include "TouchStatus.h"

#include "../base/Logger.h"
#include "../base/ObjectCounter.h"
#include "../base/Exception.h"

#include <CoreFoundation/CoreFoundation.h>

using namespace std;

namespace avg {

AppleTrackpadInputDevice* AppleTrackpadInputDevice::s_pInstance(0);

AppleTrackpadInputDevice::AppleTrackpadInputDevice()
    : m_LastID(0)
{
    s_pInstance = this;
}

AppleTrackpadInputDevice::~AppleTrackpadInputDevice()
{
    MTDeviceStop(m_Device);
    MTUnregisterContactFrameCallback(m_Device, callback);
    MTDeviceRelease(m_Device);
    s_pInstance = 0;
}

void AppleTrackpadInputDevice::start()
{
    MultitouchInputDevice::start();
    m_Device = MTDeviceCreateDefault();
    MTRegisterContactFrameCallback(m_Device, callback);
    MTDeviceStart(m_Device, 0);
    AVG_TRACE(Logger::category::CONFIG, Logger::severity::INFO,
            "Apple Trackpad Multitouch event source created.");
}

void AppleTrackpadInputDevice::onData(int device, Finger* pFingers, int numFingers, 
        float timestamp, int frame)
{
    boost::mutex::scoped_lock lock(getMutex());
    for (int i = 0; i < numFingers; i++) {
        Finger* pFinger = &pFingers[i];
        TouchStatusPtr pTouchStatus = getTouchStatus(pFinger->identifier);
        if (!pTouchStatus) {
            m_LastID++;
            TouchEventPtr pEvent = createEvent(m_LastID, pFinger, Event::CURSOR_DOWN);
            addTouchStatus(pFinger->identifier, pEvent);
        } else {
            Event::Type eventType;
            if (pFinger->state == 7) {
                eventType = Event::CURSOR_UP;
                removeTouchStatus(pFinger->identifier);
            } else {
                eventType = Event::CURSOR_MOTION;
            }
            TouchEventPtr pEvent = createEvent(0, pFinger, eventType);
            pTouchStatus->pushEvent(pEvent);
        }
    }
}

int AppleTrackpadInputDevice::callback(int device, Finger *data, int nFingers, 
        double timestamp, int frame) 
{
    AVG_ASSERT(s_pInstance != 0);
    s_pInstance->onData(device, data, nFingers, timestamp, frame);
    return 0;
}

TouchEventPtr AppleTrackpadInputDevice::createEvent(int avgID, Finger* pFinger, 
        Event::Type eventType)
{
    glm::vec2 size = getTouchArea();
    IntPoint pos = getScreenPos(glm::vec2(pFinger->normalized.pos.x,
            1-pFinger->normalized.pos.y));
    glm::vec2 speed(pFinger->normalized.vel.x*size.x, pFinger->normalized.vel.y*size.y);
    float eccentricity = pFinger->majorAxis/pFinger->minorAxis;
    glm::vec2 majorAxis = fromPolar(pFinger->angle, pFinger->majorAxis);
    majorAxis.y = -majorAxis.y;
    glm::vec2 minorAxis = fromPolar(pFinger->angle+1.57, pFinger->minorAxis);
    minorAxis.y = -minorAxis.y;

    TouchEventPtr pEvent(new TouchEvent(avgID, eventType, pos, Event::TOUCH,
                speed, pFinger->angle, pFinger->size, eccentricity, majorAxis, 
                minorAxis)); 
    return pEvent;
}

}
