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

#ifndef _Point_H_
#define _Point_H_

#include "../api.h"
#include "MathHelper.h"
#include "ObjectCounter.h"
#include "StringHelper.h"

#include <ostream>
#include <vector>
#include <string>
#include <math.h>
#include <assert.h>
#include <float.h>

// Fix for non-C99 win compilers up to MSVC++2008
#if defined _MSC_VER && _MSC_VER <= 1500
#define isinf(x) (!_finite(x))
#define isnan(x) (_isnan(x))
#endif

namespace avg {

template<class NUM>
class AVG_TEMPLATE_API Point
{
public:
    NUM x;
    NUM y;

    Point ();
    template<class ORIGNUM> explicit Point(const Point<ORIGNUM>& p);
    Point(NUM X, NUM Y);
    Point(const Point<NUM>& p);
    Point(const std::vector<NUM>& v);
    Point(const std::string& s);
    ~Point();

    Point<NUM> getNormalized() const;
    double getNorm();
    bool isNaN() const;
    bool isInf() const;
    Point getRotated(double angle) const;
    Point getRotatedPivot(double angle, const Point& pivot = Point(0,0)) const;

    Point<NUM> & operator =(const Point<NUM>& p);

    bool operator ==(const Point<NUM> & pt) const;
    bool operator !=(const Point<NUM> & pt) const;
    void operator +=(const Point<NUM> & pt);
    void operator -=(const Point<NUM> & pt);
    void operator *=(double f);
    void operator /=(NUM f);
    Point<NUM> operator -() const;
    Point<NUM> operator +(const Point<NUM> & pt) const;
    Point<NUM> operator -(const Point<NUM> & pt) const;
    Point<NUM> operator /(double f) const;
    Point<NUM> operator *(double f) const;
    Point<NUM> operator *(const Point<NUM> & pt) const;
};

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

typedef Point<double> DPoint;
typedef Point<int> IntPoint;

template<class NUM>
Point<NUM>::Point()
{
//    ObjectCounter::get()->incRef(&typeid(*this));
}

template<class NUM>
template<class ORIGNUM>
Point<NUM>::Point(const Point<ORIGNUM>& p)
    : x(NUM(p.x)),
      y(NUM(p.y))
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
    assert(v.size() == 2);
    x = v[0];
    y = v[1];
}

template<class NUM>
Point<NUM>::~Point()
{
//    ObjectCounter::get()->decRef(&typeid(*this));
}
    
template<class NUM>
double Point<NUM>::getNorm()
{
    return sqrt(x*x+y*y);
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
    return Point<NUM>(x*cosVal - y*sinVal, x*sinVal + y*cosVal);
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
void Point<NUM>::operator *=(double f)
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

inline
double sqr(double x)
{
    return x*x;
}

template<class NUM>
double calcDist(const Point<NUM>& pt1, const Point<NUM>& pt2)
{
    return sqrt(sqr(pt1.x-pt2.x)+sqr(pt1.y-pt2.y));
}

template<class NUM>
double calcDistSquared(const Point<NUM>& pt1, const Point<NUM>& pt2)
{
    return sqr(pt1.x-pt2.x)+sqr(pt1.y-pt2.y);
}

bool almostEqual(const DPoint& pt1, const DPoint& pt2);

inline
double dotProduct(const DPoint& pt1, const DPoint pt2)
{
    return pt1.x*pt2.x+pt1.y*pt2.y;
}

typedef std::vector<DPoint> DPointVector;

}

#endif
