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

#ifndef _Rect_H_
#define _Rect_H_

#include "../api.h"
#include "Point.h"

#include <algorithm>

namespace avg {

// Simple rectangle class.
// If NUM is an integer, contains all points from tl up to but not including 
// br.
template<class NUM>
class AVG_TEMPLATE_API Rect
{
public:
    Point<NUM> tl;
    Point<NUM> br;

    Rect();
    Rect(NUM left, NUM top, NUM right, NUM bottom);
    Rect(const Point<NUM>& TL, const Point<NUM>& BR);
    template<class ORIGNUM> Rect(const Rect<ORIGNUM>& rc);

    bool operator ==(const Rect<NUM>& rect) const;
    bool operator !=(const Rect<NUM> & rect) const;
    NUM width() const;
    NUM height() const;
    Point<NUM> center() const;
    void setWidth(NUM width);
    void setHeight(NUM height);
    void setSize(const Point<NUM>& size);
    bool contains(const Point<NUM>& pt) const;
    bool contains(const Rect<NUM>& rect) const;
    bool intersects(const Rect<NUM>& rect) const;
    void expand(const Rect<NUM>& rect);
    void intersect(const Rect<NUM>& rect);
    Point<NUM> size() const;
    Point<NUM> cropPoint(const Point<NUM>& pt) const;
};

template<class NUM>
std::ostream& operator<<( std::ostream& os, const Rect<NUM> &r)
{
    os << "(" << r.tl << "-" << r.br << ")";
    return os;
}


typedef Rect<double> DRect;
typedef Rect<int> IntRect;

template<class NUM>
Rect<NUM>::Rect()
{}

template<class NUM>
Rect<NUM>::Rect(const Point<NUM>& TL, const Point<NUM>& BR)
    : tl(TL), br(BR)
{}

template<class NUM>
Rect<NUM>::Rect(NUM left, NUM top, NUM right, NUM bottom) 
    : tl(left, top), 
      br(right, bottom)
{}

template<class NUM>
template<class ORIGNUM>
Rect<NUM>::Rect(const Rect<ORIGNUM>& rc)
    : tl (NUM(rc.tl.x), NUM(rc.tl.y)),
      br (NUM(rc.br.x), NUM(rc.br.y))
{
}

template<class NUM>
bool Rect<NUM>::operator ==(const Rect<NUM> & rect) const
{
  return (tl == rect.tl && br == rect.br);
}

template<class NUM>
bool Rect<NUM>::operator !=(const Rect<NUM> & rect) const
{
  return !(rect==*this);
}

template<class NUM>
NUM Rect<NUM>::width() const
{
  return br.x-tl.x;
}

template<class NUM>
NUM Rect<NUM>::height() const
{
  return br.y-tl.y;
}

template<class NUM>
Point<NUM> Rect<NUM>::center() const
{
    return Point<NUM>(tl+br)/2;
}

template<class NUM>
void Rect<NUM>::setWidth(NUM width)
{
    br.x = tl.x+width;
}
 
template<class NUM>
void Rect<NUM>::setHeight(NUM height)
{
    br.y = tl.y+height;
}
    
template<class NUM>
void Rect<NUM>::setSize(const Point<NUM>& size)
{
    setWidth(size.x);
    setHeight(size.y);
}

template<class NUM>
bool Rect<NUM>::contains(const Point<NUM>& pt) const
{
    return (pt.x >= tl.x && pt.x < br.x &&
        pt.y >= tl.y && pt.y < br.y);
}

template<class NUM>
bool Rect<NUM>::contains(const Rect<NUM>& rect) const
{
    Point<NUM> brpt (rect.br.x-1, rect.br.y-1);
    return Contains(rect.tl) && Contains(brpt);
}

template<class NUM>
bool Rect<NUM>::intersects(const Rect<NUM>& rect) const
{   
    if (rect.br.x <= tl.x || rect.tl.x >= br.x ||
        rect.br.y <= tl.y || rect.tl.y >= br.y)
      return false;
    else
      return true;
}

template<class NUM>
void Rect<NUM>::expand(const Rect<NUM>& rect)
{
    tl.x = min(tl.x, rect.tl.x);
    tl.y = min(tl.y, rect.tl.y);
    br.x = max(br.x, rect.br.x);
    br.y = max(br.y, rect.br.y);
}

template<class NUM>
void Rect<NUM>::intersect(const Rect<NUM>& rect)
{
    tl.x = max(tl.x, rect.tl.x);
    tl.y = max(tl.y, rect.tl.y);
    br.x = min(br.x, rect.br.x);
    br.y = min(br.y, rect.br.y);
}

template<class NUM>
Point<NUM> Rect<NUM>::size() const
{
    return Point<NUM>(width(), height());
}

template<class NUM>
Point<NUM> Rect<NUM>::cropPoint(const Point<NUM>& pt) const
{
    Point<NUM> Result;
    Result.x = std::min(std::max(pt.x, tl.x), br.x-1);
    Result.y = std::min(std::max(pt.y, tl.y), br.y-1);
    return Result;
}

#undef min
#undef max

}

#endif

