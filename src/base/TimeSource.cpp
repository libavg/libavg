//
// $Id$
//

#include "TimeSource.h"

#include "../base/Logger.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef __APPLE__ 
#include <mach/mach_time.h>
#endif
#include <errno.h>
#include <sys/ioctl.h>
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
    usleep((TargetTime-now)*1000);
#else
    while (now<TargetTime) {
        if (TargetTime-now<=2) {
            usleep(0);
        } else {
            usleep((TargetTime-now-2)*1000);
        }
        now = getCurrentMillisecs();
    }
#endif
}

}

