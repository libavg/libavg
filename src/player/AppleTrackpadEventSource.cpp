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

#include "../base/Logger.h"
#include "../base/ObjectCounter.h"
#include "../base/Exception.h"

#include <CoreFoundation/CoreFoundation.h>

using namespace std;

namespace avg {

AppleTrackpadEventSource* AppleTrackpadEventSource::s_pInstance(0);

AppleTrackpadEventSource::AppleTrackpadEventSource()
{
    s_pInstance = this;
    cerr << this << endl;
}

AppleTrackpadEventSource::~AppleTrackpadEventSource()
{
    s_pInstance = 0;
    cerr << "~" << this << endl;
    MTDeviceStop(m_Device);
}

void AppleTrackpadEventSource::start()
{
    cerr << "start " << this << endl;
    m_Device = MTDeviceCreateDefault();
    cerr << m_Device << endl;
    MTRegisterContactFrameCallback(m_Device, callback);
    MTDeviceStart(m_Device, 0);
}

vector<EventPtr> AppleTrackpadEventSource::pollEvents()
{
    return vector<EventPtr>();
}

void AppleTrackpadEventSource::onData(int device, Finger* pFingers, int numFingers, 
        double timestamp, int frame)
{
    for (int i = 0; i < numFingers; i++) {
        Finger* pFinger = &pFingers[i];
        vector<int>::iterator it;
        it = find(m_TouchIDs.begin(), m_TouchIDs.end(), pFinger->identifier);
        if (it == m_TouchIDs.end()) {
            m_TouchIDs.push_back(pFinger->identifier);
        } else {
            if (pFinger->state == 7) {
                m_TouchIDs.erase(it);
            }
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
    vector<int>::iterator it;
    for (it = m_TouchIDs.begin(); it != m_TouchIDs.end(); ++it) {
        cerr << *it << " ";
    }
    printf("\n");
}

int AppleTrackpadEventSource::callback(int device, Finger *data, int nFingers, 
        double timestamp, int frame) 
{
    AVG_ASSERT(s_pInstance != 0);
    s_pInstance->onData(device, data, nFingers, timestamp, frame);
    return 0;
}

}
