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

#include "ThreadProfiler.h"
#include "Profiler.h"
#include "Logger.h"
#include "ObjectCounter.h"

#include <sstream>
#include <iomanip>

using namespace std;
using namespace boost;

namespace avg {
    
ThreadProfiler::ThreadProfiler(const string& sName)
    : m_sName(sName)
{
    m_pActiveZone = 0;
    m_bRunning = false;
    ObjectCounter::get()->incRef(&typeid(*this));
}

ThreadProfiler::~ThreadProfiler() 
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

ThreadProfilerPtr ThreadProfiler::get()
{
    return Profiler::get().getThreadProfiler();
}

void ThreadProfiler::addZone(ProfilingZone& Zone)
{
    ZoneList::iterator itPrevZone = m_Zones.begin();
    for (ZoneList::iterator it=m_Zones.begin(); it != m_Zones.end(); ++it) {
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

void ThreadProfiler::clear()
{
    ZoneList::iterator it;
    for (it=m_Zones.begin(); it != m_Zones.end(); ++it) {
        (*it)->clear();
    }
    m_Zones.clear();
    m_pActiveZone = 0;
    m_bRunning = false;
}

void ThreadProfiler::start()
{
    clear();
    m_bRunning = true;
}

bool ThreadProfiler::isRunning()
{
    return m_bRunning;
}

void ThreadProfiler::setActiveZone(ProfilingZone * pZone)
{
    m_pActiveZone = pZone;
}

void ThreadProfiler::dumpFrame()
{
    AVG_TRACE(Logger::PROFILE_LATEFRAMES, "Frame Profile:");
    ZoneList::iterator it;
    for (it=m_Zones.begin(); it != m_Zones.end(); ++it) {
        AVG_TRACE(Logger::PROFILE_LATEFRAMES,
                "    " << std::setw(30) << std::left << (*it)->getName() << ": " 
                << std::setw(9) << std::right << (*it)->getUSecs());
    }
    AVG_TRACE(Logger::PROFILE_LATEFRAMES, "");
}

void ThreadProfiler::dumpStatistics()
{
    if (!m_Zones.empty()) {
        AVG_TRACE(Logger::PROFILE, "Thread " << m_sName);
        AVG_TRACE(Logger::PROFILE, "Zone name                          Avg. time");
        AVG_TRACE(Logger::PROFILE, "---------                          ---------");

        ZoneList::iterator it;
        for (it=m_Zones.begin(); it != m_Zones.end(); ++it) {
            AVG_TRACE(Logger::PROFILE,
                    "  " << std::setw(33) << std::left << (*it)->getName() << ": " 
                    << std::setw(7) << std::right << (*it)->getAvgUSecs());
        }
        AVG_TRACE(Logger::PROFILE, "");
    }
}

void ThreadProfiler::reset()
{
    ZoneList::iterator it;
    for (it=m_Zones.begin(); it != m_Zones.end(); ++it) {
        (*it)->reset();
    }
}

bool ThreadProfiler::isCurrent()
{
    return thread() == m_Thread;
}

const std::string& ThreadProfiler::getName()
{
    return m_sName;
}

}

