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

#include "AppleTrackpadEventSource.h"
#include "TouchEvent.h"
#include "Player.h"
#include "AVGNode.h"
#include "Touch.h"

#include "../base/Logger.h"
#include "../base/ObjectCounter.h"
#include "../base/Exception.h"

#include <CoreFoundation/CoreFoundation.h>

using namespace std;

namespace avg {

AppleTrackpadEventSource* AppleTrackpadEventSource::s_pInstance(0);

AppleTrackpadEventSource::AppleTrackpadEventSource()
    : m_LastID(0)
{
    s_pInstance = this;
}

AppleTrackpadEventSource::~AppleTrackpadEventSource()
{
    MTDeviceStop(m_Device);
    MTUnregisterContactFrameCallback(m_Device, callback);
    MTDeviceRelease(m_Device);
    s_pInstance = 0;
}

void AppleTrackpadEventSource::start()
{
    m_WindowSize = Player::get()->getRootNode()->getSize();
    m_pMutex = MutexPtr(new boost::mutex);
    m_Device = MTDeviceCreateDefault();
    MTRegisterContactFrameCallback(m_Device, callback);
    MTDeviceStart(m_Device, 0);
}

vector<EventPtr> AppleTrackpadEventSource::pollEvents()
{
    boost::mutex::scoped_lock lock(*m_pMutex);

    vector<EventPtr> events;
    map<int, TouchPtr>::iterator it;
    for (it = m_Touches.begin(); it != m_Touches.end(); ) {
        TouchPtr pTouch = it->second;
        TouchEventPtr pEvent = pTouch->getEvent();
        if (pEvent) {
            events.push_back(pEvent);
            if (pEvent->getType() == Event::CURSORUP) {
                m_Touches.erase(it++);
            } else {
                ++it;
            }
        } else {
            ++it;
        }
    }

    return events;
}

void AppleTrackpadEventSource::onData(int device, Finger* pFingers, int numFingers, 
        double timestamp, int frame)
{
    boost::mutex::scoped_lock lock(*m_pMutex);
    for (int i = 0; i < numFingers; i++) {
        Finger* pFinger = &pFingers[i];
        map<int, TouchPtr>::iterator it = m_Touches.find(pFinger->identifier);
        if (it == m_Touches.end()) {
            m_LastID++;
            TouchEventPtr pEvent = createEvent(m_LastID, pFinger, Event::CURSORDOWN);
            TouchPtr pTouch(new Touch(pEvent));
            m_Touches[pFinger->identifier] = pTouch;
        } else {
            TouchPtr pTouch = it->second;
            Event::Type eventType;
            if (pFinger->state == 7) {
                eventType = Event::CURSORUP;
            } else {
                eventType = Event::CURSORMOTION;
            }
            TouchEventPtr pEvent = createEvent(0, pFinger, eventType);
            pTouch->updateEvent(pEvent);
        }
/*        
        printf("Frame %7d: Angle %6.2f, ellipse %6.3f x%6.3f; "
                "position (%6.3f,%6.3f) vel (%6.3f,%6.3f) "
                "ID %d, state %d [%d %d?] size %6.3f, %6.3f?\n",
                f->frame,
                f->angle * 90 / atan2(1,0),
                f->majorAxis,
                f->minorAxis,
                f->normalized.pos.x,
                f->normalized.pos.y,
                f->normalized.vel.x,
                f->normalized.vel.y,
                f->identifier, f->state, f->foo3, f->foo4,
                f->size, f->unk2);
*/        
    }
/*    
    set<int>::iterator it;
    for (it = m_TouchIDs.begin(); it != m_TouchIDs.end(); ++it) {
        cerr << *it << " ";
    }
    cerr << endl;
*/
}

int AppleTrackpadEventSource::callback(int device, Finger *data, int nFingers, 
        double timestamp, int frame) 
{
    AVG_ASSERT(s_pInstance != 0);
    s_pInstance->onData(device, data, nFingers, timestamp, frame);
    return 0;
}

TouchEventPtr AppleTrackpadEventSource::createEvent(int avgID, Finger* pFinger, 
        Event::Type eventType)
{
    // TODO: 
    // - Calc majorAxis, minorAxis from axis+angle
    IntPoint pos(pFinger->normalized.pos.x*m_WindowSize.x, 
            (1-pFinger->normalized.pos.y)*m_WindowSize.y);
    DPoint speed(pFinger->normalized.vel.x*m_WindowSize.x, 
            pFinger->normalized.vel.y*m_WindowSize.y);
    double eccentricity = pFinger->majorAxis/pFinger->minorAxis;
    TouchEventPtr pEvent(new TouchEvent(avgID, eventType, pos, Event::TOUCH, speed, 
                pFinger->angle, pFinger->size, eccentricity, DPoint(0,0), 
                DPoint(0,0)));
    return pEvent;
}

}
