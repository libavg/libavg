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
//  Original author of this file is igor@c-base.org
//

#include "TrackerInputDeviceBase.h"
#include "TrackerTouchStatus.h"
#include "TouchEvent.h"

namespace avg {

void TrackerInputDeviceBase::pollEventType(std::vector<EventPtr>& result,
        TouchStatusMap& Events,  CursorEvent::Source source)
{
    EventPtr pEvent;
    for (TouchStatusMap::iterator it = Events.begin(); it!= Events.end();) {
        TrackerTouchStatusPtr pTouchStatus = (*it).second;
        pEvent = pTouchStatus->pollEvent();
        if (pEvent) {
            result.push_back(pEvent);
            if (pEvent->getType() == Event::CURSORUP) {
                Events.erase(it++);
            } else {
                ++it;
            }
        } else {
            ++it;
        }
    }
}

} //End namespace avg
