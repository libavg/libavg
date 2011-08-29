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

#ifndef _Point_H_
#define _Point_H_

#include "../api.h"

#include <ostream>
#include <vector>

// Fix for non-C99 win compilers up to MSVC++2008
#if defined _MSC_VER
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
    ~Point();

    Point<NUM> getNormalized() const;
    Point<NUM> safeGetNormalized() const;
    double getNorm() const;
    bool isNaN() const;
    bool isInf() const;
    Point getRotated(double angle) const;
    Point getRotatedPivot(double angle, const Point& pivot = Point(0,0)) const;

    Point<NUM> & operator =(const Point<NUM>& p);

    bool operator ==(const Point<NUM> & pt) const;
    bool operator !=(const Point<NUM> & pt) const;
    void operator +=(const Point<NUM> & pt);
    void operator -=(const Point<NUM> & pt);
    void operator *=(NUM f);
    void operator /=(NUM f);
    Point<NUM> operator -() const;
    Point<NUM> operator +(const Point<NUM> & pt) const;
    Point<NUM> operator -(const Point<NUM> & pt) const;
    Point<NUM> operator /(double f) const;
    Point<NUM> operator *(double f) const;
    Point<NUM> operator *(const Point<NUM> & pt) const;

    static Point<NUM> fromPolar(double angle, double radius);
    double getAngle() const;
};

template<class NUM>
std::ostream& operator<<( std::ostream& os, const Point<NUM> &p);

template<class NUM>
Point<NUM> operator *(double f, const Point<NUM>& pt);

template<class NUM>
Point<NUM> operator /(double f, const Point<NUM>& pt);

template<class NUM>
template<class ORIGNUM>
Point<NUM>::Point(const Point<ORIGNUM>& p)
    : x(NUM(p.x)),
      y(NUM(p.y))
{
//    ObjectCounter::get()->incRef(&typeid(*this));
}

template<class NUM>
double calcDist(const Point<NUM>& pt1, const Point<NUM>& pt2);

template<class NUM>
double calcDistSquared(const Point<NUM>& pt1, const Point<NUM>& pt2);

typedef Point<double> DPoint;
typedef Point<int> IntPoint;

bool almostEqual(const DPoint& pt1, const DPoint& pt2);

inline
double dotProduct(const DPoint& pt1, const DPoint pt2)
{
    return pt1.x*pt2.x+pt1.y*pt2.y;
}

double vecAngle(const DPoint& pt1, const DPoint pt2);

typedef std::vector<DPoint> DPointVector;

}

#endif
