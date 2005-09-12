//
// $Id$
//

#ifndef _ScopeTimer_H_ 
#define _ScopeTimer_H_

#include "ProfilingZone.h"
#include "TimeSource.h"

namespace avg {
    
class ScopeTimer {
public:
    ScopeTimer(ProfilingZone& Zone);
    ~ScopeTimer();
   
private:
    ProfilingZone& m_Zone;
    long long m_StartTime;
};

}

#endif
