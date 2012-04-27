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

#include "ProfilingZoneID.h"
#include "ThreadProfiler.h"

using namespace std;

namespace avg {

ProfilingZoneID::ProfilingZoneID(const string& sName, bool bMultithreaded)
    : m_sName(sName),
      m_bMultithreaded(bMultithreaded),
      m_pProfiler(0)
{
}

ProfilingZoneID::~ProfilingZoneID() 
{
}

const string& ProfilingZoneID::getName() const
{
    return m_sName;
}

ThreadProfiler* ProfilingZoneID::getProfiler()
{
    if (!m_pProfiler) {
        if (m_bMultithreaded) {
            return ThreadProfiler::get();
        } else {
            m_pProfiler = ThreadProfiler::get();
        }
    }
    return m_pProfiler;
}

}
