//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2014 Ulrich von Zadow
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

#include "GLMHelper.h"
#include "StringHelper.h"
#include "../glm/glm.hpp"

#include <algorithm>

namespace avg {

// Simple rectangle class.
// If NUM is an integer, contains all points from tl up to but not including 
// br.
template<typename NUM, glm::precision precision>
class AVG_TEMPLATE_API Rect
{
public:
    typedef glm::detail::tvec2<NUM, precision> Vec2;
    Vec2 tl;
    Vec2 br;

    Rect();
    Rect(NUM left, NUM top, NUM right, NUM bottom);
    Rect(const Vec2& TL, const Vec2& BR);
    template<typename ORIGNUM, glm::precision origPrecision> Rect(const Rect<ORIGNUM, origPrecision>& rc);

    bool operator ==(const Rect<NUM, precision>& rect) const;
    bool operator !=(const Rect<NUM, precision>& rect) const;
    NUM width() const;
    NUM height() const;
    Vec2 center() const;
    void setWidth(NUM width);
    void setHeight(NUM height);
    void setSize(const Vec2& size);
    bool contains(const Vec2& pt) const;
    bool contains(const Rect<NUM, precision>& rect) const;
    bool intersects(const Rect<NUM, precision>& rect) const;
    void expand(const Rect<NUM, precision>& rect);
    void intersect(const Rect<NUM, precision>& rect);
    Vec2 size() const;
    Vec2 cropPoint(const Vec2& pt) const;
};

typedef Rect<float, glm::highp> FRect;
typedef Rect<int, glm::highp> IntRect;

template<typename NUM, glm::precision precision>
std::ostream& operator<<( std::ostream& os, const Rect<NUM, precision> &r)
{
    os << "(" << r.tl << "-" << r.br << ")";
    return os;
}

template<typename NUM, glm::precision precision>
std::istream& operator>>(std::istream& is, Rect<NUM, precision>& r)
{
    skipToken(is, '(');
    is >> r.tl;
    skipToken(is, ',');
    is >> r.br;
    skipToken(is, ')');
    return is;
}

inline IntRect stringToIntRect(const std::string& s)
{
    IntRect r;
    fromString(s, r);
    return r;
}

template<typename NUM, glm::precision precision>
Rect<NUM, precision>::Rect()
{}

template<typename NUM, glm::precision precision>
Rect<NUM, precision>::Rect(const Vec2& TL, const Vec2& BR)
    : tl(TL), br(BR)
{}

template<typename NUM, glm::precision precision>
Rect<NUM, precision>::Rect(NUM left, NUM top, NUM right, NUM bottom) 
    : tl(left, top), 
      br(right, bottom)
{}

template<typename NUM, glm::precision precision>
template<class ORIGNUM, glm::precision origPrecision>
Rect<NUM, precision>::Rect(const Rect<ORIGNUM, origPrecision>& rc)
    : tl (NUM(rc.tl.x), NUM(rc.tl.y)),
      br (NUM(rc.br.x), NUM(rc.br.y))
{
}

template<typename NUM, glm::precision precision>
bool Rect<NUM, precision>::operator ==(const Rect<NUM, precision> & rect) const
{
  return (tl == rect.tl && br == rect.br);
}

template<typename NUM, glm::precision precision>
bool Rect<NUM, precision>::operator !=(const Rect<NUM, precision> & rect) const
{
  return !(rect==*this);
}

template<typename NUM, glm::precision precision>
NUM Rect<NUM, precision>::width() const
{
  return br.x-tl.x;
}

template<typename NUM, glm::precision precision>
NUM Rect<NUM, precision>::height() const
{
  return br.y-tl.y;
}

template<typename NUM, glm::precision precision>
glm::detail::tvec2<NUM, precision> Rect<NUM, precision>::center() const
{
    return Vec2(tl+br)/2;
}

template<typename NUM, glm::precision precision>
void Rect<NUM, precision>::setWidth(NUM width)
{
    br.x = tl.x+width;
}
 
template<typename NUM, glm::precision precision>
void Rect<NUM, precision>::setHeight(NUM height)
{
    br.y = tl.y+height;
}
    
template<typename NUM, glm::precision precision>
void Rect<NUM, precision>::setSize(const Vec2& size)
{
    setWidth(size.x);
    setHeight(size.y);
}

template<typename NUM, glm::precision precision>
bool Rect<NUM, precision>::contains(const Vec2& pt) const
{
    return (pt.x >= tl.x && pt.x < br.x &&
        pt.y >= tl.y && pt.y < br.y);
}

template<typename NUM, glm::precision precision>
bool Rect<NUM, precision>::contains(const Rect<NUM, precision>& rect) const
{
    Vec2 brpt (rect.br.x-1, rect.br.y-1);
    return Contains(rect.tl) && Contains(brpt);
}

template<typename NUM, glm::precision precision>
bool Rect<NUM, precision>::intersects(const Rect<NUM, precision>& rect) const
{   
    if (rect.br.x <= tl.x || rect.tl.x >= br.x ||
        rect.br.y <= tl.y || rect.tl.y >= br.y)
      return false;
    else
      return true;
}

template<typename NUM, glm::precision precision>
void Rect<NUM, precision>::expand(const Rect<NUM, precision>& rect)
{
    if (width() == 0 && height() == 0) {
        *this = rect;
    } else {
        tl.x = glm::min(tl.x, rect.tl.x);
        tl.y = glm::min(tl.y, rect.tl.y);
        br.x = glm::max(br.x, rect.br.x);
        br.y = glm::max(br.y, rect.br.y);
    }
}

template<typename NUM, glm::precision precision>
void Rect<NUM, precision>::intersect(const Rect<NUM, precision>& rect)
{
    tl.x = glm::max(tl.x, rect.tl.x);
    tl.y = glm::max(tl.y, rect.tl.y);
    br.x = glm::min(br.x, rect.br.x);
    br.y = glm::min(br.y, rect.br.y);
}

template<typename NUM, glm::precision precision>
glm::detail::tvec2<NUM, precision> Rect<NUM, precision>::size() const
{
    return Vec2(width(), height());
}

template<typename NUM, glm::precision precision>
glm::detail::tvec2<NUM, precision> Rect<NUM, precision>::cropPoint(const Vec2& pt) const
{
    Vec2 Result;
    Result.x = std::min(std::max(pt.x, tl.x), br.x-1);
    Result.y = std::min(std::max(pt.y, tl.y), br.y-1);
    return Result;
}

#undef min
#undef max

}

#endif

