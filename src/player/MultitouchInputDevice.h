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

#ifndef _MultitouchInputDevice_H_
#define _MultitouchInputDevice_H_

#include "../api.h"
#include "IInputDevice.h"

#include "../base/GLMHelper.h"
#include "../base/ConfigMgr.h"

#include <boost/thread.hpp>
#include <map>
#include <set>

typedef boost::shared_ptr<boost::mutex> MutexPtr;

namespace avg {

class TouchStatus;
typedef boost::shared_ptr<class TouchStatus> TouchStatusPtr;
class TouchEvent;
typedef boost::shared_ptr<class TouchEvent> TouchEventPtr;
class Event;
typedef boost::shared_ptr<Event> EventPtr;

class AVG_API MultitouchInputDevice: public IInputDevice
{
public:
    MultitouchInputDevice();
    virtual ~MultitouchInputDevice() = 0;
    virtual void start();

    std::vector<EventPtr> pollEvents();

protected:
    int getNumTouches() const;
    // Note that the id used here is not the libavg cursor id but a touch-driver-specific
    // id handed up from the driver level.
    TouchStatusPtr getTouchStatus(int id);
    void addTouchStatus(int id, TouchEventPtr pInitialEvent);
    void removeTouchStatus(int id);
    void getDeadIDs(const std::set<int>& liveIDs, std::set<int>& deadIDs);
    glm::vec2 getTouchArea() const;
    IntPoint getScreenPos(const glm::vec2& pos) const;
    boost::mutex& getMutex();
    glm::vec2 mTouchOffset;

private:
    std::map<int, TouchStatusPtr> m_TouchIDMap;
    std::vector<TouchStatusPtr> m_Touches;
    MutexPtr m_pMutex;
    glm::vec2 mTouchArea;
};

typedef boost::shared_ptr<MultitouchInputDevice> MultitouchInputDevicePtr;

}

#endif
