//
// $Id$
//

#include "AVGTimeSource.h"
#include "AVGLogger.h"
#include "IAVGPlayer.h"

#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/rtc.h>
#include <sys/ioctl.h>
#include <iostream>
#include <sstream>

using namespace std;

AVGTimeSource* AVGTimeSource::m_pTimeSource = 0;

AVGTimeSource * AVGTimeSource::get()
{
    if (!m_pTimeSource) {
        m_pTimeSource = new AVGTimeSource;
    }
    return m_pTimeSource;
}

AVGTimeSource::AVGTimeSource()
{
    tryOpenRTC();
}

void AVGTimeSource::tryOpenRTC()
{
    m_bUseRTC = true;
    m_RTCFD = open("/dev/rtc", O_RDONLY);
    if (m_RTCFD == -1) {
        AVG_TRACE(IAVGPlayer::DEBUG_PROFILE,
            "Couldn't open /dev/rtc: " << strerror(errno) << " (Running as root?)"); 
        m_bUseRTC = false;
    } else {
        const unsigned IntrFrequency = 1024;
        int err = ioctl(m_RTCFD, RTC_IRQP_SET, IntrFrequency);
        if (err == -1) {
            AVG_TRACE(IAVGPlayer::DEBUG_PROFILE, "Couldn't set rtc interrupt (Running as root?)");
            close(m_RTCFD);
            m_bUseRTC = false;
            return;
        }
        err = ioctl(m_RTCFD, RTC_PIE_ON, 0);
        if (err == -1) {
            AVG_TRACE(IAVGPlayer::DEBUG_ERROR, "ioctl(...RTC_PIE_ON...) failed. Huh?");
            close(m_RTCFD);
            m_bUseRTC = false;
        }
    }
}

AVGTimeSource::~AVGTimeSource()
{
    if (m_bUseRTC) {
        close(m_RTCFD);
    }
}

int AVGTimeSource::getCurrentTicks()
{
    struct timeval now;
    int ticks;

    gettimeofday(&now, NULL);
    ticks=now.tv_sec*1000+now.tv_usec/1000;
    return(ticks);
}

void AVGTimeSource::sleepUntil(int ticks)
{
    if (m_bUseRTC) {
        while(getCurrentTicks()<ticks) {
            unsigned long dummy;
            int err = read(m_RTCFD, &dummy, sizeof(dummy));
            if (err == -1) {
                AVG_TRACE(IAVGPlayer::DEBUG_ERROR, "failed to read RTC (" << strerror(errno) <<
                        "). Switching to usleep.");
                m_bUseRTC = false;
                return;
            }
        }
    } else {
        unsigned int jitter = 20;
        int now = getCurrentTicks();
        while (now+jitter<ticks) {
            if (ticks-now<=20) {
                usleep(0);
            } else {
                usleep(ticks-now-20);
            }
            now = getCurrentTicks();
        }
    }
}

