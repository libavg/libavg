//
// $Id$
//

#include "Profiler.h"
#include "Logger.h"

#include <sstream>
#include <iomanip>

using namespace std;

namespace avg {
    
Profiler& Profiler::get() 
{
    static Profiler s_Instance;
    return s_Instance;
}

Profiler::Profiler() 
{
    m_pActiveZone = 0;
    m_bRunning = false;
}

Profiler::~Profiler() 
{
}

void Profiler::addZone(ProfilingZone& Zone)
{
    ZoneList::iterator it;
    ZoneList::iterator itPrevZone = m_Zones.begin();
    for (it=m_Zones.begin(); it != m_Zones.end(); ++it) {
        if (Zone.getName() == (*it)->getName()) {
            AVG_TRACE(Logger::WARNING,
                    "Warning: Two profiling zones have name " <<
                    Zone.getName());
        }
        if (m_pActiveZone == (*it)) {
            itPrevZone = it;
            itPrevZone++;
        }
    }
    m_Zones.insert(itPrevZone, &Zone);
}

void Profiler::clear()
{
    ZoneList::iterator it;
    for (it=m_Zones.begin(); it != m_Zones.end(); ++it) {
        (*it)->clear();
    }
    m_Zones.clear();
    m_pActiveZone = 0;
    m_bRunning = false;
}

void Profiler::start()
{
    clear();
    m_bRunning = true;
}

bool Profiler::isRunning()
{
    return m_bRunning;
}

void Profiler::setActiveZone(ProfilingZone * pZone)
{
    m_pActiveZone = pZone;
}

void Profiler::dumpFrame()
{
    AVG_TRACE(Logger::PROFILE_LATEFRAMES,
            "Frame Profile:");
    ZoneList::iterator it;
    for (it=m_Zones.begin(); it != m_Zones.end(); ++it) {
        AVG_TRACE(Logger::PROFILE_LATEFRAMES,
                "  " << std::setw(30) << std::left << (*it)->getName() << ": " 
                << std::setw(9) << std::right << (*it)->getUSecs());
    }
    AVG_TRACE(Logger::PROFILE_LATEFRAMES, "");
}

void Profiler::dumpStatistics()
{
    AVG_TRACE(Logger::PROFILE,
            "Profile Statistics (in us):");
    AVG_TRACE(Logger::PROFILE,
            "  Zone name                          Avg. time");
    AVG_TRACE(Logger::PROFILE,
            "  ---------                          ---------");

    ZoneList::iterator it;
    for (it=m_Zones.begin(); it != m_Zones.end(); ++it) {
        AVG_TRACE(Logger::PROFILE,
                "  " << std::setw(33) << std::left << (*it)->getName() << ": " 
                << std::setw(9) << std::right << (*it)->getAvgUSecs());
    }
    AVG_TRACE(Logger::PROFILE, "");
}

void Profiler::reset()
{
    ZoneList::iterator it;
    for (it=m_Zones.begin(); it != m_Zones.end(); ++it) {
        (*it)->reset();
    }
}

}

