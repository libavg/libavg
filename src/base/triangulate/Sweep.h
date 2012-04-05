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

#ifndef SWEEP_H
#define SWEEP_H

#include <vector>

namespace avg {

class SweepContext;
struct Node;
struct Point;
struct Edge;
class TriangulationTriangle;

class Sweep
{

public:

    void Triangulate(SweepContext& sc);

    ~Sweep();

private:

    void sweepPoints(SweepContext& sc);

    Node& pointEvent(SweepContext& sc, Point& point);

    void edgeEvent(SweepContext& sc, Edge* edge, Node* node);

    void edgeEvent(SweepContext& sc, Point& ep, Point& eq,
            TriangulationTriangle* triangle, Point& point);

    Node& newFrontTriangle(SweepContext& sc, Point& point, Node& node);

    void fill(SweepContext& sc, Node& node);

    bool legalize(SweepContext& sc, TriangulationTriangle& t);

    /**
     * <b>Requirement</b>:<br>
     * 1. a,b and c form a triangle.<br>
     * 2. a and d is know to be on opposite side of bc<br>
     * <pre>
     *                a
     *                +
     *               / \
     *              /   \
     *            b/     \c
     *            +-------+
     *           /    d    \
     *          /           \
     * </pre>
     * <b>Fact</b>: d has to be in area B to have a chance to be inside the circle formed by
     *  a,b and c<br>
     *  d is outside B if orient2d(a,b,d) or orient2d(c,a,d) is CW<br>
     *  This preknowledge gives us a way to optimize the incircle test
     * @param a - triangle point, opposite d
     * @param b - triangle point
     * @param c - triangle point
     * @param d - point opposite a
     * @return true if d is inside circle, false if on circle edge
     */
    bool incircle(Point& pa, Point& pb, Point& pc, Point& pd);

    /**
     * Rotates a triangle pair one vertex CW
     *<pre>
     *       n2                    n2
     *  P +-----+             P +-----+
     *    | t  /|               |\  t |
     *    |   / |               | \   |
     *  n1|  /  |n3           n1|  \  |n3
     *    | /   |    after CW   |   \ |
     *    |/ oT |               | oT \|
     *    +-----+ oP            +-----+
     *       n4                    n4
     * </pre>
     */
    void rotateTrianglePair(TriangulationTriangle& t, Point& p, TriangulationTriangle& ot,
            Point& op);

    void fillAdvancingFront(SweepContext& sc, Node& n);

    double holeAngle(Node& node);

    /**
     * The basin angle is decided against the horizontal line [1,0]
     */
    double basinAngle(Node& node);

    void fillBasin(SweepContext& sc, Node& node);

    void fillBasinReq(SweepContext& sc, Node* node);

    bool isShallow(SweepContext& sc, Node& node);

    bool isEdgeSideOfTriangle(TriangulationTriangle& triangle, Point& ep, Point& eq);

    void fillEdgeEvent(SweepContext& sc, Edge* edge, Node* node);

    void fillRightAboveEdgeEvent(SweepContext& sc, Edge* edge, Node* node);

    void fillRightBelowEdgeEvent(SweepContext& sc, Edge* edge, Node& node);

    void fillRightConcaveEdgeEvent(SweepContext& sc, Edge* edge, Node& node);

    void fillRightConvexEdgeEvent(SweepContext& sc, Edge* edge, Node& node);

    void fillLeftAboveEdgeEvent(SweepContext& sc, Edge* edge, Node* node);

    void fillLeftBelowEdgeEvent(SweepContext& sc, Edge* edge, Node& node);

    void fillLeftConcaveEdgeEvent(SweepContext& sc, Edge* edge, Node& node);

    void fillLeftConvexEdgeEvent(SweepContext& sc, Edge* edge, Node& node);

    void flipEdgeEvent(SweepContext& sc, Point& ep, Point& eq, TriangulationTriangle* t,
            Point& p);

    /**
     * After a flip we have two triangles and know that only one will still be
     * intersecting the edge. So decide which to contiune with and legalize the other
     *
     * @param sc
     * @param o - should be the result of an orient2d( eq, op, ep )
     * @param t - triangle 1
     * @param ot - triangle 2
     * @param p - a point shared by both triangles
     * @param op - another point shared by both triangles
     * @return returns the triangle still intersecting the edge
     */
    TriangulationTriangle& nextFlipTriangle(SweepContext& sc, int o,
            TriangulationTriangle& t, TriangulationTriangle& ot, Point& p, Point& op);

    /**
     * When we need to traverse from one triangle to the next we need
     * the point in current triangle that is the opposite point to the next
     * triangle.
     *
     * @param ep
     * @param eq
     * @param ot
     * @param op
     * @return
     */
    Point& nextFlipPoint(Point& ep, Point& eq, TriangulationTriangle& ot, Point& op);

    /**
     * Scan part of the FlipScan algorithm<br>
     * When a triangle pair isn't flippable we will scan for the next
     * point that is inside the flip triangle scan area. When found
     * we generate a new flipEdgeEvent
     *
     * @param sc
     * @param ep - last point on the edge we are traversing
     * @param eq - first point on the edge we are traversing
     * @param flipTriangle - the current triangle sharing the point eq with edge
     * @param t
     * @param p
     */
    void flipScanEdgeEvent(SweepContext& sc, Point& ep, Point& eq,
            TriangulationTriangle& flip_triangle, TriangulationTriangle& t, Point& p);

    void finalizationPolygon(SweepContext& sc);

    std::vector<Node*> m_Nodes;

};

}

#endif
