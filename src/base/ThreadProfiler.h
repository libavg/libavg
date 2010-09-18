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
#include "ProfilingZone.h"

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/tss.hpp>

#include <list>
#include <map>

namespace avg {

class ThreadProfiler;
typedef boost::shared_ptr<ThreadProfiler> ThreadProfilerPtr;

class AVG_API ThreadProfiler {
public:
    static ThreadProfilerPtr& get();
    static void kill();
    ThreadProfiler();
    virtual ~ThreadProfiler();
    void setLogCategory(long category);
 
    int addZone(ProfilingZone& Zone);
    void clear();
    void start();
    bool isRunning();
    void pushActiveZone(ProfilingZone * pZone);
    void popActiveZone(ProfilingZone * pZone);
    void dumpFrame();
    void dumpStatistics();
    void reset();
    int getNumZones();

    const std::string& getName() const;
    void setName(const std::string& sName);

private:
    std::string m_sName;

    typedef std::list<ProfilingZone*> ZoneList;
    ZoneList m_Zones;
    ZoneList m_ActiveZones;
    bool m_bRunning;
    long m_LogCategory;

    static boost::thread_specific_ptr<ThreadProfilerPtr> s_pInstance;
};

}

#endif
