//
// $Id$
//

#include "AVGScopeTimer.h"

using namespace std;

AVGScopeTimer::AVGScopeTimer(AVGProfilingZone& Zone)
    : m_Zone(Zone)
{
    m_StartTime = AVGTimeSource::rdtsc();
}

AVGScopeTimer::~AVGScopeTimer() 
{
    CycleCount ActiveTime = AVGTimeSource::rdtsc()-m_StartTime;
    m_Zone.addCycles(ActiveTime);
}

