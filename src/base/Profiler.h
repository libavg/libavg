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

#ifndef _Profiler_H_ 
#define _Profiler_H_

#include "ProfilingZone.h"

#include <list>

namespace avg {
    
class Profiler {
public:
    static Profiler& get();
    virtual ~Profiler();
 
    void addZone(ProfilingZone& Zone);
    void clear();
    void start();
    bool isRunning();
    void setActiveZone(ProfilingZone * pZone);
    void dumpFrame();
    void dumpStatistics();
    void reset();


private:
    Profiler();

    typedef std::list<ProfilingZone*> ZoneList;
    ZoneList m_Zones;
    ProfilingZone * m_pActiveZone;
    bool m_bRunning;
};

}

#endif
