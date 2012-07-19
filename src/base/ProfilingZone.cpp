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

#include "ProfilingZone.h"
#include "ObjectCounter.h"

#include <iostream>

using namespace boost;
using namespace std;

namespace avg {

ProfilingZone::ProfilingZone(const ProfilingZoneID& zoneID)
    : m_TimeSum(0),
      m_AvgTime(0),
      m_NumFrames(0),
      m_Indent(0),
      m_ZoneID(zoneID)
{
    ObjectCounter::get()->incRef(&typeid(*this));
}

ProfilingZone::~ProfilingZone() 
{
    ObjectCounter::get()->decRef(&typeid(*this));
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

void ProfilingZone::setIndentLevel(int indent)
{
    m_Indent = indent;
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
    return m_ZoneID.getName();
}
    
}
