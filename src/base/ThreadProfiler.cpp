//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2011 Ulrich von Zadow
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
#include "ProfilingZone.h"
#include "ScopeTimer.h"

#include <sstream>
#include <iomanip>
#include <iostream>

using namespace std;
using namespace boost;

namespace avg {
    
thread_specific_ptr<ThreadProfiler*> ThreadProfiler::s_pInstance;

ThreadProfiler* ThreadProfiler::get() 
{
    if (s_pInstance.get() == 0) {
        s_pInstance.reset(new (ThreadProfiler*));
        *s_pInstance = new ThreadProfiler();
    }
    return *s_pInstance;
}

void ThreadProfiler::kill()
{
    delete *s_pInstance;
    s_pInstance.reset();
}

ThreadProfiler::ThreadProfiler()
    : m_sName(""),
      m_LogCategory(Logger::category::PROFILE)
{
    m_bRunning = false;
    ScopeTimer::enableTimers(Logger::get()->shouldLog(m_LogCategory,
            Logger::severity::INFO));
}

ThreadProfiler::~ThreadProfiler() 
{
}

void ThreadProfiler::setLogCategory(category_t category)
{
    AVG_ASSERT(!m_bRunning);
    m_LogCategory = category;
}

void ThreadProfiler::start()
{
    m_bRunning = true;
}

void ThreadProfiler::restart()
{
    ZoneVector::iterator it;
    for (it = m_Zones.begin(); it != m_Zones.end(); ++it) {
        (*it)->restart();
    }
}

void ThreadProfiler::startZone(const ProfilingZoneID& zoneID)
{
    ZoneMap::iterator it = m_ZoneMap.find(&zoneID);
    // Duplicated code to avoid instantiating a new smart pointer when it's not
    // necessary.
    if (it == m_ZoneMap.end()) {
        ProfilingZonePtr pZone = addZone(zoneID);
        pZone->start();
        m_ActiveZones.push_back(pZone);
    } else {
        ProfilingZonePtr& pZone = it->second;
        pZone->start();
        m_ActiveZones.push_back(pZone);
    }
}

void ThreadProfiler::stopZone(const ProfilingZoneID& zoneID)
{
    ZoneMap::iterator it = m_ZoneMap.find(&zoneID);
    ProfilingZonePtr& pZone = it->second;
    pZone->stop();
    m_ActiveZones.pop_back();
}

void ThreadProfiler::dumpStatistics()
{
    if (!m_Zones.empty()) {
        AVG_TRACE(m_LogCategory, Logger::severity::INFO, "Thread " << m_sName);
        AVG_TRACE(m_LogCategory, Logger::severity::INFO,
                "Zone name                          Avg. time");
        AVG_TRACE(m_LogCategory, Logger::severity::INFO,
                "---------                          ---------");

        ZoneVector::iterator it;
        for (it = m_Zones.begin(); it != m_Zones.end(); ++it) {
            AVG_TRACE(m_LogCategory, Logger::severity::INFO,
                    std::setw(35) << std::left 
                    << ((*it)->getIndentString()+(*it)->getName())
                    << std::setw(9) << std::right << (*it)->getAvgUSecs());
        }
        AVG_TRACE(m_LogCategory, Logger::severity::INFO, "");
    }
}

void ThreadProfiler::reset()
{
    ZoneVector::iterator it;
    for (it = m_Zones.begin(); it != m_Zones.end(); ++it) {
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


ProfilingZonePtr ThreadProfiler::addZone(const ProfilingZoneID& zoneID)
{
    ProfilingZonePtr pZone(new ProfilingZone(zoneID));
    m_ZoneMap[&zoneID] = pZone;
    ZoneVector::iterator it;
    int parentIndent = -2;
    if (m_ActiveZones.empty()) {
        it = m_Zones.end();
    } else {
        ProfilingZonePtr pActiveZone = m_ActiveZones.back();
        bool bParentFound = false;
        for (it = m_Zones.begin(); it != m_Zones.end(); ++it) 
        {
            if (pActiveZone == *it) {
                bParentFound = true;
                break;
            }
        }
        AVG_ASSERT(bParentFound);
        parentIndent = pActiveZone->getIndentLevel();
        ++it;
        for (; it != m_Zones.end() && (*it)->getIndentLevel() > parentIndent; ++it) {};
    }
    m_Zones.insert(it, pZone);
    pZone->setIndentLevel(parentIndent+2);
    return pZone;
}

}

