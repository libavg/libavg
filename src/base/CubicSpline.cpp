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

#include "CubicSpline.h"

#include <iostream>

using namespace std;

namespace avg {

CubicSpline::CubicSpline(const vector<double>& x, const vector<double>& y)
    : m_X(x),
      m_Y(y)
{
    assert(x.size() == y.size());
    // Add fake points before the first and after the last point so all derivatives
    // are defined.
    double edgeX = 2*m_X[0]-m_X[1];
    double edgeY = 2*m_Y[0]-m_Y[1];
    m_X.insert(m_X.begin(), edgeX);
    m_Y.insert(m_Y.begin(), edgeY);

    int len = m_X.size();
    edgeX = 2*m_X[len-1]-m_X[len-2];
    edgeY = 2*m_Y[len-1]-m_Y[len-2];
    m_X.push_back(edgeX);
    m_Y.push_back(edgeY);
}

CubicSpline::~CubicSpline()
{
}

double normedInterpolate(double y0, double y1, double y2, double y3,
    double mu)
{
   double mu2 = mu*mu;
   double a0 = y3 - y2 - y0 + y1;
   double a1 = y0 - y1 - a0;
   double a2 = y2 - y0;
   double a3 = y1;

   return(a0*mu*mu2+a1*mu2+a2*mu+a3);
}


double CubicSpline::interpolate(double orig)
{
    unsigned i = 0;
    if (m_X[m_X.size()-1] <= orig) {
        i = m_X.size();
    } else {
        while (m_X[i] < orig) {
            i++;
        }
    }
    if (i < 2) {
        double dxdy = (m_X[1]-m_X[0])/(m_Y[1]-m_Y[0]);
        return m_Y[1]+(orig-m_X[1])/dxdy;
    } else  if (i > m_X.size()-2) {
        unsigned len = m_X.size();
        double dxdy = (m_X[len-1]-m_X[len-2])/(m_Y[len-1]-m_Y[len-2]);
        return m_Y[len-2]+(orig-m_X[len-2])/dxdy;
    } else {
        double ratio = (orig-m_X[i-1])/(m_X[i]-m_X[i-1]);
        return normedInterpolate(m_Y[i-2], m_Y[i-1], m_Y[i], m_Y[i+1], ratio);
    }
}

}
