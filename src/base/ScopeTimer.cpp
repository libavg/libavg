//
// $Id$
//

#include "ScopeTimer.h"

using namespace std;

namespace avg {

ScopeTimer::ScopeTimer(ProfilingZone& Zone)
    : m_Zone(Zone)
{
    m_StartTime = TimeSource::get()->getCurrentMicrosecs();
    m_Zone.start();
}

ScopeTimer::~ScopeTimer() 
{
    long long ActiveTime = TimeSource::get()->getCurrentMicrosecs()-m_StartTime;
    m_Zone.add(ActiveTime);
}

}
