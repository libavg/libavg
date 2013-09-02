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

#include "BezierCurve.h"

#include "GLMHelper.h"

#include <iostream>

using namespace std;

namespace avg {

BezierCurve::BezierCurve(const glm::vec2& p0, const glm::vec2& p1, const glm::vec2& p2, 
        const glm::vec2& p3)
    : m_P0(p0),
      m_P1(p1),
      m_P2(p2),
      m_P3(p3)
{
}

glm::vec2 BezierCurve::interpolate(float t) const
{
    return (1.f-t)*(1.f-t)*(1.f-t)*m_P0+
           3.f*t*(1.f-t)*(1.f-t)  *m_P1+
           3.f*t*t*(1.f-t)        *m_P2+
           t*t*t                  *m_P3;
}

glm::vec2 BezierCurve::getDeriv(float t) const
{
    return 3.f*(m_P1-m_P0)*(1.f-t)*(1.f-t)+
           6.f*(m_P2-m_P1)*(1.f-t)*t+ 
           3.f*(m_P3-m_P2)*t*t;
}

}
