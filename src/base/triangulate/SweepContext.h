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

#ifndef SWEEP_CONTEXT_H
#define SWEEP_CONTEXT_H

#include <list>
#include <vector>
#include <cstddef>

namespace avg {

// Inital triangle factor, seed triangle will extend 30% of
// PointSet width to both left and right.
const double kAlpha = 0.3;

struct Point;
class TriangulationTriangle;
struct Node;
struct Edge;
class AdvancingFront;

class SweepContext
{

public:

    SweepContext(std::vector<Point*> polyline);

    ~SweepContext();

    void setHead(Point* p1);

    Point* head();

    void setTail(Point* p1);

    Point* tail();

    int pointCount();

    Node& locateNode(Point& point);

    void removeNode(Node* node);

    void createAdvancingFront(std::vector<Node*> nodes);

/// Try to map a node to all sides of this triangle that don't have a neighbor
    void mapTriangleToNodes(TriangulationTriangle& t);

    void addToMap(TriangulationTriangle* triangle);

    Point* getPoint(const int& index);

    Point* GetPoints();

    void removeFromMap(TriangulationTriangle* triangle);

    void addHole(std::vector<Point*> polyline);

    void addPoint(Point* point);

    AdvancingFront* front();

    void meshClean(TriangulationTriangle& triangle);

    std::vector<TriangulationTriangle*>& getTriangles();

    std::vector<Edge*> m_EdgeList;

    struct Basin
    {
        Node* m_LeftNode;
        Node* m_BottomNode;
        Node* m_RightNode;
        double m_Width;
        bool m_LeftHighest;

        Basin()
        {
            clear();
        }

        void clear() {
            m_LeftNode = NULL;
            m_BottomNode = NULL;
            m_RightNode = NULL;
            m_Width = 0.0;
            m_LeftHighest = false;
        }
    };

    struct EdgeEvent
    {
        Edge* m_ConstrainedEdge;
        bool m_Right;

        EdgeEvent() :
                m_ConstrainedEdge(NULL), m_Right(false) {
        }
    };

    Basin m_Basin;
    EdgeEvent m_EdgeEvent;

private:

    friend class Sweep;

    std::vector<TriangulationTriangle*> m_Triangles;
    std::list<TriangulationTriangle*> m_Map;
    std::vector<Point*> m_Points;

    AdvancingFront* m_Front;
    Point* m_Head;
    Point* m_Tail;

    Node *m_AfHead, *m_AfMiddle, *m_AfTail;

    void initTriangulation();
    void initEdges(std::vector<Point*> polyline);

};

inline AdvancingFront* SweepContext::front()
{
    return m_Front;
}

inline int SweepContext::pointCount()
{
    return m_Points.size();
}

inline void SweepContext::setHead(Point* p1)
{
    m_Head = p1;
}

inline Point* SweepContext::head()
{
    return m_Head;
}

inline void SweepContext::setTail(Point* p1)
{
    m_Tail = p1;
}

inline Point* SweepContext::tail()
{
    return m_Tail;
}

}

#endif
