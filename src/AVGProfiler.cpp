//
// $Id$
//

#include "AVGProfiler.h"
#include "AVGPlayer.h"
#include "AVGLogger.h"

#include <sstream>
#include <iomanip>

using namespace std;

AVGProfiler& AVGProfiler::get() 
{
    static AVGProfiler s_Instance;
    return s_Instance;
}

AVGProfiler::AVGProfiler() 
{
}

AVGProfiler::~AVGProfiler() 
{
}

void AVGProfiler::addZone(AVGProfilingZone& Zone)
{
    ZoneList::iterator it;
    for (it=m_Zones.begin(); it != m_Zones.end(); ++it) {
        if (Zone.getName() == (*it)->getName()) {
            AVG_TRACE(AVGPlayer::DEBUG_WARNING,
                    "Warning: Two profiling zones have name " <<
                            Zone.getName());
        }
    }
    m_Zones.push_back(&Zone);
}

void AVGProfiler::dump()
{
    AVG_TRACE(AVGPlayer::DEBUG_PROFILE2,
            "Profile:");
    ZoneList::iterator it;
    for (it=m_Zones.begin(); it != m_Zones.end(); ++it) {
        AVG_TRACE(AVGPlayer::DEBUG_PROFILE2,
                "  " << (*it)->getName() << ": " 
                << (*it)->getCycles()/1000);
    }
}

void AVGProfiler::dumpStatistics()
{
    AVG_TRACE(AVGPlayer::DEBUG_PROFILE,
            "Profile Statistics:");
    AVG_TRACE(AVGPlayer::DEBUG_PROFILE,
            "  Zone name                       Avg. time");

    ZoneList::iterator it;
    for (it=m_Zones.begin(); it != m_Zones.end(); ++it) {
        AVG_TRACE(AVGPlayer::DEBUG_PROFILE,
                "  " << std::setw(30) << std::left << (*it)->getName() << ": " 
                << std::setw(6) << std::right << (*it)->getAvgCycles()/1000);
    }
}

void AVGProfiler::reset()
{
    ZoneList::iterator it;
    for (it=m_Zones.begin(); it != m_Zones.end(); ++it) {
        (*it)->reset();
    }
}

