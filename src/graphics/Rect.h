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

#ifndef _Rect_H_
#define _Rect_H_

#include "Point.h"

namespace avg {

// Simple rectangle class.
// If NUM is an integer, contains all points from tl up to but not including 
// br.
template<class NUM>
class Rect
{
public:
    Point<NUM> tl;
    Point<NUM> br;

    Rect ();
    Rect (NUM left, NUM top, NUM right, NUM bottom);
    Rect (const Point<NUM>& TL, const Point<NUM>& BR);
    template<class ORIGNUM> Rect (const Rect<ORIGNUM>& rc);

    bool operator == (const Rect<NUM>& rect) const;
    bool operator != (const Rect<NUM> & rect) const;
    NUM Width () const;
    NUM Height () const;
    void SetWidth (NUM width);
    void SetHeight (NUM height);
    bool Contains (const Point<NUM>& pt) const;
    bool Contains (const Rect<NUM>& rect) const;
    bool Intersects (const Rect<NUM>& rect) const;
    void Expand (const Rect<NUM>& rect);
    void Intersect (const Rect<NUM>& rect);
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
Rect<NUM>::Rect ()
{}

template<class NUM>
Rect<NUM>::Rect (const Point<NUM>& TL, const Point<NUM>& BR)
    : tl(TL), br(BR)
{}

template<class NUM>
Rect<NUM>::Rect (NUM left, NUM top, NUM right, NUM bottom) 
    : tl(left, top), 
      br(right, bottom)
{}

template<class NUM>
template<class ORIGNUM>
Rect<NUM>::Rect (const Rect<ORIGNUM>& rc)
    : tl (NUM(rc.tl.x), NUM(rc.tl.y)),
      br (NUM(rc.br.x), NUM(rc.br.y))
{
}

template<class NUM>
bool Rect<NUM>::operator == (const Rect<NUM> & rect) const
{
  return (tl == rect.tl && br == rect.br);
}

template<class NUM>
bool Rect<NUM>::operator != (const Rect<NUM> & rect) const
{
  return !(rect==*this);
}

template<class NUM>
NUM Rect<NUM>::Width () const
{
  return br.x-tl.x;
}

template<class NUM>
NUM Rect<NUM>::Height () const
{
  return br.y-tl.y;
}

template<class NUM>
void Rect<NUM>::SetWidth (NUM width)
{
    br.x = tl.x+width;
}
 
template<class NUM>
void Rect<NUM>::SetHeight (NUM height)
{
    br.y = tl.y+height;
}

template<class NUM>
bool Rect<NUM>::Contains (const Point<NUM>& pt) const
{
    return (pt.x >= tl.x && pt.x < br.x &&
        pt.y >= tl.y && pt.y < br.y);
}

template<class NUM>
bool Rect<NUM>::Contains (const Rect<NUM>& rect) const
{
    Point<NUM> brpt (rect.br.x-1, rect.br.y-1);
    return Contains(rect.tl) && Contains(brpt);
}

template<class NUM>
bool Rect<NUM>::Intersects (const Rect<NUM>& rect) const
{   
    if (rect.br.x <= tl.x || rect.tl.x >= br.x ||
        rect.br.y <= tl.y || rect.tl.y >= br.y)
      return false;
    else
      return true;
}

#ifndef  min
#define min(a, b)       ((a) < (b) ? (a) : (b))
#endif

#ifndef  max
#define max(a, b)       ((a) < (b) ? (b) : (a))
#endif

template<class NUM>
void Rect<NUM>::Expand (const Rect<NUM>& rect)
{
    tl.x = min(tl.x, rect.tl.x);
    tl.y = min(tl.y, rect.tl.y);
    br.x = max(br.x, rect.br.x);
    br.y = max(br.y, rect.br.y);
}

template<class NUM>
void Rect<NUM>::Intersect (const Rect<NUM>& rect)
{
    tl.x = max(tl.x, rect.tl.x);
    tl.y = max(tl.y, rect.tl.y);
    br.x = min(br.x, rect.br.x);
    br.y = min(br.y, rect.br.y);
}

#undef min
#undef max
}

#endif

