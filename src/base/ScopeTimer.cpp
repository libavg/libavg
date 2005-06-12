//
// $Id$
//

#include "ScopeTimer.h"

using namespace std;

namespace avg {

ScopeTimer::ScopeTimer(ProfilingZone& Zone)
    : m_Zone(Zone)
{
    m_StartTime = TimeSource::get()->getCurrentCycles();
    m_Zone.start();
}

ScopeTimer::~ScopeTimer() 
{
    CycleCount ActiveTime = TimeSource::get()->getCurrentCycles()-m_StartTime;
    m_Zone.addCycles(ActiveTime);
}

}
