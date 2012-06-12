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

#include "Shapes.h"
#include <iostream>

namespace avg {

TriangulationTriangle::TriangulationTriangle(Point& a, Point& b, Point& c)
{
    m_Points[0] = &a;
    m_Points[1] = &b;
    m_Points[2] = &c;
    m_Neighbors[0] = NULL;
    m_Neighbors[1] = NULL;
    m_Neighbors[2] = NULL;
    m_ConstrainedEdge[0] = m_ConstrainedEdge[1] = m_ConstrainedEdge[2] = false;
    m_DelaunayEdge[0] = m_DelaunayEdge[1] = m_DelaunayEdge[2] = false;
    m_Interior = false;
}

// Update neighbor pointers
void TriangulationTriangle::markNeighbor(Point* p1, Point* p2,
        TriangulationTriangle* t)
{
    if ((p1 == m_Points[2] && p2 == m_Points[1])
            || (p1 == m_Points[1] && p2 == m_Points[2])) {
        m_Neighbors[0] = t;
    } else if ((p1 == m_Points[0] && p2 == m_Points[2])
            || (p1 == m_Points[2] && p2 == m_Points[0])) {
        m_Neighbors[1] = t;
    } else if ((p1 == m_Points[0] && p2 == m_Points[1])
            || (p1 == m_Points[1] && p2 == m_Points[0])) {
        m_Neighbors[2] = t;
    } else {
        assert(0);
    }

}

// Exhaustive search to update neighbor pointers
void TriangulationTriangle::markNeighbor(TriangulationTriangle& t)
{
    if (t.contains(m_Points[1], m_Points[2])) {
        m_Neighbors[0] = &t;
        t.markNeighbor(m_Points[1], m_Points[2], this);
    } else if (t.contains(m_Points[0], m_Points[2])) {
        m_Neighbors[1] = &t;
        t.markNeighbor(m_Points[0], m_Points[2], this);
    } else if (t.contains(m_Points[0], m_Points[1])) {
        m_Neighbors[2] = &t;
        t.markNeighbor(m_Points[0], m_Points[1], this);
    }
}

void TriangulationTriangle::clear()
{
    TriangulationTriangle *t;
    for (int i = 0; i < 3; i++) {
        t = m_Neighbors[i];
        if (t != NULL) {
            t->clearNeighbor(this);
        }
    }
    clearNeighbors();
    m_Points[0] = m_Points[1] = m_Points[2] = NULL;
}

void TriangulationTriangle::clearNeighbor(TriangulationTriangle *triangle)
{
    if (m_Neighbors[0] == triangle) {
        m_Neighbors[0] = NULL;
    } else if (m_Neighbors[1] == triangle) {
        m_Neighbors[1] = NULL;
    } else {
        m_Neighbors[2] = NULL;
    }
}

void TriangulationTriangle::clearNeighbors()
{
    m_Neighbors[0] = NULL;
    m_Neighbors[1] = NULL;
    m_Neighbors[2] = NULL;
}

void TriangulationTriangle::clearDelunayEdges()
{
    m_DelaunayEdge[0] = m_DelaunayEdge[1] = m_DelaunayEdge[2] = false;
}

Point* TriangulationTriangle::oppositePoint(TriangulationTriangle& t,
        Point& p) {
    Point *cw = t.pointCW(p);
    double x = p.m_X;
    double y = p.m_Y;
    return pointCW(*cw);
}

// Legalized triangle by rotating clockwise around point(0)
void TriangulationTriangle::legalize(Point& point)
{
    m_Points[1] = m_Points[0];
    m_Points[0] = m_Points[2];
    m_Points[2] = &point;
}

// Legalize triagnle by rotating clockwise around oPoint
void TriangulationTriangle::legalize(Point& opoint, Point& npoint)
{
    if (&opoint == m_Points[0]) {
        m_Points[1] = m_Points[0];
        m_Points[0] = m_Points[2];
        m_Points[2] = &npoint;
    } else if (&opoint == m_Points[1]) {
        m_Points[2] = m_Points[1];
        m_Points[1] = m_Points[0];
        m_Points[0] = &npoint;
    } else if (&opoint == m_Points[2]) {
        m_Points[0] = m_Points[2];
        m_Points[2] = m_Points[1];
        m_Points[1] = &npoint;
    } else {
        assert(0);
    }
}

unsigned int TriangulationTriangle::index(const Point* p)
{
    if (p == m_Points[0]) {
        return 0;
    } else if (p == m_Points[1]) {
        return 1;
    } else if (p == m_Points[2]) {
        return 2;
    }
    assert(0);
    return 0;
}

unsigned int TriangulationTriangle::edgeIndex(const Point* p1, const Point* p2)
{
    if (m_Points[0] == p1) {
        if (m_Points[1] == p2) {
            return 2;
        } else if (m_Points[2] == p2) {
            return 1;
        }
    } else if (m_Points[1] == p1) {
        if (m_Points[2] == p2) {
            return 0;
        } else if (m_Points[0] == p2) {
            return 2;
        }
    } else if (m_Points[2] == p1) {
        if (m_Points[0] == p2) {
            return 1;
        } else if (m_Points[1] == p2) {
            return 0;
        }
    }
    return -1;
}

void TriangulationTriangle::markConstrainedEdge(const int index)
{
    m_ConstrainedEdge[index] = true;
}

void TriangulationTriangle::markConstrainedEdge(Edge& edge)
{
    markConstrainedEdge(edge.m_P, edge.m_Q);
}

void TriangulationTriangle::markConstrainedEdge(Point* p, Point* q)
{
    if ((q == m_Points[0] && p == m_Points[1])
            || (q == m_Points[1] && p == m_Points[0])) {
        m_ConstrainedEdge[2] = true;
    } else if ((q == m_Points[0] && p == m_Points[2])
            || (q == m_Points[2] && p == m_Points[0])) {
        m_ConstrainedEdge[1] = true;
    } else if ((q == m_Points[1] && p == m_Points[2])
            || (q == m_Points[2] && p == m_Points[1])) {
        m_ConstrainedEdge[0] = true;
    }
}

// The point counter-clockwise to given point
Point* TriangulationTriangle::pointCW(Point& point)
{
    if (&point == m_Points[0]) {
        return m_Points[2];
    } else if (&point == m_Points[1]) {
        return m_Points[0];
    } else if (&point == m_Points[2]) {
        return m_Points[1];
    }
    assert(0);
    return 0;   // Silence compiler warning
}

Point* TriangulationTriangle::pointCCW(Point& point)
{
    if (&point == m_Points[0]) {
        return m_Points[1];
    } else if (&point == m_Points[1]) {
        return m_Points[2];
    } else if (&point == m_Points[2]) {
        return m_Points[0];
    }
    assert(0);
    return 0;   // Silence compiler warning
}

TriangulationTriangle* TriangulationTriangle::neighborCW(Point& point)
{
    if (&point == m_Points[0]) {
        return m_Neighbors[1];
    } else if (&point == m_Points[1]) {
        return m_Neighbors[2];
    }
    return m_Neighbors[0];
}

TriangulationTriangle* TriangulationTriangle::neighborCCW(Point& point)
{
    if (&point == m_Points[0]) {
        return m_Neighbors[2];
    } else if (&point == m_Points[1]) {
        return m_Neighbors[0];
    }
    return m_Neighbors[1];
}

bool TriangulationTriangle::getConstrainedEdgeCCW(Point& p)
{
    if (&p == m_Points[0]) {
        return m_ConstrainedEdge[2];
    } else if (&p == m_Points[1]) {
        return m_ConstrainedEdge[0];
    }
    return m_ConstrainedEdge[1];
}

bool TriangulationTriangle::getConstrainedEdgeCW(Point& p)
{
    if (&p == m_Points[0]) {
        return m_ConstrainedEdge[1];
    } else if (&p == m_Points[1]) {
        return m_ConstrainedEdge[2];
    }
    return m_ConstrainedEdge[0];
}

void TriangulationTriangle::setConstrainedEdgeCCW(Point& p, bool ce)
{
    if (&p == m_Points[0]) {
        m_ConstrainedEdge[2] = ce;
    } else if (&p == m_Points[1]) {
        m_ConstrainedEdge[0] = ce;
    } else {
        m_ConstrainedEdge[1] = ce;
    }
}

void TriangulationTriangle::setConstrainedEdgeCW(Point& p, bool ce)
{
    if (&p == m_Points[0]) {
        m_ConstrainedEdge[1] = ce;
    } else if (&p == m_Points[1]) {
        m_ConstrainedEdge[2] = ce;
    } else {
        m_ConstrainedEdge[0] = ce;
    }
}

bool TriangulationTriangle::getDelunayEdgeCCW(Point& p)
{
    if (&p == m_Points[0]) {
        return m_DelaunayEdge[2];
    } else if (&p == m_Points[1]) {
        return m_DelaunayEdge[0];
    }
    return m_DelaunayEdge[1];
}

bool TriangulationTriangle::getDelunayEdgeCW(Point& p)
{
    if (&p == m_Points[0]) {
        return m_DelaunayEdge[1];
    } else if (&p == m_Points[1]) {
        return m_DelaunayEdge[2];
    }
    return m_DelaunayEdge[0];
}

void TriangulationTriangle::setDelunayEdgeCCW(Point& p, bool e)
{
    if (&p == m_Points[0]) {
        m_DelaunayEdge[2] = e;
    } else if (&p == m_Points[1]) {
        m_DelaunayEdge[0] = e;
    } else {
        m_DelaunayEdge[1] = e;
    }
}

void TriangulationTriangle::setDelunayEdgeCW(Point& p, bool e)
{
    if (&p == m_Points[0]) {
        m_DelaunayEdge[1] = e;
    } else if (&p == m_Points[1]) {
        m_DelaunayEdge[2] = e;
    } else {
        m_DelaunayEdge[0] = e;
    }
}

TriangulationTriangle& TriangulationTriangle::neighborAcross(Point& opoint)
{
    if (&opoint == m_Points[0]) {
        return *m_Neighbors[0];
    } else if (&opoint == m_Points[1]) {
        return *m_Neighbors[1];
    }
    return *m_Neighbors[2];
}

}

