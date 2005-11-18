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
