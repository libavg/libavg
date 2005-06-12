//
// $Id$
// 

#ifndef _Rect_H_
#define _Rect_H_

#include "Point.h"

#include <paintlib/plrect.h>

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
  Rect (const PLRect& rc);

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

  operator PLRect () const;
};

typedef Rect<double> DRect;

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
      br (right, bottom)
{}

template<class NUM>
Rect<NUM>::Rect (const PLRect& rc)
    : tl (rc.tl.x, rc.tl.y),
      br (rc.br.x, rc.br.y)
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

template<class NUM>
Rect<NUM>::operator PLRect () const
{
    return PLRect(int(tl.x), int(tl.y), int(br.x), int(br.y));
}

#undef min
#undef max
}

#endif

