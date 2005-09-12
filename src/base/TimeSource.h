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
   
    long long getCurrentTicks();     // For millisecond accuracy.
    long long getCurrentMicrosecs();
    
    void sleepUntil(long long ticks);

private:    
    TimeSource();
    void tryOpenRTC();
    bool m_bUseRTC;
    int m_RTCFD;
    
    static TimeSource* m_pTimeSource;

};

}

#endif
