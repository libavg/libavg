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

#include "TimeSource.h"

#include "../base/Logger.h"

#ifdef _WIN32
#include <time.h>
#include <sys/timeb.h>
#define WIN32_LEAN_AND_MEAN  /* somewhat limit Win32 pollution */
#include <windows.h>
#include <Mmsystem.h>
#else
#include <unistd.h>
#include <fcntl.h>
#ifdef __APPLE__ 
#include <mach/mach_time.h>
#endif
#endif
#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>
//#include <sys/ioctl.h>
#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

TimeSource* TimeSource::m_pTimeSource = 0;

TimeSource * TimeSource::get()
{
    if (!m_pTimeSource) {
        m_pTimeSource = new TimeSource;
    }
    return m_pTimeSource;
}

TimeSource::TimeSource()
{
}

TimeSource::~TimeSource()
{
}

long long TimeSource::getCurrentMillisecs()
{
    long long ticks;
#ifdef _WIN32
    ticks = timeGetTime();
#else
    struct timeval now;
    gettimeofday(&now, NULL);
    ticks=((long long)now.tv_sec)*1000+now.tv_usec/1000;
#endif
    return ticks;
}

long long TimeSource::getCurrentMicrosecs()
{
    long long ticks;
#ifdef _WIN32
    ticks = timeGetTime()*1000;
#else
    struct timeval now;
    gettimeofday(&now, NULL);
    ticks=((long long)now.tv_sec)*1000000+now.tv_usec;
#endif    
    return(ticks);
}

void TimeSource::sleepUntil(long long TargetTime)
{
    long long now = getCurrentMillisecs();
#ifdef __APPLE__
    if (TargetTime > now) { 
        msleep(TargetTime-now);
    }
#else
    while (now<TargetTime) {
        if (TargetTime-now<=2) {
            msleep(0);
         } else {
            msleep(int(TargetTime-now-2));
        }
        now = getCurrentMillisecs();
    }
#endif
}
    
void TimeSource::msleep(int millisecs)
{
#if _WIN32
            Sleep(millisecs);
#else
            usleep((long long)(millisecs)*1000);
#endif
}

}

