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
    m_bRunning = false;
    ObjectCounter::get()->incRef(&typeid(*this));
}

ThreadProfiler::~ThreadProfiler() 
{
    for (ZoneList::iterator it=m_Zones.begin(); it != m_Zones.end(); ++it) {
        if (!((*it)->isStatic())) {
            delete *it;
        }
    }
    ObjectCounter::get()->decRef(&typeid(*this));
}

ThreadProfilerPtr ThreadProfiler::get()
{
    return Profiler::get().getThreadProfiler();
}

void ThreadProfiler::addZone(ProfilingZone& Zone)
{
    ZoneList::iterator it;
    if (m_ActiveZones.empty()) {
        it = m_Zones.end();
    } else {
        ProfilingZone* pActiveZone = m_ActiveZones.back();
        ZoneList::iterator itPrevZone = m_Zones.begin();
        bool bParentFound = false;
        for (it=m_Zones.begin(); it != m_Zones.end() && !bParentFound; ++it) 
        {
            if (Zone.getName() == (*it)->getName()) {
                AVG_TRACE(Logger::WARNING,
                        "Warning: Two profiling zones have name " <<
                        Zone.getName());
            }
            if (pActiveZone == *it) {
                bParentFound = true;
            }
        }
        assert(bParentFound);
        int ParentIndent = pActiveZone->getIndentLevel();
        for (; it != m_Zones.end() && (*it)->getIndentLevel() > ParentIndent; ++it);
    }
    m_Zones.insert(it, &Zone);
}

void ThreadProfiler::clear()
{
    ZoneList::iterator it;
    for (it=m_Zones.begin(); it != m_Zones.end(); ++it) {
        (*it)->clear();
    }
    m_Zones.clear();
    m_ActiveZones.clear();
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

void ThreadProfiler::pushActiveZone(ProfilingZone * pZone)
{
    m_ActiveZones.push_back(pZone);
}

void ThreadProfiler::popActiveZone(ProfilingZone * pZone)
{
    assert(m_ActiveZones.back() == pZone);
    m_ActiveZones.pop_back();
}

void ThreadProfiler::dumpFrame()
{
    AVG_TRACE(Logger::PROFILE_LATEFRAMES, "Frame Profile:");
    ZoneList::iterator it;
    for (it=m_Zones.begin(); it != m_Zones.end(); ++it) {
        AVG_TRACE(Logger::PROFILE_LATEFRAMES,
                std::setw(35) << std::left 
                << ((*it)->getIndentString() + (*it)->getName()) 
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
                    std::setw(35) << std::left 
                    << ((*it)->getIndentString()+(*it)->getName())
                    << std::setw(9) << std::right << (*it)->getAvgUSecs());
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

int ThreadProfiler::getIndent()
{
    return int(2*m_ActiveZones.size());
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

