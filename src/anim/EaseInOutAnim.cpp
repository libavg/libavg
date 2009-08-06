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

#include "EaseInOutAnim.h"

#include "../player/Player.h"
#include "../base/MathHelper.h"

#include <math.h>

using namespace boost::python;
using namespace std;

namespace avg {

EaseInOutAnim::EaseInOutAnim(const object& node, const string& sAttrName, 
            long long duration, const object& startValue, const object& endValue, 
            long long easeInDuration, long long easeOutDuration, bool bUseInt, 
            const object& startCallback, const object& stopCallback)
    : SimpleAnim(node, sAttrName, duration, startValue, endValue, bUseInt, startCallback,
            stopCallback),
      m_EaseInDuration(double(easeInDuration)/duration),
      m_EaseOutDuration(double(easeOutDuration)/duration)
{
}

EaseInOutAnim::~EaseInOutAnim()
{
}

double EaseInOutAnim::interpolate(double t)
{
    double accelDist = m_EaseInDuration*2/PI;
    double decelDist = m_EaseOutDuration*2/PI;
    double dist;
    if (t<m_EaseInDuration) {
        // Acceleration stage 
        double nt = t/m_EaseInDuration;
        double s = sin(-PI/2+nt*PI/2)+1;
        dist = s*accelDist;
    } else if (t > 1-m_EaseOutDuration) {
        // Deceleration stage
        double nt = (t-(1-m_EaseOutDuration))/m_EaseOutDuration;
        double s = sin(nt*PI/2);
        dist = accelDist+(1-m_EaseInDuration-m_EaseOutDuration)+s*decelDist;
    } else {
        // Linear stage
        dist = accelDist+t-m_EaseInDuration;
    }
    return dist/(accelDist+(1-m_EaseInDuration-m_EaseOutDuration)+decelDist);
}
    
}
