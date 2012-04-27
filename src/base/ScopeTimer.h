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

#ifndef _ScopeTimer_H_ 
#define _ScopeTimer_H_

#include "../api.h"
#include "ProfilingZoneID.h"
#include "ThreadProfiler.h"

namespace avg {

class AVG_API ScopeTimer {
public:
    ScopeTimer(ProfilingZoneID& zoneID)
    {
        if (s_bTimersEnabled) {
            m_pZoneID = &zoneID;
            m_pZoneID->getProfiler()->startZone(zoneID);
        } else {
            m_pZoneID = 0;
        }
    };

    ~ScopeTimer()
    {
        if (m_pZoneID) {
            m_pZoneID->getProfiler()->stopZone(*m_pZoneID);
        }
    };

    static void enableTimers(bool bEnable);

private:
    ProfilingZoneID* m_pZoneID;

    static bool s_bTimersEnabled;
};

}

#endif
