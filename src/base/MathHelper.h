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

#ifndef _MathHelper_H_
#define _MathHelper_H_

#include "../api.h"
#include <vector>
#include <math.h>

#define PI 3.14159265358979323846
#define EPSILON 0.00001

namespace avg {

bool ispow2(int n);

int nextpow2(int n);

int safeCeil(double d);

bool almostEqual(double d1, double d2, double epsilon=EPSILON);

float invSqrt(float x);

template<class T>
T sqr(T d)
{
    return d*d;
}

template<class T>
int sgn(T val)
{
    return int(val/fabs(val));
}

template<class T>
std::vector<T> vectorFromCArray(int n, T* pData)
{
    std::vector<T> v;
    for (int i=0; i<n; ++i) {
        v.push_back(*(pData+i));
    }
    return v;
}

template<class T>
std::vector<std::vector<T> > vector2DFromCArray(int n, int m, T* pData)
{
    std::vector<std::vector<T> > v(4, std::vector<T>());
    for (int i=0; i<n; ++i) {
        for (int j=0; j<m; ++j) {
            v[i].push_back(*(pData+j+i*m));
        }
    }
    return v;
}

template<typename T>
T signum(T n)
{
    if (n < 0) return -1;
    if (n > 0) return 1;
    return 0;
}

}
#endif
 
