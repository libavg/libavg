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

#ifndef _AppleTrackpadEventSource_H_
#define _AppleTrackpadEventSource_H_

#include "../api.h"
#include "CursorEvent.h"
#include "IEventSource.h"

#include <vector>

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

typedef void *MTDeviceRef;
typedef int (*MTContactCallbackFunction)(int,Finger*,int,double,int);

MTDeviceRef MTDeviceCreateDefault();
void MTRegisterContactFrameCallback(MTDeviceRef, MTContactCallbackFunction);
void MTDeviceStart(MTDeviceRef, int);
void MTDeviceStop(MTDeviceRef);
void MTDeviceRelease(MTDeviceRef);

/*
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
  */  
}

namespace avg {

class AVG_API AppleTrackpadEventSource: public IEventSource
{
public:
    AppleTrackpadEventSource();
    virtual ~AppleTrackpadEventSource();
    void start();
    
    std::vector<EventPtr> pollEvents();

private:
    void onData(int device, Finger *data, int nFingers, double timestamp, int frame);
    static int callback(int device, Finger *data, int nFingers, double timestamp, 
            int frame);
    MTDeviceRef m_Device;
    static AppleTrackpadEventSource* s_pInstance;
    std::vector<int> m_TouchIDs;
};

typedef boost::shared_ptr<AppleTrackpadEventSource> AppleTrackpadEventSourcePtr;

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

#ifndef _AppleTrackpadEventSource_H_
#define _AppleTrackpadEventSource_H_

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

class AVG_API AppleTrackpadEventSource: public IEventSource
{
    public:
        AppleTrackpadEventSource();
        virtual ~AppleTrackpadEventSource();
        void start();
        
        std::vector<EventPtr> pollEvents();

    private:
        MTDeviceRef m_Device;
};

typedef boost::shared_ptr<AppleTrackpadEventSource> AppleTrackpadEventSourcePtr;

}

#endif

