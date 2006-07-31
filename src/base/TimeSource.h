//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
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

#ifndef _TimeSource_H_ 
#define _TimeSource_H_

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN  /* somewhat limit Win32 pollution */
#include <Winsock2.h>
#undef ERROR
#undef WARNING
#else
#include <sys/time.h>
#endif

#ifdef __APPLE__
#include <mach/mach_time.h>
#endif

namespace avg {

typedef unsigned long long CycleCount;

class TimeSource {
public:
    static TimeSource* get();
    virtual ~TimeSource();
   
    long long getCurrentMillisecs();     // For millisecond accuracy.
    long long getCurrentMicrosecs();
    
    void sleepUntil(long long ticks);

private:    
    TimeSource();
    
    static TimeSource* m_pTimeSource;
};

}

#endif
