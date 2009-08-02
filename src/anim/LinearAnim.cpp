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

#include "LinearAnim.h"

#include "../player/Player.h"

using namespace boost::python;
using namespace std;

namespace avg {

LinearAnim::LinearAnim(const object& node, const string& sAttrName, double duration,
            const object& startValue, const object& endValue, bool bUseInt, 
            const object& startCallback, const object& stopCallback)
    : SimpleAnim(node, sAttrName, duration, bUseInt, startCallback, stopCallback),
      m_StartValue(startValue),
      m_EndValue(endValue)
{
}

LinearAnim::~LinearAnim()
{
}

void LinearAnim::step(double t)
{
    SimpleAnim::step(t);
    object curValue = m_StartValue+(m_EndValue-m_StartValue)*t;
/*        if (getUseInt()) {
            curValue = 
        }
*/        
    setValue(curValue);
}
    
void LinearAnim::regularStop()
{
    setValue(m_EndValue);
    remove();
}

double LinearAnim::calcStartTime()
{
    double part = extract<double>((getValue()-m_StartValue)/(m_EndValue-m_StartValue));
    return Player::get()->getFrameTime()-part*getDuration();
}

}
