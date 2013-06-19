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
#include "MathHelper.h"

#include <iostream>

using namespace std;

namespace avg {

CubicSpline::CubicSpline(const vector<float>& x, const vector<float>& y, bool bLoop)
{
    AVG_ASSERT(x.size() == y.size());
    for (unsigned i=0; i<x.size(); ++i) {
        m_Pts.push_back(glm::vec2(x[i], y[i]));
    }
    init();
}

CubicSpline::CubicSpline(const vector<glm::vec2>& pts, bool bLoop)
    : m_Pts(pts)
{
    init();
}

CubicSpline::~CubicSpline()
{
}

float CubicSpline::interpolate(float orig)
{
    int len = m_Pts.size();
    int low = 0;
    int high = len-1;
    // Binary search.
    while (high - low > 1) {
        int avg = (high+low) / 2;
        if (m_Pts[avg].x > orig) {
            high = avg;
        } else {
            low = avg;
        }
    }
    float h = m_Pts[high].x - m_Pts[low].x;
    float a = (m_Pts[high].x-orig)/h;
    float b = (orig-m_Pts[low].x)/h;
    
    float y = a*m_Pts[low].y + b*m_Pts[high].y 
            + ((a*a*a-a)*m_Y2[low] + (b*b*b-b)*m_Y2[high])*(h*h)/6.f;
    return y;
}

void CubicSpline::init()
{
    int len = m_Pts.size();
    for (int i=1; i<len; ++i) {
        if (m_Pts[i].x <= m_Pts[i-1].x) {
            throw Exception(AVG_ERR_INVALID_ARGS,
                    "CubicSplines must have increasing x coordinates.");
        }
    }
    vector<float> u(len-1,0);
    m_Y2.push_back(0.f);
    u[0] = 0.f;
    for (int i=1; i<len-1; ++i) {
        float sig = (m_Pts[i].x-m_Pts[i-1].x) / (m_Pts[i+1].x-m_Pts[i-1].x);
        float p = sig * m_Y2[i-1]+2.0f;
        m_Y2.push_back((sig-1.0f)/p);
        u[i] = (m_Pts[i+1].y-m_Pts[i].y) / (m_Pts[i+1].x-m_Pts[i].x) - 
                (m_Pts[i].y - m_Pts[i-1].y) / (m_Pts[i].x-m_Pts[i-1].x);
        u[i] = (6.f*u[i]/(m_Pts[i+1].x-m_Pts[i-1].x) - sig*u[i-1]) / p;
    }
    float qn = 0.f;
    float un = 0.f;

    m_Y2.push_back((un-qn*u[len-2]) / (qn*m_Y2[len-2]-1.0f));

    for (int i=len-2; i>=0; i--) {
        m_Y2[i] = m_Y2[i]*m_Y2[i+1]+u[i];
    }
}

}
