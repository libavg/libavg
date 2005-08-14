//
// $Id$
// 

#ifndef _Point_H_
#define _Point_H_

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
  return Point<NUM> (x/f, y/f);
}

}

#endif

