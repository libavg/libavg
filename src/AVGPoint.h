//
// $Id$
// 

#ifndef _AVGPoint_H_
#define _AVGPoint_H_

#include <paintlib/plpoint.h>

template<class NUM>
class AVGPoint
{
public:
  NUM x;
  NUM y;

  AVGPoint ();
  explicit AVGPoint(const PLPoint& p);
  AVGPoint (NUM X, NUM Y);

  bool operator == (const AVGPoint<NUM> & pt) const;
  bool operator != (const AVGPoint<NUM> & pt) const;
  void operator += (const AVGPoint<NUM> & pt);
  void operator -= (const AVGPoint<NUM> & pt);
  AVGPoint<NUM> operator - () const;
  AVGPoint<NUM> operator + (const AVGPoint<NUM> & pt) const;
  AVGPoint<NUM> operator - (const AVGPoint<NUM> & pt) const;
  AVGPoint<NUM> operator / (double f) const;
};

typedef AVGPoint<double> AVGDPoint;

template<class NUM>
AVGPoint<NUM>::AVGPoint ()
{}

template<class NUM>
AVGPoint<NUM>::AVGPoint(const PLPoint& p)
    : x(p.x),
      y(p.y)
{
}

template<class NUM>
AVGPoint<NUM>::AVGPoint (NUM X, NUM Y)
{
  x = X;
  y = Y;
}

template<class NUM>
bool AVGPoint<NUM>::operator == (const AVGPoint<NUM> & pt) const
{
  return (x == pt.x && y == pt.y);
}

template<class NUM>
bool AVGPoint<NUM>::operator != (const AVGPoint<NUM> & pt) const
{
  return (x != pt.x || y != pt.y);
}

template<class NUM>
void AVGPoint<NUM>::operator += (const AVGPoint<NUM>& pt)
{
  x += pt.x;
  y += pt.y;
}

template<class NUM>
void AVGPoint<NUM>::operator -= (const AVGPoint<NUM> & pt)
{
  x -= pt.x;
  y -= pt.y;
}

template<class NUM>
AVGPoint<NUM> AVGPoint<NUM>::operator - () const
{
  return AVGPoint<NUM>(-x, -y);
}

template<class NUM>
AVGPoint<NUM> AVGPoint<NUM>::operator + (const AVGPoint<NUM> & pt) const
{
  return AVGPoint<NUM>(x + pt.x, y + pt.y);
}

template<class NUM>
AVGPoint<NUM> AVGPoint<NUM>::operator - (const AVGPoint<NUM> & pt) const
{
  return AVGPoint<NUM>(x - pt.x, y - pt.y);
}

template<class NUM>
AVGPoint<NUM> AVGPoint<NUM>::operator / (double f) const
{
  return AVGPoint<NUM> (x/f, y/f);
}

#endif

