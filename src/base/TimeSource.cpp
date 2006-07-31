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
#ifdef _WIN32
static int gettimeofday(struct timeval *time_Info, struct timezone *timezone_Info)
{
  // remarks: a DWORD is an unsigned long
  static DWORD time_t0, time_delta, mm_t0;
  static int t_initialized = 0;
  DWORD mm_t, delta_t;

  if( !t_initialized )
  {
    time_t0 = time(NULL);
    time_delta = 0;
    mm_t0 = timeGetTime();
    t_initialized = 1;
  }
  /* Get the time, if they want it */
  if (time_Info != NULL) 
  {
    // timeGetTime() returns the system time in milliseconds
    mm_t = timeGetTime();
  
    // handle wrap around of system time (happens every 
    // 2^32 milliseconds = 49.71 days)
    if( mm_t < mm_t0 )
      delta_t = (0xffffffff - mm_t0) + mm_t + 1; 
    else
      delta_t = mm_t - mm_t0;
    mm_t0 = mm_t;

    time_delta += delta_t;
    if( time_delta >= 1000 )
    {
      time_t0 += time_delta / 1000;
      time_delta = time_delta % 1000;
    }
    time_Info->tv_sec = time_t0;
    time_Info->tv_usec = time_delta * 1000;
  }
  /* Get the timezone, if they want it */
/*
  if (timezone_Info != NULL) {
    _tzset();
    timezone_Info->tz_minuteswest = _timezone;
    timezone_Info->tz_dsttime = _daylight;
  }
  */
  /* And return */
  return 0;
}
#endif

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
    struct timeval now;
    long long ticks;

    gettimeofday(&now, NULL);
    ticks=((long long)now.tv_sec)*1000+now.tv_usec/1000;
    return(ticks);
}

long long TimeSource::getCurrentMicrosecs()
{
    struct timeval now;
    long long ticks;

    gettimeofday(&now, NULL);
    ticks=((long long)now.tv_sec)*1000000+now.tv_usec;
    return(ticks);
}

void TimeSource::sleepUntil(long long TargetTime)
{
    long long now = getCurrentMillisecs();
#ifdef __APPLE__
    if (TargetTime > now) { 
        usleep((TargetTime-now)*1000);
    }
#else
    while (now<TargetTime) {
        if (TargetTime-now<=2) {
#if _WIN32
            Sleep(0);
#else
            usleep(0);
#endif
         } else {
#if _WIN32
            Sleep(TargetTime-now-2);
#else
            usleep((TargetTime-now-2)*1000);
#endif
        }
        now = getCurrentMillisecs();
    }
#endif
}

}

