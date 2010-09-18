//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2008 Ulrich von Zadow
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
#include "Logger.h"
#include "Exception.h"

#include <sstream>
#include <iomanip>
#include <iostream>

using namespace std;
using namespace boost;

namespace avg {
    
thread_specific_ptr<ThreadProfilerPtr> ThreadProfiler::s_pInstance;

ThreadProfilerPtr& ThreadProfiler::get() 
{
    if (s_pInstance.get() == 0) {
        s_pInstance.reset(new ThreadProfilerPtr(new ThreadProfiler()));
    }
    return *s_pInstance;
}

void ThreadProfiler::kill()
{
    s_pInstance.reset();
}

ThreadProfiler::ThreadProfiler()
    : m_sName(""),
      m_LogCategory(Logger::PROFILE)
{
    m_bRunning = false;
}

ThreadProfiler::~ThreadProfiler() 
{
    for (ZoneList::iterator it=m_Zones.begin(); it != m_Zones.end(); ++it) {
        if (!((*it)->isStatic())) {
            delete *it;
        }
    }
}

void ThreadProfiler::setLogCategory(long category)
{
    AVG_ASSERT(!m_bRunning);
    m_LogCategory = category;
}

int ThreadProfiler::addZone(ProfilingZone& Zone)
{
    ZoneList::iterator it;
    int parentIndent = -2;
    if (m_ActiveZones.empty()) {
        it = m_Zones.end();
    } else {
        ProfilingZone* pActiveZone = m_ActiveZones.back();
        ZoneList::iterator itPrevZone = m_Zones.begin();
        bool bParentFound = false;
        for (it=m_Zones.begin(); it != m_Zones.end(); ++it) 
        {
            if (pActiveZone == *it) {
                bParentFound = true;
                break;
            }
        }
        AVG_ASSERT(bParentFound);
        parentIndent = pActiveZone->getIndentLevel();
        ++it;
        for (; it != m_Zones.end() && (*it)->getIndentLevel() > parentIndent; ++it);
    }
    m_Zones.insert(it, &Zone);
    return parentIndent+2;
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
    AVG_ASSERT(m_ActiveZones.back() == pZone);
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
        AVG_TRACE(m_LogCategory, "Thread " << m_sName);
        AVG_TRACE(m_LogCategory, "Zone name                          Avg. time");
        AVG_TRACE(m_LogCategory, "---------                          ---------");

        ZoneList::iterator it;
        for (it=m_Zones.begin(); it != m_Zones.end(); ++it) {
            AVG_TRACE(m_LogCategory,
                    std::setw(35) << std::left 
                    << ((*it)->getIndentString()+(*it)->getName())
                    << std::setw(9) << std::right << (*it)->getAvgUSecs());
        }
        AVG_TRACE(m_LogCategory, "");
    }
}

void ThreadProfiler::reset()
{
    ZoneList::iterator it;
    for (it=m_Zones.begin(); it != m_Zones.end(); ++it) {
        (*it)->reset();
    }
}

int ThreadProfiler::getNumZones()
{
    return m_Zones.size();
}

const std::string& ThreadProfiler::getName() const
{
    return m_sName;
}

void ThreadProfiler::setName(const std::string& sName)
{
    m_sName = sName;
}

}

