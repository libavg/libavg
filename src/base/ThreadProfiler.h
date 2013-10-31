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

#ifndef _ThreadProfiler_H_ 
#define _ThreadProfiler_H_

#include "../api.h"
#include "ILogSink.h"

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/tss.hpp>

#include <vector>
#include <map>
#if defined(_WIN32) || defined(_LIBCPP_VERSION)
#include <unordered_map>
#else
#include <tr1/unordered_map>
#endif
namespace avg {

class ThreadProfiler;
typedef boost::shared_ptr<ThreadProfiler> ThreadProfilerPtr;
class ProfilingZone;
typedef boost::shared_ptr<ProfilingZone> ProfilingZonePtr;
class ProfilingZoneID;

class AVG_API ThreadProfiler
{
public:
    static ThreadProfiler* get();
    static void kill();
    ThreadProfiler();
    virtual ~ThreadProfiler();
    void setLogCategory(category_t category);
 
    void start();
    void restart();
    void startZone(const ProfilingZoneID& zoneID);
    void stopZone(const ProfilingZoneID& zoneID);
    void dumpStatistics();
    void reset();
    int getNumZones();

    const std::string& getName() const;
    void setName(const std::string& sName);

private:
    ProfilingZonePtr addZone(const ProfilingZoneID& zoneID);
    std::string m_sName;

#if defined(_WIN32) || defined(_LIBCPP_VERSION)
    typedef std::unordered_map<const ProfilingZoneID*, ProfilingZonePtr> ZoneMap;
#else
    typedef std::tr1::unordered_map<const ProfilingZoneID*, ProfilingZonePtr> ZoneMap;
#endif
    typedef std::vector<ProfilingZonePtr> ZoneVector;
    ZoneMap m_ZoneMap;
    ZoneVector m_ActiveZones;
    ZoneVector m_Zones;
    bool m_bRunning;
    category_t m_LogCategory;

    static boost::thread_specific_ptr<ThreadProfiler*> s_pInstance;
};

}

#endif
