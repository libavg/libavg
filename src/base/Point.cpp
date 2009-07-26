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

#include "Point.h"

#include <math.h>

#if defined(__SSE__) || defined(_WIN32)
#include <xmmintrin.h>
#endif

namespace avg {

template<>
Point<double>::Point(const std::string& s)
{
    std::string sCropped = removeStartEndSpaces(s);
    if (sCropped[0] != '(' || sCropped[sCropped.length()-1] != ')') {
        throw (Exception(AVG_ERR_TYPE, std::string("Could not convert ")+s+" to Point."));
    }
    std::string::size_type pos = sCropped.find(',');
    x = stringToDouble(sCropped.substr(1, pos-1));
    y = stringToDouble(sCropped.substr(pos+1, sCropped.length()-pos-2));
}


template<>
Point<double> Point<double>::getNormalized() const
{
#if defined(__SSE__) || defined(_WIN32)
#pragma pack(16)
    float result[4];
    float normSqr = float(x*x+y*y);
    __m128 src = _mm_setr_ps(float(x), float(y), 0, 0);
    __m128 normSqrVec = _mm_set_ps1(normSqr);
    __m128 invSqrt = _mm_rsqrt_ps(normSqrVec);
    __m128 resultVec = _mm_mul_ps(src, invSqrt);
    _mm_storeu_ps(result, resultVec);
    return Point<double>(result[0], result[1]);
#pragma pack()
#else
    double invNorm = invSqrt(float(x*x+y*y));
    if (invNorm != 0) {
        return Point<double>(x*invNorm, y*invNorm);
    } else {
        return *this;
    }
#endif
}

bool almostEqual(const DPoint& pt1, const DPoint& pt2)
{
    return (fabs(pt1.x-pt2.x)+fabs(pt1.y-pt2.y)) < 0.0001;
}

}
