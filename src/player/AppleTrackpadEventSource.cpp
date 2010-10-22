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
    // TODO:
    // - Consolidate events: one event per ID per frame.
    // - Keep an eventStream internally and create the vector<EventPtr> in this func.
    // - (This func should return a pointer to a vector to avoid copying the whole 
    //   vector.)
    boost::mutex::scoped_lock lock(*m_pMutex);
    vector<EventPtr> events = m_Events;
    m_Events.clear();
    return events;
}

void AppleTrackpadEventSource::onData(int device, Finger* pFingers, int numFingers, 
        double timestamp, int frame)
{
    boost::mutex::scoped_lock lock(*m_pMutex);
    for (int i = 0; i < numFingers; i++) {
        Event::Type eventType;
        Finger* pFinger = &pFingers[i];
        map<int, int>::iterator it = m_TouchIDs.find(pFinger->identifier);
        int avgID;
        if (it == m_TouchIDs.end()) {
            eventType = Event::CURSORDOWN;
            m_LastID++;
            m_TouchIDs[pFinger->identifier] = m_LastID;
            avgID = m_LastID;
        } else if (pFinger->state == 7) {
            eventType = Event::CURSORUP;
            avgID = it->second;
            m_TouchIDs.erase(it);
        } else {
            eventType = Event::CURSORMOTION;
            avgID = it->second;
        }
        // TODO: 
        // - Keep lastDownPos
        // - Calc majorAxis, minorAxis from axis+angle
        IntPoint pos(pFinger->normalized.pos.x*m_WindowSize.x, 
                (1-pFinger->normalized.pos.y)*m_WindowSize.y);
        DPoint speed(pFinger->normalized.vel.x*m_WindowSize.x, 
                pFinger->normalized.vel.y*m_WindowSize.y);
        IntPoint lastDownPos(0, 0);
        double eccentricity = pFinger->majorAxis/pFinger->minorAxis;
        EventPtr pEvent(new TouchEvent(avgID, eventType, pos, Event::TOUCH, speed, 
                lastDownPos, pFinger->angle, pFinger->size, eccentricity, DPoint(0,0), 
                DPoint(0,0)));
        m_Events.push_back(pEvent);
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

}
