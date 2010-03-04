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

#include "ProfilingZone.h"
#include "ThreadProfiler.h"
#include "ObjectCounter.h"
#include "Exception.h"

#include <iostream>

using namespace boost;
using namespace std;

namespace avg {

ProfilingZone::ProfilingZone(const string& sName, bool bIsStatic)
    : m_sName(sName),
      m_TimeSum(0),
      m_AvgTime(0),
      m_NumFrames(0),
      m_Indent(0),
      m_bIsRegistered(false),
      m_bIsStatic(bIsStatic)
{
    ObjectCounter::get()->incRef(&typeid(*this));
}

ProfilingZone::~ProfilingZone() 
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void ProfilingZone::clear()
{
    m_TimeSum = 0;
    m_AvgTime = 0;
    m_NumFrames = 0;
    m_bIsRegistered = false;
}

bool ProfilingZone::isStatic()
{
    return m_bIsStatic;
}

void ProfilingZone::start()
{
    ThreadProfilerPtr pProfiler = ThreadProfiler::get();
    if (!pProfiler) {
        AVG_ASSERT(false);
    }
    // Start gets called when the zone is first entered. 
    if (!m_bIsRegistered && pProfiler->isRunning()) {
        // This stuff makes sure that the zones are registered in the order 
        // they are entered.
        pProfiler->addZone(*this);
        m_Indent = pProfiler->getIndent();
        clear();
        m_bIsRegistered = true;
    }
    pProfiler->pushActiveZone(this);
}

void ProfilingZone::reset()
{
    m_NumFrames++;
    m_AvgTime = (m_AvgTime*(m_NumFrames-1)+m_TimeSum)/m_NumFrames;
    m_TimeSum = 0;
}

long long ProfilingZone::getUSecs() const
{
    return m_TimeSum;
}

long long ProfilingZone::getAvgUSecs() const
{
    return m_AvgTime;
}

int ProfilingZone::getIndentLevel() const
{
    return m_Indent;
}

string ProfilingZone::getIndentString() const 
{
    return string(m_Indent, ' ');
}

const string& ProfilingZone::getName() const
{
    return m_sName;
}
    
void ProfilingZone::add(long long usecs)
{
    ThreadProfiler::get()->popActiveZone(this);
    m_TimeSum += usecs;
}

}
