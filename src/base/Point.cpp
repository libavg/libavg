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

#include "Point.h"

#include "MathHelper.h"
#include "Exception.h"

#if defined(__SSE__) || defined(_WIN32)
#include <xmmintrin.h>
#endif

#include <math.h>
#include <float.h>

#include <string>

namespace avg {

template<class NUM>
Point<NUM>::Point()
{
//    ObjectCounter::get()->incRef(&typeid(*this));
}

template<class NUM>
Point<NUM>::Point(NUM X, NUM Y)
{
//    ObjectCounter::get()->incRef(&typeid(*this));
    x = X;
    y = Y;
}

template<class NUM>
Point<NUM>::Point(const Point<NUM>& p)
{
//    ObjectCounter::get()->incRef(&typeid(*this));
    x = p.x;
    y = p.y;
}
    
template<class NUM>
Point<NUM>::Point(const std::vector<NUM>& v)
{
    AVG_ASSERT_MSG(v.size() == 2, 
            "Point can only be constructed from 2-component vector");
    x = v[0];
    y = v[1];
}

template<class NUM>
Point<NUM>::~Point()
{
//    ObjectCounter::get()->decRef(&typeid(*this));
}
    
template<class NUM>
double Point<NUM>::getNorm() const
{
    return sqrt(double(x*x+y*y));
}

template<class NUM>
bool Point<NUM>::isNaN() const
{
    return isnan(x) || isnan(y);
}

template<class NUM>
bool Point<NUM>::isInf() const
{
    return isinf(x) || isinf(y);
}

template<class NUM>
Point<NUM> Point<NUM>::getRotated(double angle) const
{
    double cosVal = cos(angle);
    double sinVal = sin(angle);
    return Point<NUM>(NUM(x*cosVal - y*sinVal), NUM(x*sinVal + y*cosVal));
}

template<class NUM>
Point<NUM> Point<NUM>::getRotatedPivot(double angle, const Point<NUM>& pivot) const
{
    // translate pivot to origin
    Point<NUM> translated = *this - pivot;
   
    // calculate rotated coordinates about the origin
    Point<NUM> rotated = translated.getRotated(angle);

    // re-translate pivot to original position
    rotated += pivot;

    return rotated;
}

template<class NUM>
Point<NUM>& Point<NUM>::operator =(const Point<NUM>& p)
{
    x = p.x;
    y = p.y;
    return *this;
}

template<class NUM>
bool Point<NUM>::operator ==(const Point<NUM> & pt) const
{
  return (x == pt.x && y == pt.y);
}

template<class NUM>
bool Point<NUM>::operator !=(const Point<NUM> & pt) const
{
  return (x != pt.x || y != pt.y);
}

template<class NUM>
void Point<NUM>::operator +=(const Point<NUM>& pt)
{
  x += pt.x;
  y += pt.y;
}

template<class NUM>
void Point<NUM>::operator -=(const Point<NUM> & pt)
{
  x -= pt.x;
  y -= pt.y;
}

template<class NUM>
void Point<NUM>::operator *=(NUM f)
{
  x *= f;
  y *= f;
}

template<class NUM>
void Point<NUM>::operator /=(NUM f)
{
  x /= f;
  y /= f;
}

template<class NUM>
Point<NUM> Point<NUM>::operator -() const
{
  return Point<NUM>(-x, -y);
}

template<class NUM>
Point<NUM> Point<NUM>::operator +(const Point<NUM> & pt) const
{
  return Point<NUM>(x + pt.x, y + pt.y);
}

template<class NUM>
Point<NUM> Point<NUM>::operator -(const Point<NUM> & pt) const
{
  return Point<NUM>(x - pt.x, y - pt.y);
}

template<class NUM>
Point<NUM> Point<NUM>::operator /(double f) const
{
  return Point<NUM> (NUM(x/f), NUM(y/f));
}

template<class NUM>
Point<NUM> Point<NUM>::operator *(double f) const
{
  return Point<NUM> (NUM(x*f), NUM(y*f));
}

template<class NUM>
Point<NUM> Point<NUM>::operator *(const Point<NUM>& pt) const
{
  return Point<NUM> (x*pt.x, y*pt.y);
}

template<>
Point<int> Point<int>::fromPolar(double angle, double radius)
{
    AVG_ASSERT_MSG(false, "fromPolar undefined for IntPoint");
    return Point<int>(0,0);
}

template<>
Point<double> Point<double>::fromPolar(double angle, double radius)
{
    return Point<double>(cos(angle)*radius, sin(angle)*radius);
}

template<class NUM>
double Point<NUM>::getAngle() const
{
    return atan2(double(y), double(x));
}

template<>
Point<int> Point<int>::getNormalized() const
{
    AVG_ASSERT_MSG(false, "getNormalized undefined for IntPoint");
    return Point<int>(0,0);
}

template<>
Point<double> Point<double>::getNormalized() const
{
    // This is imprecise but fast
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

template<>
Point<double> Point<double>::safeGetNormalized() const
{
    // This is precise but slower, and the version exported to python
    if (x==0 && y==0) {
        throw Exception(AVG_ERR_OUT_OF_RANGE, "Can't normalize (0,0).");
    } else {
        double invNorm = 1/sqrt(x*x+y*y);
        return Point<double>(x*invNorm, y*invNorm);
    }
}

template<>
Point<int> Point<int>::safeGetNormalized() const
{
    // Not implemented - done to silence compiler warnings.
    AVG_ASSERT(false);
    return Point<int>(0,0);
}

template<class NUM>
std::ostream& operator<<( std::ostream& os, const Point<NUM> &p)
{
    os << "(" << p.x << "," << p.y << ")";
    return os;
}

template<class NUM>
Point<NUM> operator *(double f, const Point<NUM>& pt)
{
    return pt*f;
}

template<class NUM>
Point<NUM> operator /(double f, const Point<NUM>& pt)
{
    return pt/f;
}

bool almostEqual(const DPoint& pt1, const DPoint& pt2)
{
    return (fabs(pt1.x-pt2.x)+fabs(pt1.y-pt2.y)) < 0.0001;
}

double vecAngle(const DPoint& pt1, const DPoint pt2)
{
    double angle = fmod((atan2(pt1.y, pt1.x) - atan2(pt2.y, pt2.x)), 2*M_PI);
    if (angle < 0) {
        angle += 2*M_PI;
    }
    return angle;
}

template<class NUM>
double calcDist(const Point<NUM>& pt1, const Point<NUM>& pt2)
{
    return sqrt(double(sqr(pt1.x-pt2.x)+sqr(pt1.y-pt2.y)));
}

// Explicit instantiations.
template class Point<double>;
template std::ostream& operator<<( std::ostream& os, const Point<double> &p);
template Point<double> operator *(double f, const Point<double>& pt);
template Point<double> operator /(double f, const Point<double>& pt);
template double calcDist(const Point<double>& pt1, const Point<double>& pt2);
template double calcDistSquared(const Point<double>& pt1, const Point<double>& pt2);

template class Point<int>;
template std::ostream& operator<<( std::ostream& os, const Point<int> &p);
template Point<int> operator *(double f, const Point<int>& pt);
template Point<int> operator /(double f, const Point<int>& pt);
template double calcDist(const Point<int>& pt1, const Point<int>& pt2);
template double calcDistSquared(const Point<int>& pt1, const Point<int>& pt2);

}
