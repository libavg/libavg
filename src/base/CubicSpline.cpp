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

#include "CubicSpline.h"
#include "Exception.h"

#include <iostream>

using namespace std;

namespace avg {

CubicSpline::CubicSpline(const vector<float>& x, const vector<float>& y)
{
    AVG_ASSERT(x.size() == y.size());
    for (unsigned i=0; i<x.size(); ++i) {
        m_Pts.push_back(glm::vec2(x[i], y[i]));
    }
    init();
}

CubicSpline::CubicSpline(const vector<glm::vec2>& pts)
    : m_Pts(pts)
{
    init();
}

CubicSpline::~CubicSpline()
{
}

float normedInterpolate(float y0, float y1, float y2, float y3, float mu)
{
   float mu2 = mu*mu;
   float a0 = y3 - y2 - y0 + y1;
   float a1 = y0 - y1 - a0;
   float a2 = y2 - y0;
   float a3 = y1;

   return(a0*mu*mu2+a1*mu2+a2*mu+a3);
}


float CubicSpline::interpolate(float orig)
{
    unsigned i = 0;
    unsigned size = m_Pts.size();
    if (m_Pts[size-1].x <= orig) {
        i = m_Pts.size();
    } else {
        while (m_Pts[i].x < orig) {
            i++;
        }
    }
    if (i < 2) {
        float dxdy = (m_Pts[1].x-m_Pts[0].x)/(m_Pts[1].y-m_Pts[0].y);
        return m_Pts[1].y+(orig-m_Pts[1].x)/dxdy;
    } else if (i > size-2) {
        float dxdy = (m_Pts[size-1].x-m_Pts[size-2].x)/(m_Pts[size-1].y-m_Pts[size-2].y);
        return m_Pts[size-2].y+(orig-m_Pts[size-2].x)/dxdy;
    } else {
        float ratio = (orig-m_Pts[i-1].x)/(m_Pts[i].x-m_Pts[i-1].x);
        return normedInterpolate(m_Pts[i-2].y, m_Pts[i-1].y, m_Pts[i].y, m_Pts[i+1].y,
                ratio);
    }
}

void CubicSpline::init()
{
    // Add fake points before the first and after the last point so all derivatives
    // are defined.
    glm::vec2 edge = 2.f*m_Pts[0]-m_Pts[1];
    m_Pts.insert(m_Pts.begin(), edge);

    int len = m_Pts.size();
    edge = 2.f*m_Pts[len-1]-m_Pts[len-2];
    m_Pts.push_back(edge);
}

}
