//
// $Id$
//

#include "AVGProfiler.h"
#include "AVGPlayer.h"
#include "AVGLogger.h"

#include <sstream>

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
    ZoneList::iterator it;
    AVG_TRACE(AVGPlayer::DEBUG_PROFILE,
            "Profile:");
    for (it=m_Zones.begin(); it != m_Zones.end(); ++it) {
        AVG_TRACE(AVGPlayer::DEBUG_PROFILE,
                "  " << (*it)->getName() << ": " 
                << (*it)->getCycles()/1000);
    }
}

void AVGProfiler::reset()
{
    ZoneList::iterator it;
    for (it=m_Zones.begin(); it != m_Zones.end(); ++it) {
        (*it)->reset();
    }
}

