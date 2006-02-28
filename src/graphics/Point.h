//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
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

#include <ostream>

namespace avg {

template<class NUM>
class Point
{
public:
  NUM x;
  NUM y;

  Point ();
  template<class ORIGNUM> explicit Point(const Point<ORIGNUM>& p);
  Point (NUM X, NUM Y);
  Point(const Point<NUM>& p);

  Point<NUM> & operator = (const Point<NUM>& p);

  bool operator == (const Point<NUM> & pt) const;
  bool operator != (const Point<NUM> & pt) const;
  void operator += (const Point<NUM> & pt);
  void operator -= (const Point<NUM> & pt);
  Point<NUM> operator - () const;
  Point<NUM> operator + (const Point<NUM> & pt) const;
  Point<NUM> operator - (const Point<NUM> & pt) const;
  Point<NUM> operator / (double f) const;

};

template<class NUM>
std::ostream& operator<<( std::ostream& os, const Point<NUM> &p)
{
    os << "(" << p.x << "," << p.y << ")";
    return os;
}

typedef Point<double> DPoint;
typedef Point<int> IntPoint;

template<class NUM>
Point<NUM>::Point ()
{}

template<class NUM>
template<class ORIGNUM>
Point<NUM>::Point(const Point<ORIGNUM>& p)
    : x(NUM(p.x)),
      y(NUM(p.y))
{
}

template<class NUM>
Point<NUM>::Point (NUM X, NUM Y)
{
  x = X;
  y = Y;
}

template<class NUM>
Point<NUM>::Point(const Point<NUM>& p)
{
    x = p.x;
    y = p.y;
}

template<class NUM>
Point<NUM>& Point<NUM>::operator = (const Point<NUM>& p)
{
    x = p.x;
    y = p.y;
    return *this;
}

template<class NUM>
bool Point<NUM>::operator == (const Point<NUM> & pt) const
{
  return (x == pt.x && y == pt.y);
}

template<class NUM>
bool Point<NUM>::operator != (const Point<NUM> & pt) const
{
  return (x != pt.x || y != pt.y);
}

template<class NUM>
void Point<NUM>::operator += (const Point<NUM>& pt)
{
  x += pt.x;
  y += pt.y;
}

template<class NUM>
void Point<NUM>::operator -= (const Point<NUM> & pt)
{
  x -= pt.x;
  y -= pt.y;
}

template<class NUM>
Point<NUM> Point<NUM>::operator - () const
{
  return Point<NUM>(-x, -y);
}

template<class NUM>
Point<NUM> Point<NUM>::operator + (const Point<NUM> & pt) const
{
  return Point<NUM>(x + pt.x, y + pt.y);
}

template<class NUM>
Point<NUM> Point<NUM>::operator - (const Point<NUM> & pt) const
{
  return Point<NUM>(x - pt.x, y - pt.y);
}

template<class NUM>
Point<NUM> Point<NUM>::operator / (double f) const
{
  return Point<NUM> (NUM(x/f), NUM(y/f));
}

}

#endif

