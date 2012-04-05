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

#include "SweepContext.h"
#include <algorithm>
#include "AdvancingFront.h"

namespace avg {

SweepContext::SweepContext(std::vector<Point*> polyline)
{
    m_Basin = Basin();
    m_EdgeEvent = EdgeEvent();

    m_Points = polyline;

    initEdges(m_Points);
}

void SweepContext::addHole(std::vector<Point*> polyline)
{
    initEdges(polyline);
    for (unsigned int i = 0; i < polyline.size(); i++) {
        m_Points.push_back(polyline[i]);
    }
}

void SweepContext::addPoint(Point* point)
{
    m_Points.push_back(point);
}

std::vector<TriangulationTriangle*>& SweepContext::getTriangles()
{
    return m_Triangles;
}

void SweepContext::initTriangulation()
{
    double xmax(m_Points[0]->m_X), xmin(m_Points[0]->m_X);
    double ymax(m_Points[0]->m_Y), ymin(m_Points[0]->m_Y);

    // Calculate bounds.
    for (unsigned int i = 0; i < m_Points.size(); i++) {
        Point& p = *m_Points[i];
        if (p.m_X > xmax)
            xmax = p.m_X;
        if (p.m_X < xmin)
            xmin = p.m_X;
        if (p.m_Y > ymax)
            ymax = p.m_Y;
        if (p.m_Y < ymin)
            ymin = p.m_Y;
    }

    double dx = kAlpha * (xmax - xmin);
    double dy = kAlpha * (ymax - ymin);
    m_Head = new Point(xmax + dx, ymin - dy, 0);
    m_Tail = new Point(xmin - dx, ymin - dy, 0);

    // Sort along y-axis
    std::sort(m_Points.begin(), m_Points.end(), cmp);

}

void SweepContext::initEdges(std::vector<Point*> polyline)
{
    int numPoints = polyline.size();
    for (int i = 0; i < numPoints; i++) {
        int j = i < numPoints - 1 ? i + 1 : 0;

        m_EdgeList.push_back(new Edge(*polyline[i], *polyline[j]));
    }
}

Point* SweepContext::getPoint(const int& index)
{
    return m_Points[index];
}

void SweepContext::addToMap(TriangulationTriangle* triangle)
{
    m_Map.push_back(triangle);
}

Node& SweepContext::locateNode(Point& point)
{
    // TO DO implement search tree
    return *m_Front->locateNode(point.m_X);
}

void SweepContext::createAdvancingFront(std::vector<Node*> nodes)
{
    // Initial triangle
    TriangulationTriangle* triangle = new TriangulationTriangle(*m_Points[0], *m_Tail,
            *m_Head);

    m_Map.push_back(triangle);

    m_AfHead = new Node(*triangle->getPoint(1), *triangle);
    m_AfMiddle = new Node(*triangle->getPoint(0), *triangle);
    m_AfTail = new Node(*triangle->getPoint(2));
    m_Front = new AdvancingFront(*m_AfHead, *m_AfTail);

    m_AfHead->m_Next = m_AfMiddle;
    m_AfMiddle->m_Next = m_AfTail;
    m_AfMiddle->m_Prev = m_AfHead;
    m_AfTail->m_Prev = m_AfMiddle;
}

void SweepContext::removeNode(Node* node)
{
    delete node;
}

void SweepContext::mapTriangleToNodes(TriangulationTriangle& t)
{
    for (int i = 0; i < 3; i++) {
        if (!t.getNeighbor(i)) {
            Node* n = m_Front->locatePoint(t.pointCW(*t.getPoint(i)));
            if (n) {
                n->m_Triangle = &t;
            }
        }
    }
}

void SweepContext::removeFromMap(TriangulationTriangle* triangle)
{
    m_Map.remove(triangle);
}

void SweepContext::meshClean(TriangulationTriangle& triangle)
{
    if (&triangle != NULL && !triangle.isInterior()) {
        triangle.isInterior(true);
        m_Triangles.push_back(&triangle);
        for (int i = 0; i < 3; i++) {
            if (!triangle.m_ConstrainedEdge[i])
                meshClean(*triangle.getNeighbor(i));
        }
    }
}

SweepContext::~SweepContext()
{

    delete m_Head;
    delete m_Tail;
    delete m_Front;
    delete m_AfHead;
    delete m_AfMiddle;
    delete m_AfTail;

    typedef std::list<TriangulationTriangle*> type_list;

    for (type_list::iterator iter = m_Map.begin(); iter != m_Map.end(); ++iter) {
        TriangulationTriangle* ptr = *iter;
        delete ptr;
    }

    for (unsigned int i = 0; i < m_EdgeList.size(); i++) {
        delete m_EdgeList[i];
    }

}

}
