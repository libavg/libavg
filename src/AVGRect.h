//
// $Id$
// 

#ifndef _AVGRect_H_
#define _AVGRect_H_

#include "AVGPoint.h"

#include <paintlib/plrect.h>

// Simple rectangle class.
// If NUM is an integer, contains all points from tl up to but not including 
// br.
template<class NUM>
class AVGRect
{
public:
  AVGPoint<NUM> tl;
  AVGPoint<NUM> br;

  AVGRect ();
  AVGRect (NUM left, NUM top, NUM right, NUM bottom);
  AVGRect (const AVGPoint<NUM>& TL, const AVGPoint<NUM>& BR);
  AVGRect (const PLRect& rc);

  bool operator == (const AVGRect<NUM>& rect) const;
  bool operator != (const AVGRect<NUM> & rect) const;
  NUM Width () const;
  NUM Height () const;
  void SetWidth (NUM width);
  void SetHeight (NUM height);
  bool Contains (const AVGPoint<NUM>& pt) const;
  bool Contains (const AVGRect<NUM>& rect) const;
  bool Intersects (const AVGRect<NUM>& rect) const;
  void Expand (const AVGRect<NUM>& rect);
  void Intersect (const AVGRect<NUM>& rect);

  operator PLRect () const;
};

typedef AVGRect<double> AVGDRect;

template<class NUM>
AVGRect<NUM>::AVGRect ()
{}

template<class NUM>
AVGRect<NUM>::AVGRect (const AVGPoint<NUM>& TL, const AVGPoint<NUM>& BR)
    : tl(TL), br(BR)
{}

template<class NUM>
AVGRect<NUM>::AVGRect (NUM left, NUM top, NUM right, NUM bottom) 
    : tl(left, top), 
      br (right, bottom)
{}

template<class NUM>
AVGRect<NUM>::AVGRect (const PLRect& rc)
    : tl (rc.tl.x, rc.tl.y),
      br (rc.br.x, rc.br,y)
{
}

template<class NUM>
bool AVGRect<NUM>::operator == (const AVGRect<NUM> & rect) const
{
  return (tl == rect.tl && br == rect.br);
}

template<class NUM>
bool AVGRect<NUM>::operator != (const AVGRect<NUM> & rect) const
{
  return !(rect==*this);
}

template<class NUM>
NUM AVGRect<NUM>::Width () const
{
  return br.x-tl.x;
}

template<class NUM>
NUM AVGRect<NUM>::Height () const
{
  return br.y-tl.y;
}

template<class NUM>
void AVGRect<NUM>::SetWidth (NUM width)
{
    br.x = tl.x+width;
}
 
template<class NUM>
void AVGRect<NUM>::SetHeight (NUM height)
{
    br.y = tl.y+height;
}

template<class NUM>
bool AVGRect<NUM>::Contains (const AVGPoint<NUM>& pt) const
{
    return (pt.x >= tl.x && pt.x < br.x &&
        pt.y >= tl.y && pt.y < br.y);
}

template<class NUM>
bool AVGRect<NUM>::Contains (const AVGRect<NUM>& rect) const
{
    AVGPoint<NUM> brpt (rect.br.x-1, rect.br.y-1);
    return Contains(rect.tl) && Contains(brpt);
}

template<class NUM>
bool AVGRect<NUM>::Intersects (const AVGRect<NUM>& rect) const
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
void AVGRect<NUM>::Expand (const AVGRect<NUM>& rect)
{
    tl.x = min(tl.x, rect.tl.x);
    tl.y = min(tl.y, rect.tl.y);
    br.x = max(br.x, rect.br.x);
    br.y = max(br.y, rect.br.y);
}

template<class NUM>
void AVGRect<NUM>::Intersect (const AVGRect<NUM>& rect)
{
    tl.x = max(tl.x, rect.tl.x);
    tl.y = max(tl.y, rect.tl.y);
    br.x = min(br.x, rect.br.x);
    br.y = min(br.y, rect.br.y);
}

template<class NUM>
AVGRect<NUM>::operator PLRect () const
{
    return PLRect(int(tl.x), int(tl.y), int(br.x), int(br.y));
}

#undef min
#undef max

#endif

