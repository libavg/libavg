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

#include <stdexcept>
#include "Sweep.h"
#include "SweepContext.h"
#include "AdvancingFront.h"
#include "Utils.h"

namespace avg {

void Sweep::Triangulate(SweepContext& sc)
{
    sc.initTriangulation();
    sc.createAdvancingFront(m_Nodes);
    // Sweep points; build mesh
    sweepPoints(sc);
    // Clean up
    finalizationPolygon(sc);
}

void Sweep::sweepPoints(SweepContext& sc)
{
    for (int i = 1; i < sc.pointCount(); i++) {
        Point& point = *sc.getPoint(i);
        Node* node = &pointEvent(sc, point);
        for (unsigned int i = 0; i < point.m_EdgeList.size(); i++) {
            edgeEvent(sc, point.m_EdgeList[i], node);
        }
    }
}

void Sweep::finalizationPolygon(SweepContext& sc)
{
    // Get an Internal triangle to start with
    TriangulationTriangle* t = sc.front()->head()->m_Next->m_Triangle;
    Point* p = sc.front()->head()->m_Next->m_Point;
    while (!t->getConstrainedEdgeCW(*p)) {
        t = t->neighborCCW(*p);
    }

    // Collect interior triangles constrained by edges
    sc.meshClean(*t);
}

Node& Sweep::pointEvent(SweepContext& sc, Point& point)
{
    Node& node = sc.locateNode(point);
    Node& new_node = newFrontTriangle(sc, point, node);

    // Only need to check +epsilon since point never have smaller
    // x value than node due to how we fetch nodes from the front
    if (point.m_X <= node.m_Point->m_X + EPSILON) {
        fill(sc, node);
    }

    //tcx.AddNode(new_node);

    fillAdvancingFront(sc, new_node);
    return new_node;
}

void Sweep::edgeEvent(SweepContext& sc, Edge* edge, Node* node)
{
    sc.m_EdgeEvent.m_ConstrainedEdge = edge;
    sc.m_EdgeEvent.m_Right = (edge->m_P->m_X > edge->m_Q->m_X);

    if (isEdgeSideOfTriangle(*node->m_Triangle, *edge->m_P, *edge->m_Q)) {
        return;
    }

    // to do: integrate with flip process might give some better performance
    //       but for now this avoid the issue with cases that needs both flips and fills
    fillEdgeEvent(sc, edge, node);
    edgeEvent(sc, *edge->m_P, *edge->m_Q, node->m_Triangle, *edge->m_Q);
}

void Sweep::edgeEvent(SweepContext& sc, Point& ep, Point& eq,
        TriangulationTriangle* triangle, Point& point)
{
    if (isEdgeSideOfTriangle(*triangle, ep, eq)) {
        return;
    }

    Point* p1 = triangle->pointCCW(point);
    Orientation o1 = orient2d(eq, *p1, ep);
    if (o1 == COLLINEAR) {
        if (triangle->contains(&eq, p1)) {
            triangle->markConstrainedEdge(&eq, p1);
            // We are modifying the constraint maybe it would be better to
            // not change the given constraint and just keep a variable for the new constraint
            sc.m_EdgeEvent.m_ConstrainedEdge->m_Q = p1;
            triangle = &triangle->neighborAcross(point);
            edgeEvent(sc, ep, *p1, triangle, *p1);
        } else {
            std::runtime_error("EdgeEvent - collinear points not supported");
            assert(0);
        }
        return;
    }

    Point* p2 = triangle->pointCW(point);
    Orientation o2 = orient2d(eq, *p2, ep);
    if (o2 == COLLINEAR) {
        if (triangle->contains(&eq, p2)) {
            triangle->markConstrainedEdge(&eq, p2);
            // We are modifying the constraint maybe it would be better to
            // not change the given constraint and just keep a variable for the new constraint
            sc.m_EdgeEvent.m_ConstrainedEdge->m_Q = p2;
            triangle = &triangle->neighborAcross(point);
            edgeEvent(sc, ep, *p2, triangle, *p2);
        } else {
            std::runtime_error("EdgeEvent - collinear points not supported");
            assert(0);
        }
        return;
    }

    if (o1 == o2) {
        // Need to decide if we are rotating CW or CCW to get to a triangle
        // that will cross edge
        if (o1 == CW) {
            triangle = triangle->neighborCCW(point);
        } else {
            triangle = triangle->neighborCW(point);
        }
        edgeEvent(sc, ep, eq, triangle, point);
    } else {
        // This triangle crosses constraint so lets flippin start!
        flipEdgeEvent(sc, ep, eq, triangle, point);
    }
}

bool Sweep::isEdgeSideOfTriangle(TriangulationTriangle& triangle, Point& ep,
        Point& eq)
{
    int index = triangle.edgeIndex(&ep, &eq);

    if (index != -1) {
        triangle.markConstrainedEdge(index);
        TriangulationTriangle* t = triangle.getNeighbor(index);
        if (t) {
            t->markConstrainedEdge(&ep, &eq);
        }
        return true;
    }
    return false;
}

Node& Sweep::newFrontTriangle(SweepContext& sc, Point& point, Node& node)
{
    TriangulationTriangle* triangle = new TriangulationTriangle(point,
            *node.m_Point, *node.m_Next->m_Point);

    triangle->markNeighbor(*node.m_Triangle);
    sc.addToMap(triangle);

    Node* newNode = new Node(point);
    m_Nodes.push_back(newNode);

    newNode->m_Next = node.m_Next;
    newNode->m_Prev = &node;
    node.m_Next->m_Prev = newNode;
    node.m_Next = newNode;

    if (!legalize(sc, *triangle)) {
        sc.mapTriangleToNodes(*triangle);
    }

    return *newNode;
}

void Sweep::fill(SweepContext& sc, Node& node)
{
    TriangulationTriangle* triangle = new TriangulationTriangle(
            *node.m_Prev->m_Point, *node.m_Point, *node.m_Next->m_Point);

    // TO DO: should copy the constrained_edge value from neighbor triangles
    //       for now constrained_edge values are copied during the legalize
    triangle->markNeighbor(*node.m_Prev->m_Triangle);
    triangle->markNeighbor(*node.m_Triangle);

    sc.addToMap(triangle);

    // Update the advancing front
    node.m_Prev->m_Next = node.m_Next;
    node.m_Next->m_Prev = node.m_Prev;

    // If it was legalized the triangle has already been mapped
    if (!legalize(sc, *triangle)) {
        sc.mapTriangleToNodes(*triangle);
    }

}

void Sweep::fillAdvancingFront(SweepContext& sc, Node& n)
{
    Node* node = n.m_Next;

    while (node->m_Next) {
        double angle = holeAngle(*node);
        if (angle > M_PI_2 || angle < -M_PI_2)
            break;
// ---------- LEAK FIX --------------
//      Fill(tcx, *node);
//      node = node->m_next;


        Node *tmp = node;
        node = node->m_Next;
        fill(sc, *tmp);
// ----------------------------------
    }

    node = n.m_Prev;

    while (node->m_Prev) {
        double angle = holeAngle(*node);
        if (angle > M_PI_2 || angle < -M_PI_2)
            break;
        fill(sc, *node);
        node = node->m_Prev;
    }

    if (n.m_Next && n.m_Next->m_Next) {
        double angle = basinAngle(n);
        if (angle < PI_3div4) {
            fillBasin(sc, n);
        }
    }
}

double Sweep::basinAngle(Node& node)
{
    double ax = node.m_Point->m_X - node.m_Next->m_Next->m_Point->m_X;
    double ay = node.m_Point->m_Y - node.m_Next->m_Next->m_Point->m_Y;
    return atan2(ay, ax);
}

double Sweep::holeAngle(Node& node)
{
    /* Complex plane
     * ab = cosA +i*sinA
     * ab = (ax + ay*i)(bx + by*i) = (ax*bx + ay*by) + i(ax*by-ay*bx)
     * atan2(y,x) computes the principal value of the argument function
     * applied to the complex number x+iy
     * Where x = ax*bx + ay*by
     *       y = ax*by - ay*bx
     */
    double ax = node.m_Next->m_Point->m_X - node.m_Point->m_X;
    double ay = node.m_Next->m_Point->m_Y - node.m_Point->m_Y;
    double bx = node.m_Prev->m_Point->m_X - node.m_Point->m_X;
    double by = node.m_Prev->m_Point->m_Y - node.m_Point->m_Y;
    return atan2(ax * by - ay * bx, ax * bx + ay * by);
}

bool Sweep::legalize(SweepContext& sc, TriangulationTriangle& t)
{
    // To legalize a triangle we start by finding if any of the three edges
    // violate the Delaunay condition
    for (int i = 0; i < 3; i++) {
        if (t.m_DelaunayEdge[i])
            continue;

        TriangulationTriangle* ot = t.getNeighbor(i);

        if (ot) {
            Point* p = t.getPoint(i);
            Point* op = ot->oppositePoint(t, *p);
            int oi = ot->index(op);

            // If this is a Constrained Edge or a Delaunay Edge(only during recursive legalization)
            // then we should not try to legalize
            if (ot->m_ConstrainedEdge[oi] || ot->m_DelaunayEdge[oi]) {
                t.m_ConstrainedEdge[i] = ot->m_ConstrainedEdge[oi];
                continue;
            }

            bool inside = incircle(*p, *t.pointCCW(*p), *t.pointCW(*p), *op);

            if (inside) {
                // Lets mark this shared edge as Delaunay
                t.m_DelaunayEdge[i] = true;
                ot->m_DelaunayEdge[oi] = true;

                // Lets rotate shared edge one vertex CW to legalize it
                rotateTrianglePair(t, *p, *ot, *op);

                // We now got one valid Delaunay Edge shared by two triangles
                // This gives us 4 new edges to check for Delaunay

                // Make sure that triangle to node mapping is done only one time for a specific triangle
                bool notLegalized = !legalize(sc, t);
                if (notLegalized) {
                    sc.mapTriangleToNodes(t);
                }

                notLegalized = !legalize(sc, *ot);
                if (notLegalized)
                    sc.mapTriangleToNodes(*ot);

                // Reset the Delaunay edges, since they only are valid Delaunay edges
                // until we add a new triangle or point.
                // XX X: need to think about this. Can these edges be tried after we
                //      return to previous recursive level?
                t.m_DelaunayEdge[i] = false;
                ot->m_DelaunayEdge[oi] = false;

                // If triangle have been legalized no need to check the other edges since
                // the recursive legalization will handles those so we can end here.
                return true;
            }
        }
    }
    return false;
}

bool Sweep::incircle(Point& pa, Point& pb, Point& pc, Point& pd)
{
    double adx = pa.m_X - pd.m_X;
    double ady = pa.m_Y - pd.m_Y;
    double bdx = pb.m_X - pd.m_X;
    double bdy = pb.m_Y - pd.m_Y;

    double adxbdy = adx * bdy;
    double bdxady = bdx * ady;
    double oabd = adxbdy - bdxady;

    if (oabd <= 0)
        return false;

    double cdx = pc.m_X - pd.m_X;
    double cdy = pc.m_Y - pd.m_Y;

    double cdxady = cdx * ady;
    double adxcdy = adx * cdy;
    double ocad = cdxady - adxcdy;

    if (ocad <= 0)
        return false;

    double bdxcdy = bdx * cdy;
    double cdxbdy = cdx * bdy;

    double alift = adx * adx + ady * ady;
    double blift = bdx * bdx + bdy * bdy;
    double clift = cdx * cdx + cdy * cdy;

    double det = alift * (bdxcdy - cdxbdy) + blift * ocad + clift * oabd;

    return det > 0;
}

void Sweep::rotateTrianglePair(TriangulationTriangle& t, Point& p,
        TriangulationTriangle& ot, Point& op)
{
    TriangulationTriangle* n1, *n2, *n3, *n4;
    n1 = t.neighborCCW(p);
    n2 = t.neighborCW(p);
    n3 = ot.neighborCCW(op);
    n4 = ot.neighborCW(op);

    bool ce1, ce2, ce3, ce4;
    ce1 = t.getConstrainedEdgeCCW(p);
    ce2 = t.getConstrainedEdgeCW(p);
    ce3 = ot.getConstrainedEdgeCCW(op);
    ce4 = ot.getConstrainedEdgeCW(op);

    bool de1, de2, de3, de4;
    de1 = t.getDelunayEdgeCCW(p);
    de2 = t.getDelunayEdgeCW(p);
    de3 = ot.getDelunayEdgeCCW(op);
    de4 = ot.getDelunayEdgeCW(op);

    t.legalize(p, op);
    ot.legalize(op, p);

    // Remap delaunay_edge
    ot.setDelunayEdgeCCW(p, de1);
    t.setDelunayEdgeCW(p, de2);
    t.setDelunayEdgeCCW(op, de3);
    ot.setDelunayEdgeCW(op, de4);

    // Remap constrained_edge
    ot.setConstrainedEdgeCCW(p, ce1);
    t.setConstrainedEdgeCW(p, ce2);
    t.setConstrainedEdgeCCW(op, ce3);
    ot.setConstrainedEdgeCW(op, ce4);

    // Remap neighbors
    // XX X: might optimize the markNeighbor by keeping track of
    //      what side should be assigned to what neighbor after the
    //      rotation. Now mark neighbor does lots of testing to find
    //      the right side.
    t.clearNeighbors();
    ot.clearNeighbors();
    if (n1)
        ot.markNeighbor(*n1);
    if (n2)
        t.markNeighbor(*n2);
    if (n3)
        t.markNeighbor(*n3);
    if (n4)
        ot.markNeighbor(*n4);
    t.markNeighbor(ot);
}

void Sweep::fillBasin(SweepContext& sc, Node& node)
{
    if (orient2d(*node.m_Point, *node.m_Next->m_Point, *node.m_Next->m_Next->m_Point)
            == CCW) {
        sc.m_Basin.m_LeftNode = node.m_Next->m_Next;
    } else {
        sc.m_Basin.m_LeftNode = node.m_Next;
    }

    // Find the bottom and right node
    sc.m_Basin.m_BottomNode = sc.m_Basin.m_LeftNode;
    while (sc.m_Basin.m_BottomNode->m_Next
            && sc.m_Basin.m_BottomNode->m_Point->m_Y
                    >= sc.m_Basin.m_BottomNode->m_Next->m_Point->m_Y) {
        sc.m_Basin.m_BottomNode = sc.m_Basin.m_BottomNode->m_Next;
    }
    if (sc.m_Basin.m_BottomNode == sc.m_Basin.m_LeftNode) {
        // No valid basin
        return;
    }

    sc.m_Basin.m_RightNode = sc.m_Basin.m_BottomNode;
    while (sc.m_Basin.m_RightNode->m_Next
            && sc.m_Basin.m_RightNode->m_Point->m_Y
                    < sc.m_Basin.m_RightNode->m_Next->m_Point->m_Y) {
        sc.m_Basin.m_RightNode = sc.m_Basin.m_RightNode->m_Next;
    }
    if (sc.m_Basin.m_RightNode == sc.m_Basin.m_BottomNode) {
        // No valid basins
        return;
    }

    sc.m_Basin.m_Width = sc.m_Basin.m_RightNode->m_Point->m_X
            - sc.m_Basin.m_LeftNode->m_Point->m_X;
    sc.m_Basin.m_LeftHighest = sc.m_Basin.m_LeftNode->m_Point->m_Y
            > sc.m_Basin.m_RightNode->m_Point->m_Y;

    fillBasinReq(sc, sc.m_Basin.m_BottomNode);
}

void Sweep::fillBasinReq(SweepContext& sc, Node* node)
{
    // if shallow stop filling
    if (isShallow(sc, *node)) {
        return;
    }

    fill(sc, *node);

    if (node->m_Prev == sc.m_Basin.m_LeftNode
            && node->m_Next == sc.m_Basin.m_RightNode) {
        return;
    } else if (node->m_Prev == sc.m_Basin.m_LeftNode) {
        Orientation o = orient2d(*node->m_Point, *node->m_Next->m_Point,
                *node->m_Next->m_Next->m_Point);
        if (o == CW) {
            return;
        }
        node = node->m_Next;
    } else if (node->m_Next == sc.m_Basin.m_RightNode) {
        Orientation o = orient2d(*node->m_Point, *node->m_Prev->m_Point,
                *node->m_Prev->m_Prev->m_Point);
        if (o == CCW) {
            return;
        }
        node = node->m_Prev;
    } else {
        // Continue with the neighbor node with lowest Y value
        if (node->m_Prev->m_Point->m_Y < node->m_Next->m_Point->m_Y) {
            node = node->m_Prev;
        } else {
            node = node->m_Next;
        }
    }

    fillBasinReq(sc, node);
}

bool Sweep::isShallow(SweepContext& sc, Node& node)
{
    double height;

    if (sc.m_Basin.m_LeftHighest) {
        height = sc.m_Basin.m_LeftNode->m_Point->m_Y - node.m_Point->m_Y;
    } else {
        height = sc.m_Basin.m_RightNode->m_Point->m_Y - node.m_Point->m_Y;
    }

    // if shallow stop filling
    if (sc.m_Basin.m_Width > height) {
        return true;
    }
    return false;
}

void Sweep::fillEdgeEvent(SweepContext& sc, Edge* edge, Node* node)
{
    if (sc.m_EdgeEvent.m_Right) {
        fillRightAboveEdgeEvent(sc, edge, node);
    } else {
        fillLeftAboveEdgeEvent(sc, edge, node);
    }
}

void Sweep::fillRightAboveEdgeEvent(SweepContext& sc, Edge* edge, Node* node)
{
    while (node->m_Next->m_Point->m_X < edge->m_P->m_X) {
        // Check if next node is below the edge
        if (orient2d(*edge->m_Q, *node->m_Next->m_Point, *edge->m_P) == CCW) {
            fillRightBelowEdgeEvent(sc, edge, *node);
        } else {
            node = node->m_Next;
        }
    }
}

void Sweep::fillRightBelowEdgeEvent(SweepContext& sc, Edge* edge, Node& node)
{
    if (node.m_Point->m_X < edge->m_P->m_X) {
        if (orient2d(*node.m_Point, *node.m_Next->m_Point, *node.m_Next->m_Next->m_Point)
                == CCW) {
            // Concave
            fillRightConcaveEdgeEvent(sc, edge, node);
        } else {
            // Convex
            fillRightConvexEdgeEvent(sc, edge, node);
            // Retry this one
            fillRightBelowEdgeEvent(sc, edge, node);
        }
    }
}

void Sweep::fillRightConcaveEdgeEvent(SweepContext& sc, Edge* edge, Node& node)
{
    fill(sc, *node.m_Next);
    if (node.m_Next->m_Point != edge->m_P) {
        // Next above or below edge?
        if (orient2d(*edge->m_Q, *node.m_Next->m_Point, *edge->m_P) == CCW) {
            // Below
            if (orient2d(*node.m_Point, *node.m_Next->m_Point,
                    *node.m_Next->m_Next->m_Point) == CCW) {
                // Next is concave
                fillRightConcaveEdgeEvent(sc, edge, node);
            } else {
                // Next is convex
            }
        }
    }

}

void Sweep::fillRightConvexEdgeEvent(SweepContext& sc, Edge* edge, Node& node)
{
    // Next concave or convex?
    if (orient2d(*node.m_Next->m_Point, *node.m_Next->m_Next->m_Point,
            *node.m_Next->m_Next->m_Next->m_Point) == CCW) {
        // Concave
        fillRightConcaveEdgeEvent(sc, edge, *node.m_Next);
    } else {
        // Convex
        // Next above or below edge?
        if (orient2d(*edge->m_Q, *node.m_Next->m_Next->m_Point, *edge->m_P) == CCW) {
            // Below
            fillRightConvexEdgeEvent(sc, edge, *node.m_Next);
        } else {
            // Above
        }
    }
}

void Sweep::fillLeftAboveEdgeEvent(SweepContext& sc, Edge* edge, Node* node)
{
    while (node->m_Prev->m_Point->m_X > edge->m_P->m_X) {
        // Check if next node is below the edge
        if (orient2d(*edge->m_Q, *node->m_Prev->m_Point, *edge->m_P) == CW) {
            fillLeftBelowEdgeEvent(sc, edge, *node);
        } else {
            node = node->m_Prev;
        }
    }
}

void Sweep::fillLeftBelowEdgeEvent(SweepContext& sc, Edge* edge, Node& node)
{
    if (node.m_Point->m_X > edge->m_P->m_X) {
        if (orient2d(*node.m_Point, *node.m_Prev->m_Point, *node.m_Prev->m_Prev->m_Point)
                == CW) {
            // Concave
            fillLeftConcaveEdgeEvent(sc, edge, node);
        } else {
            // Convex
            fillLeftConvexEdgeEvent(sc, edge, node);
            // Retry this one
            fillLeftBelowEdgeEvent(sc, edge, node);
        }
    }
}

void Sweep::fillLeftConvexEdgeEvent(SweepContext& sc, Edge* edge, Node& node)
{
    // Next concave or convex?
    if (orient2d(*node.m_Prev->m_Point, *node.m_Prev->m_Prev->m_Point,
            *node.m_Prev->m_Prev->m_Prev->m_Point) == CW) {
        // Concave
        fillLeftConcaveEdgeEvent(sc, edge, *node.m_Prev);
    } else {
        // Convex
        // Next above or below edge?
        if (orient2d(*edge->m_Q, *node.m_Prev->m_Prev->m_Point, *edge->m_P) == CW) {
            // Below
            fillLeftConvexEdgeEvent(sc, edge, *node.m_Prev);
        } else {
            // Above
        }
    }
}

void Sweep::fillLeftConcaveEdgeEvent(SweepContext& sc, Edge* edge, Node& node)
{
    fill(sc, *node.m_Prev);
    if (node.m_Prev->m_Point != edge->m_P) {
        // Next above or below edge?
        if (orient2d(*edge->m_Q, *node.m_Prev->m_Point, *edge->m_P) == CW) {
            // Below
            if (orient2d(*node.m_Point, *node.m_Prev->m_Point,
                    *node.m_Prev->m_Prev->m_Point) == CW) {
                // Next is concave
                fillLeftConcaveEdgeEvent(sc, edge, node);
            } else {
                // Next is convex
            }
        }
    }

}

void Sweep::flipEdgeEvent(SweepContext& sc, Point& ep, Point& eq,
        TriangulationTriangle* t, Point& p)
{
    TriangulationTriangle& ot = t->neighborAcross(p);
    Point& op = *ot.oppositePoint(*t, p);

    if (&ot == NULL) {
        // If we want to integrate the fillEdgeEvent do it here
        // With current implementation we should never get here
        //throw new RuntimeException( "[BUG:FIXM E] FLIP failed due to missing triangle");
        assert(0);
    }

    if (inScanArea(p, *t->pointCCW(p), *t->pointCW(p), op)) {
        // Lets rotate shared edge one vertex CW
        rotateTrianglePair(*t, p, ot, op);
        sc.mapTriangleToNodes(*t);
        sc.mapTriangleToNodes(ot);

        if (p == eq && op == ep) {
            if (eq == *sc.m_EdgeEvent.m_ConstrainedEdge->m_Q
                    && ep == *sc.m_EdgeEvent.m_ConstrainedEdge->m_P) {
                t->markConstrainedEdge(&ep, &eq);
                ot.markConstrainedEdge(&ep, &eq);
                legalize(sc, *t);
                legalize(sc, ot);
            } else {
                // One of the triangles should be legalized here?
            }
        } else {
            Orientation o = orient2d(eq, op, ep);
            t = &nextFlipTriangle(sc, (int) o, *t, ot, p, op);
            flipEdgeEvent(sc, ep, eq, t, p);
        }
    } else {
        Point& newP = nextFlipPoint(ep, eq, ot, op);
        flipScanEdgeEvent(sc, ep, eq, *t, ot, newP);
        edgeEvent(sc, ep, eq, t, p);
    }
}

TriangulationTriangle& Sweep::nextFlipTriangle(SweepContext& sc, int o,
        TriangulationTriangle& t, TriangulationTriangle& ot, Point& p, Point& op)
{
    if (o == CCW) {
        // ot is not crossing edge after flip
        int edgeIndex = ot.edgeIndex(&p, &op);
        ot.m_DelaunayEdge[edgeIndex] = true;
        legalize(sc, ot);
        ot.clearDelunayEdges();
        return t;
    }

    // t is not crossing edge after flip
    int edgeIndex = t.edgeIndex(&p, &op);

    t.m_DelaunayEdge[edgeIndex] = true;
    legalize(sc, t);
    t.clearDelunayEdges();
    return ot;
}

Point& Sweep::nextFlipPoint(Point& ep, Point& eq, TriangulationTriangle& ot, Point& op)
{
    Orientation o2d = orient2d(eq, op, ep);
    if (o2d == CW) {
        // Right
        return *ot.pointCCW(op);
    } else if (o2d == CCW) {
        // Left
        return *ot.pointCW(op);
    } else {
        //throw new RuntimeException("[Unsupported] Opposing point on constrained edge");
        assert(0);
        return ep; // Silence compiler warning.
    }
}

void Sweep::flipScanEdgeEvent(SweepContext& sc, Point& ep, Point& eq,
        TriangulationTriangle& flipTriangle, TriangulationTriangle& t, Point& p)
{
    TriangulationTriangle& ot = t.neighborAcross(p);
    Point& op = *ot.oppositePoint(t, p);

    if (&t.neighborAcross(p) == NULL) {
        // If we want to integrate the fillEdgeEvent do it here
        // With current implementation we should never get here
        //throw new RuntimeException( "[BUG:FIXM E] FLIP failed due to missing triangle");
        assert(0);
    }

    if (inScanArea(eq, *flipTriangle.pointCCW(eq), *flipTriangle.pointCW(eq),
            op)) {
        // flip with new edge op->eq
        flipEdgeEvent(sc, eq, op, &ot, op);
        // To do: Actually I just figured out that it should be possible to
        //       improve this by getting the next ot and op before the the above
        //       flip and continue the flipScanEdgeEvent here
        // set new ot and op here and loop back to inScanArea test
        // also need to set a new flip_triangle first
        // Turns out at first glance that this is somewhat complicated
        // so it will have to wait.
    } else {
        Point& newP = nextFlipPoint(ep, eq, ot, op);
        flipScanEdgeEvent(sc, ep, eq, flipTriangle, ot, newP);
    }
}

Sweep::~Sweep()
{
    for (unsigned int i = 0; i < m_Nodes.size(); i++) {
        delete m_Nodes[i];
    }

}

}
