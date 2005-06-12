//
// $Id$
//

#ifndef _TimeSource_H_ 
#define _TimeSource_H_

#include <sys/time.h>

#ifdef __APPLE__
#include <mach/mach_time.h>
#endif

namespace avg {

typedef unsigned long long CycleCount;

class TimeSource {
public:
    static TimeSource* get();
    ~TimeSource();
   
    long long getCurrentTicks();
    __inline__ CycleCount getCurrentCycles();
    CycleCount getCyclesPerSecond();
    
    void sleepUntil(long long ticks);

private:    
    TimeSource();
    void tryOpenRTC();
    void calcCyclesPerSecond();
    bool m_bUseRTC;
    int m_RTCFD;
    CycleCount m_CyclesPerSecond;
    
    static TimeSource* m_pTimeSource;

};

__inline__ CycleCount TimeSource::getCurrentCycles()
{
    CycleCount Cycles;
#ifdef __APPLE__
    return mach_absolute_time();
#else
    __asm__ volatile (".byte 0x0f, 0x31" : "=A" ( Cycles));
    return Cycles;
#endif 
}

}

#endif
