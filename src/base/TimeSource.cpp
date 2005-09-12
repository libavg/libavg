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
#else
#include <linux/rtc.h>
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
#ifndef __APPLE__   
    tryOpenRTC();
#endif
}

TimeSource::~TimeSource()
{
#ifndef __APPLE__   
    if (m_bUseRTC) {
        close(m_RTCFD);
    }
#endif
}

long long TimeSource::getCurrentTicks()
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
#ifdef __APPLE__
    long long now = getCurrentTicks();
    usleep((TargetTime-now)*1000);
#else
    if (m_bUseRTC) {
        while(getCurrentTicks()<TargetTime) {
            unsigned long dummy;
            int err = read(m_RTCFD, &dummy, sizeof(dummy));
            if (err == -1) {
                AVG_TRACE(Logger::ERROR, "failed to read RTC (" << strerror(errno) <<
                        "). Switching to usleep.");
                m_bUseRTC = false;
                return;
            }
        }
    } else {
        unsigned int jitter = 2;
        long long now = getCurrentTicks();
        while (now+jitter<TargetTime) {
            if (TargetTime-now<=2) {
                usleep(0);
            } else {
                usleep((TargetTime-now-2)*1000);
            }
            now = getCurrentTicks();
        }
    }
#endif
}

void TimeSource::tryOpenRTC()
{
#ifndef __APPLE__   
    m_bUseRTC = true;
    m_RTCFD = open("/dev/rtc", O_RDONLY);
    if (m_RTCFD == -1) {
        AVG_TRACE(Logger::PROFILE,
            "Couldn't open /dev/rtc: " << strerror(errno) << " (Running as root?)"); 
        m_bUseRTC = false;
    } else {
        const unsigned IntrFrequency = 1024;
        int err = ioctl(m_RTCFD, RTC_IRQP_SET, IntrFrequency);
        if (err == -1) {
            AVG_TRACE(Logger::PROFILE, "Couldn't set rtc interrupt (Running as root?)");
            close(m_RTCFD);
            m_bUseRTC = false;
            return;
        }
        err = ioctl(m_RTCFD, RTC_PIE_ON, 0);
        if (err == -1) {
            AVG_TRACE(Logger::ERROR, "ioctl(...RTC_PIE_ON...) failed. Huh?");
            close(m_RTCFD);
            m_bUseRTC = false;
        }
    }
#endif
}

}

