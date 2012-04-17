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

//
// Based on Poly2Tri algorithm.
// Poly2Tri Copyright (c) 2009-2010, Poly2Tri Contributors
// http://code.google.com/p/poly2tri/
//

#ifndef SHAPES_H
#define SHAPES_H

#include <vector>
#include <cstddef>
#include <assert.h>
#include <cmath>

namespace avg {

struct Edge;

struct Point
{
    double m_X, m_Y;
    int m_Index;

    /// The edges this point constitutes an upper ending point
    std::vector<Edge*> m_EdgeList;

    /// Default constructor does nothing (for performance).
    Point() {
        m_X = 0.0;
        m_Y = 0.0;
        m_Index = 0;
    }

    Point(double x, double y, int index) :
            m_X(x), m_Y(y), m_Index(index) {}

    void set_zero()
    {
        m_X = 0.0;
        m_Y = 0.0;
    }

    void set(double x_, double y_)
    {
        m_X = x_;
        m_Y = y_;
    }

    Point operator -() const
    {
        Point v;
        v.set(-m_X, -m_Y);
        return v;
    }

    void operator +=(const Point& v)
    {
        m_X += v.m_X;
        m_Y += v.m_Y;
    }

    void operator -=(const Point& v)
    {
        m_X -= v.m_X;
        m_Y -= v.m_Y;
    }

    void operator *=(double a)
    {
        m_X *= a;
        m_Y *= a;
    }

    double length() const
    {
        return sqrt(m_X * m_X + m_Y * m_Y);
    }

    /// Convert this point into a unit point. Returns the Length.
    double normalize()
    {
        double len = length();
        m_X /= len;
        m_Y /= len;
        return len;
    }

};


// Represents a simple polygon's edge
struct Edge
{
    Point* m_P, *m_Q;

    Edge(Point& p1, Point& p2) :m_P(&p1), m_Q(&p2)
    {
        if (p1.m_Y > p2.m_Y) {
            m_Q = &p1;
            m_P = &p2;
        } else if (p1.m_Y == p2.m_Y) {
            if (p1.m_X > p2.m_X) {
                m_Q = &p1;
                m_P = &p2;
            } else if (p1.m_X == p2.m_X) {
                // Repeat points
                assert(false);
            }
        }
        m_Q->m_EdgeList.push_back(this);
    }
};


class TriangulationTriangle
{

public:

    TriangulationTriangle(Point& a, Point& b, Point& c);

/// Flags to determine if an edge is a Constrained edge
    bool m_ConstrainedEdge[3];
/// Flags to determine if an edge is a Delauney edge
    bool m_DelaunayEdge[3];

    Point* getPoint(const int& index);
    Point* pointCW(Point& point);
    Point* pointCCW(Point& point);
    Point* oppositePoint(TriangulationTriangle& t, Point& p);

    TriangulationTriangle* getNeighbor(const int& index);
    void markNeighbor(Point* p1, Point* p2, TriangulationTriangle* t);
    void markNeighbor(TriangulationTriangle& t);

    void markConstrainedEdge(const int index);
    void markConstrainedEdge(Edge& edge);
    void markConstrainedEdge(Point* p, Point* q);

    unsigned int index(const Point* p);
    unsigned int edgeIndex(const Point* p1, const Point* p2);

    TriangulationTriangle* neighborCW(Point& point);
    TriangulationTriangle* neighborCCW(Point& point);
    bool getConstrainedEdgeCCW(Point& p);
    bool getConstrainedEdgeCW(Point& p);
    void setConstrainedEdgeCCW(Point& p, bool ce);
    void setConstrainedEdgeCW(Point& p, bool ce);
    bool getDelunayEdgeCCW(Point& p);
    bool getDelunayEdgeCW(Point& p);
    void setDelunayEdgeCCW(Point& p, bool e);
    void setDelunayEdgeCW(Point& p, bool e);

    bool contains(Point* p);
    bool contains(const Edge& e);
    bool contains(Point* p, Point* q);
    void legalize(Point& point);
    void legalize(Point& opoint, Point& npoint);

    void clear();
    void clearNeighbor(TriangulationTriangle *triangle);
    void clearNeighbors();
    void clearDelunayEdges();

    inline bool isInterior();
    inline void isInterior(bool b);

    TriangulationTriangle& neighborAcross(Point& opoint);

private:

    Point* m_Points[3];

    TriangulationTriangle* m_Neighbors[3];

    bool m_Interior;
};

inline bool cmp(const Point* a, const Point* b)
{
    if (a->m_Y < b->m_Y) {
        return true;
    } else if (a->m_Y == b->m_Y) {
        // Make sure q is point with greater x value
        if (a->m_X < b->m_X) {
            return true;
        }
    }
    return false;
}
/*
 inline Point operator +(const Point& a, const Point& b)
 {
 return Point(a.x + b.x, a.y + b.y);
 }

 inline Point operator -(const Point& a, const Point& b)
 {
 return Point(a.x - b.x, a.y - b.y);
 }

 inline Point operator *(double s, const Point& a)
 {
 return Point(s * a.x, s * a.y, a.index);
 } */

inline bool operator ==(const Point& a, const Point& b)
{
    return a.m_X == b.m_X && a.m_Y == b.m_Y;
}

inline bool operator !=(const Point& a, const Point& b)
{
    return a.m_X != b.m_X && a.m_Y != b.m_Y;
}

inline double dot(const Point& a, const Point& b)
{
    return a.m_X * b.m_X + a.m_Y * b.m_Y;
}

inline double cross(const Point& a, const Point& b)
{
    return a.m_X * b.m_Y - a.m_Y * b.m_X;
}

inline Point cross(const Point& a, double s)
{
    return Point(s * a.m_Y, -s * a.m_X, a.m_Index);
}

inline Point cross(const double s, const Point& a)
{
    return Point(-s * a.m_Y, s * a.m_X, a.m_Index);
}

inline Point* TriangulationTriangle::getPoint(const int& index)
{
    return m_Points[index];
}

inline TriangulationTriangle* TriangulationTriangle::getNeighbor(
        const int& index)
{
    return m_Neighbors[index];
}

inline bool TriangulationTriangle::contains(Point* p)
{
    return p == m_Points[0] || p == m_Points[1] || p == m_Points[2];
}

inline bool TriangulationTriangle::contains(const Edge& e)
{
    return contains(e.m_P) && contains(e.m_Q);
}

inline bool TriangulationTriangle::contains(Point* p, Point* q)
{
    return contains(p) && contains(q);
}

inline bool TriangulationTriangle::isInterior()
{
    return m_Interior;
}

inline void TriangulationTriangle::isInterior(bool b)
{
    m_Interior = b;
}

}

#endif
