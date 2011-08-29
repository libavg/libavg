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

#include <iostream>

using namespace std;

namespace avg {

BezierCurve::BezierCurve(const DPoint& p0, const DPoint& p1, const DPoint& p2, 
        const DPoint& p3)
    : m_P0(p0),
      m_P1(p1),
      m_P2(p2),
      m_P3(p3)
{
}

DPoint BezierCurve::interpolate(double t) const
{
    return (1-t)*(1-t)*(1-t)*m_P0+
           3*t*(1-t)*(1-t)  *m_P1+
           3*t*t*(1-t)      *m_P2+
           t*t*t            *m_P3;
}
DPoint BezierCurve::getDeriv(double t) const
{
    return 3*(m_P1-m_P0)*(1-t)*(1-t)+
           6*(m_P2-m_P1)*(1-t)*t+ 
           3*(m_P3-m_P2)*t*t;
}

}
