//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  Current versions can be found at www.libavg.de
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
    ZoneList& List = m_Zones[Zone.getThreadID()];
    ZoneList::iterator it;
    ZoneList::iterator itPrevZone = List.begin();
    for (it=List.begin(); it != List.end(); ++it) {
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
    List.insert(itPrevZone, &Zone);
}

void Profiler::clear()
{
    ZoneMap::iterator it1;
    for (it1=m_Zones.begin(); it1 != m_Zones.end(); ++it1) {
        ZoneList& List = (*it1).second;
        ZoneList::iterator it2;
        for (it2=List.begin(); it2 != List.end(); ++it2) {
            (*it2)->clear();
        }
        List.clear();
    }
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
    AVG_TRACE(Logger::PROFILE_LATEFRAMES, "Frame Profile:");
    ZoneMap::iterator it1;
    for (it1=m_Zones.begin(); it1 != m_Zones.end(); ++it1) {
        AVG_TRACE(Logger::PROFILE_LATEFRAMES,
                "  " << "Thread " << (*it1).first << ":");
        ZoneList& List = (*it1).second;
        ZoneList::iterator it2;
        for (it2=List.begin(); it2 != List.end(); ++it2) {
            AVG_TRACE(Logger::PROFILE_LATEFRAMES,
                    "    " << std::setw(30) << std::left << (*it2)->getName() << ": " 
                    << std::setw(9) << std::right << (*it2)->getUSecs());
        }
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

    ZoneMap::iterator it1;
    for (it1=m_Zones.begin(); it1 != m_Zones.end(); ++it1) {
        AVG_TRACE(Logger::PROFILE,
                "  " << "Thread " << (*it1).first << ":");
        ZoneList& List = (*it1).second;
        ZoneList::iterator it2;
        for (it2=List.begin(); it2 != List.end(); ++it2) {
            AVG_TRACE(Logger::PROFILE,
                    "    " << std::setw(33) << std::left << (*it2)->getName() << ": " 
                    << std::setw(9) << std::right << (*it2)->getAvgUSecs());
        }
    }
    AVG_TRACE(Logger::PROFILE, "");
}

void Profiler::reset(const string& ThreadID)
{
    ZoneMap::iterator it1;
    it1 = m_Zones.find(ThreadID);
    if (it1 != m_Zones.end()) {
        ZoneList& List = (*it1).second;
        ZoneList::iterator it2;
        for (it2=List.begin(); it2 != List.end(); ++it2) {
            (*it2)->reset();
        }
    }
}

}

