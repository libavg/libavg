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

#ifndef _MultitouchEventSource_H_
#define _MultitouchEventSource_H_

#include "../api.h"
#include "CursorEvent.h"
#include "IEventSource.h"

#include <boost/thread.hpp>
#include <set>

typedef boost::shared_ptr<boost::mutex> MutexPtr;

namespace avg {

class Touch;
typedef boost::shared_ptr<class Touch> TouchPtr;
class TouchEvent;
typedef boost::shared_ptr<class TouchEvent> TouchEventPtr;

class AVG_API MultitouchEventSource: public IEventSource
{
public:
    MultitouchEventSource();
    virtual ~MultitouchEventSource() = 0;
    virtual void start();
    
    std::vector<EventPtr> pollEvents();

protected:
    const DPoint& getWindowSize() const;
    TouchPtr getTouch(int id);
    void addTouch(int id, TouchEventPtr pInitialEvent);
    boost::mutex& getMutex();

private:
    std::map<int, TouchPtr> m_Touches;
    DPoint m_WindowSize;
    MutexPtr m_pMutex;
};

typedef boost::shared_ptr<MultitouchEventSource> MultitouchEventSourcePtr;

}

#endif

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

#ifndef _MultitouchEventSource_H_
#define _MultitouchEventSource_H_

#include "../api.h"
#include "CursorEvent.h"
#include "IEventSource.h"

extern "C" {
    
    typedef struct { float x,y; } mtPoint;
    typedef struct { mtPoint pos,vel; } mtReadout;
    
    typedef struct {
        int frame;
        double timestamp;
        int identifier, state, foo3, foo4;
        mtReadout normalized;
        float size;
        int zero1;
        float angle, majorAxis, minorAxis; // ellipsoid
        mtReadout mm;
        int zero2[2];
        float unk2;
    } Finger;
    
    typedef int MTDeviceRef;
    typedef int (*MTContactCallbackFunction)(int,Finger*,int,double,int);
    
    MTDeviceRef MTDeviceCreateDefault();
    void MTRegisterContactFrameCallback(MTDeviceRef, MTContactCallbackFunction);
    void MTUnregisterContactFrameCallback(MTDeviceRef, MTContactCallbackFunction);
    void MTDeviceStart(MTDeviceRef);
    void MTDeviceStop(MTDeviceRef);
    void MTDeviceRelease(MTDeviceRef);
    
}

namespace avg {

class AVG_API MultitouchEventSource: public IEventSource
{
    public:
        MultitouchEventSource();
        virtual ~MultitouchEventSource();
        void start();
        
        std::vector<EventPtr> pollEvents();

    private:
        MTDeviceRef m_Device;
};

typedef boost::shared_ptr<MultitouchEventSource> MultitouchEventSourcePtr;

}

#endif

