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

#ifndef _ThreadProfiler_H_ 
#define _ThreadProfiler_H_

#include "../api.h"
#include "Profiler.h"
#include "ProfilingZone.h"

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>

#include <list>
#include <map>

namespace avg {

class ThreadProfiler;
typedef boost::shared_ptr<ThreadProfiler> ThreadProfilerPtr;

class AVG_API ThreadProfiler {
public:
    ThreadProfiler(const std::string& sName);
    virtual ~ThreadProfiler();
    static ThreadProfilerPtr get()
    {
        return Profiler::get().getThreadProfiler();
    }
 
    void addZone(ProfilingZone& Zone);
    void clear();
    void start();
    bool isRunning();
    void pushActiveZone(ProfilingZone * pZone);
    void popActiveZone(ProfilingZone * pZone);
    void dumpFrame();
    void dumpStatistics();
    void reset();
    int getIndent();

    bool isCurrent() 
    {
        return boost::thread() == m_Thread;
    }
    const std::string& getName();

private:
    std::string m_sName;

    typedef std::list<ProfilingZone*> ZoneList;
    ZoneList m_Zones;
    ZoneList m_ActiveZones;
    bool m_bRunning;
    boost::thread m_Thread;
};

}

#endif
