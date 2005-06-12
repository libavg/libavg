//
// $Id$
// 

#ifndef _Point_H_
#define _Point_H_

#include "../Object.h"

#include <paintlib/plpoint.h>

namespace avg {

template<class NUM>
class Point: public Object
{
public:
  NUM x;
  NUM y;

  Point ();
  explicit Point(const PLPoint& p);
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

  virtual JSFactoryBase* getFactory();
};

typedef Point<double> DPoint;

template<class NUM>
Point<NUM>::Point ()
{}

template<class NUM>
Point<NUM>::Point(const PLPoint& p)
    : x(p.x),
      y(p.y)
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
  return Point<NUM> (x/f, y/f);
}

template<class NUM>
JSFactoryBase* Point<NUM>::getFactory()
{
    // TODO: Make this work.
    return 0;
}

}

#endif

