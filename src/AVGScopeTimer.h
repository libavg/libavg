//
// $Id$
//

#ifndef _AVGScopeTimer_H_ 
#define _AVGScopeTimer_H_

#include "AVGProfilingZone.h"
#include "AVGTimeSource.h"

class AVGScopeTimer {
public:
    AVGScopeTimer(AVGProfilingZone& Zone);
    ~AVGScopeTimer();
   
private:
    AVGProfilingZone& m_Zone;
    CycleCount m_StartTime;
};

#endif
