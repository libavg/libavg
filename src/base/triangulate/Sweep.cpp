/*
 * Poly2Tri Copyright (c) 2009-2010, Poly2Tri Contributors
 * http://code.google.com/p/poly2tri/
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * * Neither the name of Poly2Tri nor the names of its contributors may be
 *   used to endorse or promote products derived from this software without specific
 *   prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdexcept>
#include "Sweep.h"
#include "SweepContext.h"
#include "AdvancingFront.h"
#include "Utils.h"

namespace avg {

// Triangulate simple polygon with holes
void Sweep::Triangulate(SweepContext& tcx)
{
	tcx.InitTriangulation();
	tcx.CreateAdvancingFront(m_nodes);
	// Sweep points; build mesh
	SweepPoints(tcx);
	// Clean up
	FinalizationPolygon(tcx);
}

void Sweep::SweepPoints(SweepContext& tcx)
{
	for (int i = 1; i < tcx.pointCount(); i++) {
		Point& point = *tcx.GetPoint(i);
		Node* node = &PointEvent(tcx, point);
		for (unsigned int i = 0; i < point.edge_list.size(); i++) {
			EdgeEvent(tcx, point.edge_list[i], node);
		}
	}
}

void Sweep::FinalizationPolygon(SweepContext& tcx)
{
	// Get an Internal triangle to start with
	TriangulationTriangle* t = tcx.front()->head()->m_next->m_triangle;
	Point* p = tcx.front()->head()->m_next->m_point;
	while (!t->GetConstrainedEdgeCW(*p)) {
		t = t->NeighborCCW(*p);
	}

	// Collect interior triangles constrained by edges
	tcx.MeshClean(*t);
}

Node& Sweep::PointEvent(SweepContext& tcx, Point& point)
{
	Node& node = tcx.LocateNode(point);
	Node& new_node = NewFrontTriangle(tcx, point, node);

	// Only need to check +epsilon since point never have smaller
	// x value than node due to how we fetch nodes from the front
	if (point.m_x <= node.m_point->m_x + EPSILON) {
		Fill(tcx, node);
	}

	//tcx.AddNode(new_node);

	FillAdvancingFront(tcx, new_node);
	return new_node;
}

void Sweep::EdgeEvent(SweepContext& tcx, Edge* edge, Node* node)
{
	tcx.m_edgeEvent.m_constrainedEdge = edge;
	tcx.m_edgeEvent.m_right = (edge->m_p->m_x > edge->m_q->m_x);

	if (IsEdgeSideOfTriangle(*node->m_triangle, *edge->m_p, *edge->m_q)) {
		return;
	}

	// For now we will do all needed filling
	// to do: integrate with flip process might give some better performance
	//       but for now this avoid the issue with cases that needs both flips and fills
	FillEdgeEvent(tcx, edge, node);
	EdgeEvent(tcx, *edge->m_p, *edge->m_q, node->m_triangle, *edge->m_q);
}

void Sweep::EdgeEvent(SweepContext& tcx, Point& ep, Point& eq,
		TriangulationTriangle* triangle, Point& point)
{
	if (IsEdgeSideOfTriangle(*triangle, ep, eq)) {
		return;
	}

	Point* p1 = triangle->PointCCW(point);
	Orientation o1 = Orient2d(eq, *p1, ep);
	if (o1 == COLLINEAR) {
		if (triangle->Contains(&eq, p1)) {
			triangle->MarkConstrainedEdge(&eq, p1);
			// We are modifying the constraint maybe it would be better to
			// not change the given constraint and just keep a variable for the new constraint
			tcx.m_edgeEvent.m_constrainedEdge->m_q = p1;
			triangle = &triangle->NeighborAcross(point);
			EdgeEvent(tcx, ep, *p1, triangle, *p1);
		} else {
			std::runtime_error("EdgeEvent - collinear points not supported");
			assert(0);
		}
		return;
	}

	Point* p2 = triangle->PointCW(point);
	Orientation o2 = Orient2d(eq, *p2, ep);
	if (o2 == COLLINEAR) {
		if (triangle->Contains(&eq, p2)) {
			triangle->MarkConstrainedEdge(&eq, p2);
			// We are modifying the constraint maybe it would be better to
			// not change the given constraint and just keep a variable for the new constraint
			tcx.m_edgeEvent.m_constrainedEdge->m_q = p2;
			triangle = &triangle->NeighborAcross(point);
			EdgeEvent(tcx, ep, *p2, triangle, *p2);
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
			triangle = triangle->NeighborCCW(point);
		} else {
			triangle = triangle->NeighborCW(point);
		}
		EdgeEvent(tcx, ep, eq, triangle, point);
	} else {
		// This triangle crosses constraint so lets flippin start!
		FlipEdgeEvent(tcx, ep, eq, triangle, point);
	}
}

bool Sweep::IsEdgeSideOfTriangle(TriangulationTriangle& triangle, Point& ep,
		Point& eq)
{
	int index = triangle.EdgeIndex(&ep, &eq);

	if (index != -1) {
		triangle.MarkConstrainedEdge(index);
		TriangulationTriangle* t = triangle.GetNeighbor(index);
		if (t) {
			t->MarkConstrainedEdge(&ep, &eq);
		}
		return true;
	}
	return false;
}

Node& Sweep::NewFrontTriangle(SweepContext& tcx, Point& point, Node& node)
{
	TriangulationTriangle* triangle = new TriangulationTriangle(point,
			*node.m_point, *node.m_next->m_point);

	triangle->MarkNeighbor(*node.m_triangle);
	tcx.AddToMap(triangle);

	Node* new_node = new Node(point);
	m_nodes.push_back(new_node);

	new_node->m_next = node.m_next;
	new_node->m_prev = &node;
	node.m_next->m_prev = new_node;
	node.m_next = new_node;

	if (!Legalize(tcx, *triangle)) {
		tcx.MapTriangleToNodes(*triangle);
	}

	return *new_node;
}

void Sweep::Fill(SweepContext& tcx, Node& node)
{
	TriangulationTriangle* triangle = new TriangulationTriangle(
			*node.m_prev->m_point, *node.m_point, *node.m_next->m_point);

	// TO DO: should copy the constrained_edge value from neighbor triangles
	//       for now constrained_edge values are copied during the legalize
	triangle->MarkNeighbor(*node.m_prev->m_triangle);
	triangle->MarkNeighbor(*node.m_triangle);

	tcx.AddToMap(triangle);

	// Update the advancing front
	node.m_prev->m_next = node.m_next;
	node.m_next->m_prev = node.m_prev;

	// If it was legalized the triangle has already been mapped
	if (!Legalize(tcx, *triangle)) {
		tcx.MapTriangleToNodes(*triangle);
	}

}

void Sweep::FillAdvancingFront(SweepContext& tcx, Node& n)
{
	// Fill right holes
	Node* node = n.m_next;

	while (node->m_next) {
		double angle = HoleAngle(*node);
		if (angle > M_PI_2 || angle < -M_PI_2)
			break;
//		Fill(tcx, *node);
//		node = node->m_next;

// LEAK FIX
        Node *tmp = node;
        node = node->m_next;
        Fill(tcx, *tmp);
	}

	// Fill left holes
	node = n.m_prev;

	while (node->m_prev) {
		double angle = HoleAngle(*node);
		if (angle > M_PI_2 || angle < -M_PI_2)
			break;
		Fill(tcx, *node);
		node = node->m_prev;
	}

	// Fill right basins
	if (n.m_next && n.m_next->m_next) {
		double angle = BasinAngle(n);
		if (angle < PI_3div4) {
			FillBasin(tcx, n);
		}
	}
}

double Sweep::BasinAngle(Node& node)
{
	double ax = node.m_point->m_x - node.m_next->m_next->m_point->m_x;
	double ay = node.m_point->m_y - node.m_next->m_next->m_point->m_y;
	return atan2(ay, ax);
}

double Sweep::HoleAngle(Node& node)
{
	/* Complex plane
	 * ab = cosA +i*sinA
	 * ab = (ax + ay*i)(bx + by*i) = (ax*bx + ay*by) + i(ax*by-ay*bx)
	 * atan2(y,x) computes the principal value of the argument function
	 * applied to the complex number x+iy
	 * Where x = ax*bx + ay*by
	 *       y = ax*by - ay*bx
	 */
	double ax = node.m_next->m_point->m_x - node.m_point->m_x;
	double ay = node.m_next->m_point->m_y - node.m_point->m_y;
	double bx = node.m_prev->m_point->m_x - node.m_point->m_x;
	double by = node.m_prev->m_point->m_y - node.m_point->m_y;
	return atan2(ax * by - ay * bx, ax * bx + ay * by);
}

bool Sweep::Legalize(SweepContext& tcx, TriangulationTriangle& t)
{
	// To legalize a triangle we start by finding if any of the three edges
	// violate the Delaunay condition
	for (int i = 0; i < 3; i++) {
		if (t.m_delaunay_edge[i])
			continue;

		TriangulationTriangle* ot = t.GetNeighbor(i);

		if (ot) {
			Point* p = t.GetPoint(i);
			Point* op = ot->OppositePoint(t, *p);
			int oi = ot->Index(op);

			// If this is a Constrained Edge or a Delaunay Edge(only during recursive legalization)
			// then we should not try to legalize
			if (ot->m_constrained_edge[oi] || ot->m_delaunay_edge[oi]) {
				t.m_constrained_edge[i] = ot->m_constrained_edge[oi];
				continue;
			}

			bool inside = Incircle(*p, *t.PointCCW(*p), *t.PointCW(*p), *op);

			if (inside) {
				// Lets mark this shared edge as Delaunay
				t.m_delaunay_edge[i] = true;
				ot->m_delaunay_edge[oi] = true;

				// Lets rotate shared edge one vertex CW to legalize it
				RotateTrianglePair(t, *p, *ot, *op);

				// We now got one valid Delaunay Edge shared by two triangles
				// This gives us 4 new edges to check for Delaunay

				// Make sure that triangle to node mapping is done only one time for a specific triangle
				bool not_legalized = !Legalize(tcx, t);
				if (not_legalized) {
					tcx.MapTriangleToNodes(t);
				}

				not_legalized = !Legalize(tcx, *ot);
				if (not_legalized)
					tcx.MapTriangleToNodes(*ot);

				// Reset the Delaunay edges, since they only are valid Delaunay edges
				// until we add a new triangle or point.
				// XX X: need to think about this. Can these edges be tried after we
				//      return to previous recursive level?
				t.m_delaunay_edge[i] = false;
				ot->m_delaunay_edge[oi] = false;

				// If triangle have been legalized no need to check the other edges since
				// the recursive legalization will handles those so we can end here.
				return true;
			}
		}
	}
	return false;
}

bool Sweep::Incircle(Point& pa, Point& pb, Point& pc, Point& pd)
{
	double adx = pa.m_x - pd.m_x;
	double ady = pa.m_y - pd.m_y;
	double bdx = pb.m_x - pd.m_x;
	double bdy = pb.m_y - pd.m_y;

	double adxbdy = adx * bdy;
	double bdxady = bdx * ady;
	double oabd = adxbdy - bdxady;

	if (oabd <= 0)
		return false;

	double cdx = pc.m_x - pd.m_x;
	double cdy = pc.m_y - pd.m_y;

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

void Sweep::RotateTrianglePair(TriangulationTriangle& t, Point& p,
		TriangulationTriangle& ot, Point& op)
{
	TriangulationTriangle* n1, *n2, *n3, *n4;
	n1 = t.NeighborCCW(p);
	n2 = t.NeighborCW(p);
	n3 = ot.NeighborCCW(op);
	n4 = ot.NeighborCW(op);

	bool ce1, ce2, ce3, ce4;
	ce1 = t.GetConstrainedEdgeCCW(p);
	ce2 = t.GetConstrainedEdgeCW(p);
	ce3 = ot.GetConstrainedEdgeCCW(op);
	ce4 = ot.GetConstrainedEdgeCW(op);

	bool de1, de2, de3, de4;
	de1 = t.GetDelunayEdgeCCW(p);
	de2 = t.GetDelunayEdgeCW(p);
	de3 = ot.GetDelunayEdgeCCW(op);
	de4 = ot.GetDelunayEdgeCW(op);

	t.Legalize(p, op);
	ot.Legalize(op, p);

	// Remap delaunay_edge
	ot.SetDelunayEdgeCCW(p, de1);
	t.SetDelunayEdgeCW(p, de2);
	t.SetDelunayEdgeCCW(op, de3);
	ot.SetDelunayEdgeCW(op, de4);

	// Remap constrained_edge
	ot.SetConstrainedEdgeCCW(p, ce1);
	t.SetConstrainedEdgeCW(p, ce2);
	t.SetConstrainedEdgeCCW(op, ce3);
	ot.SetConstrainedEdgeCW(op, ce4);

	// Remap neighbors
	// XX X: might optimize the markNeighbor by keeping track of
	//      what side should be assigned to what neighbor after the
	//      rotation. Now mark neighbor does lots of testing to find
	//      the right side.
	t.ClearNeighbors();
	ot.ClearNeighbors();
	if (n1)
		ot.MarkNeighbor(*n1);
	if (n2)
		t.MarkNeighbor(*n2);
	if (n3)
		t.MarkNeighbor(*n3);
	if (n4)
		ot.MarkNeighbor(*n4);
	t.MarkNeighbor(ot);
}

void Sweep::FillBasin(SweepContext& tcx, Node& node)
{
	if (Orient2d(*node.m_point, *node.m_next->m_point, *node.m_next->m_next->m_point)
			== CCW) {
		tcx.m_basin.m_leftNode = node.m_next->m_next;
	} else {
		tcx.m_basin.m_leftNode = node.m_next;
	}

	// Find the bottom and right node
	tcx.m_basin.m_bottomNode = tcx.m_basin.m_leftNode;
	while (tcx.m_basin.m_bottomNode->m_next
			&& tcx.m_basin.m_bottomNode->m_point->m_y
					>= tcx.m_basin.m_bottomNode->m_next->m_point->m_y) {
		tcx.m_basin.m_bottomNode = tcx.m_basin.m_bottomNode->m_next;
	}
	if (tcx.m_basin.m_bottomNode == tcx.m_basin.m_leftNode) {
		// No valid basin
		return;
	}

	tcx.m_basin.m_rightNode = tcx.m_basin.m_bottomNode;
	while (tcx.m_basin.m_rightNode->m_next
			&& tcx.m_basin.m_rightNode->m_point->m_y
					< tcx.m_basin.m_rightNode->m_next->m_point->m_y) {
		tcx.m_basin.m_rightNode = tcx.m_basin.m_rightNode->m_next;
	}
	if (tcx.m_basin.m_rightNode == tcx.m_basin.m_bottomNode) {
		// No valid basins
		return;
	}

	tcx.m_basin.m_width = tcx.m_basin.m_rightNode->m_point->m_x
			- tcx.m_basin.m_leftNode->m_point->m_x;
	tcx.m_basin.m_leftHighest = tcx.m_basin.m_leftNode->m_point->m_y
			> tcx.m_basin.m_rightNode->m_point->m_y;

	FillBasinReq(tcx, tcx.m_basin.m_bottomNode);
}

void Sweep::FillBasinReq(SweepContext& tcx, Node* node)
{
	// if shallow stop filling
	if (IsShallow(tcx, *node)) {
		return;
	}

	Fill(tcx, *node);

	if (node->m_prev == tcx.m_basin.m_leftNode
			&& node->m_next == tcx.m_basin.m_rightNode) {
		return;
	} else if (node->m_prev == tcx.m_basin.m_leftNode) {
		Orientation o = Orient2d(*node->m_point, *node->m_next->m_point,
				*node->m_next->m_next->m_point);
		if (o == CW) {
			return;
		}
		node = node->m_next;
	} else if (node->m_next == tcx.m_basin.m_rightNode) {
		Orientation o = Orient2d(*node->m_point, *node->m_prev->m_point,
				*node->m_prev->m_prev->m_point);
		if (o == CCW) {
			return;
		}
		node = node->m_prev;
	} else {
		// Continue with the neighbor node with lowest Y value
		if (node->m_prev->m_point->m_y < node->m_next->m_point->m_y) {
			node = node->m_prev;
		} else {
			node = node->m_next;
		}
	}

	FillBasinReq(tcx, node);
}

bool Sweep::IsShallow(SweepContext& tcx, Node& node)
{
	double height;

	if (tcx.m_basin.m_leftHighest) {
		height = tcx.m_basin.m_leftNode->m_point->m_y - node.m_point->m_y;
	} else {
		height = tcx.m_basin.m_rightNode->m_point->m_y - node.m_point->m_y;
	}

	// if shallow stop filling
	if (tcx.m_basin.m_width > height) {
		return true;
	}
	return false;
}

void Sweep::FillEdgeEvent(SweepContext& tcx, Edge* edge, Node* node)
{
	if (tcx.m_edgeEvent.m_right) {
		FillRightAboveEdgeEvent(tcx, edge, node);
	} else {
		FillLeftAboveEdgeEvent(tcx, edge, node);
	}
}

void Sweep::FillRightAboveEdgeEvent(SweepContext& tcx, Edge* edge, Node* node)
{
	while (node->m_next->m_point->m_x < edge->m_p->m_x) {
		// Check if next node is below the edge
		if (Orient2d(*edge->m_q, *node->m_next->m_point, *edge->m_p) == CCW) {
			FillRightBelowEdgeEvent(tcx, edge, *node);
		} else {
			node = node->m_next;
		}
	}
}

void Sweep::FillRightBelowEdgeEvent(SweepContext& tcx, Edge* edge, Node& node)
{
	if (node.m_point->m_x < edge->m_p->m_x) {
		if (Orient2d(*node.m_point, *node.m_next->m_point, *node.m_next->m_next->m_point)
				== CCW) {
			// Concave
			FillRightConcaveEdgeEvent(tcx, edge, node);
		} else {
			// Convex
			FillRightConvexEdgeEvent(tcx, edge, node);
			// Retry this one
			FillRightBelowEdgeEvent(tcx, edge, node);
		}
	}
}

void Sweep::FillRightConcaveEdgeEvent(SweepContext& tcx, Edge* edge, Node& node)
{
	Fill(tcx, *node.m_next);
	if (node.m_next->m_point != edge->m_p) {
		// Next above or below edge?
		if (Orient2d(*edge->m_q, *node.m_next->m_point, *edge->m_p) == CCW) {
			// Below
			if (Orient2d(*node.m_point, *node.m_next->m_point,
					*node.m_next->m_next->m_point) == CCW) {
				// Next is concave
				FillRightConcaveEdgeEvent(tcx, edge, node);
			} else {
				// Next is convex
			}
		}
	}

}

void Sweep::FillRightConvexEdgeEvent(SweepContext& tcx, Edge* edge, Node& node)
{
	// Next concave or convex?
	if (Orient2d(*node.m_next->m_point, *node.m_next->m_next->m_point,
			*node.m_next->m_next->m_next->m_point) == CCW) {
		// Concave
		FillRightConcaveEdgeEvent(tcx, edge, *node.m_next);
	} else {
		// Convex
		// Next above or below edge?
		if (Orient2d(*edge->m_q, *node.m_next->m_next->m_point, *edge->m_p) == CCW) {
			// Below
			FillRightConvexEdgeEvent(tcx, edge, *node.m_next);
		} else {
			// Above
		}
	}
}

void Sweep::FillLeftAboveEdgeEvent(SweepContext& tcx, Edge* edge, Node* node)
{
	while (node->m_prev->m_point->m_x > edge->m_p->m_x) {
		// Check if next node is below the edge
		if (Orient2d(*edge->m_q, *node->m_prev->m_point, *edge->m_p) == CW) {
			FillLeftBelowEdgeEvent(tcx, edge, *node);
		} else {
			node = node->m_prev;
		}
	}
}

void Sweep::FillLeftBelowEdgeEvent(SweepContext& tcx, Edge* edge, Node& node)
{
	if (node.m_point->m_x > edge->m_p->m_x) {
		if (Orient2d(*node.m_point, *node.m_prev->m_point, *node.m_prev->m_prev->m_point)
				== CW) {
			// Concave
			FillLeftConcaveEdgeEvent(tcx, edge, node);
		} else {
			// Convex
			FillLeftConvexEdgeEvent(tcx, edge, node);
			// Retry this one
			FillLeftBelowEdgeEvent(tcx, edge, node);
		}
	}
}

void Sweep::FillLeftConvexEdgeEvent(SweepContext& tcx, Edge* edge, Node& node)
{
	// Next concave or convex?
	if (Orient2d(*node.m_prev->m_point, *node.m_prev->m_prev->m_point,
			*node.m_prev->m_prev->m_prev->m_point) == CW) {
		// Concave
		FillLeftConcaveEdgeEvent(tcx, edge, *node.m_prev);
	} else {
		// Convex
		// Next above or below edge?
		if (Orient2d(*edge->m_q, *node.m_prev->m_prev->m_point, *edge->m_p) == CW) {
			// Below
			FillLeftConvexEdgeEvent(tcx, edge, *node.m_prev);
		} else {
			// Above
		}
	}
}

void Sweep::FillLeftConcaveEdgeEvent(SweepContext& tcx, Edge* edge, Node& node)
{
	Fill(tcx, *node.m_prev);
	if (node.m_prev->m_point != edge->m_p) {
		// Next above or below edge?
		if (Orient2d(*edge->m_q, *node.m_prev->m_point, *edge->m_p) == CW) {
			// Below
			if (Orient2d(*node.m_point, *node.m_prev->m_point,
					*node.m_prev->m_prev->m_point) == CW) {
				// Next is concave
				FillLeftConcaveEdgeEvent(tcx, edge, node);
			} else {
				// Next is convex
			}
		}
	}

}

void Sweep::FlipEdgeEvent(SweepContext& tcx, Point& ep, Point& eq,
		TriangulationTriangle* t, Point& p)
{
	TriangulationTriangle& ot = t->NeighborAcross(p);
	Point& op = *ot.OppositePoint(*t, p);

	if (&ot == NULL) {
		// If we want to integrate the fillEdgeEvent do it here
		// With current implementation we should never get here
		//throw new RuntimeException( "[BUG:FIXM E] FLIP failed due to missing triangle");
		assert(0);
	}

	if (InScanArea(p, *t->PointCCW(p), *t->PointCW(p), op)) {
		// Lets rotate shared edge one vertex CW
		RotateTrianglePair(*t, p, ot, op);
		tcx.MapTriangleToNodes(*t);
		tcx.MapTriangleToNodes(ot);

		if (p == eq && op == ep) {
			if (eq == *tcx.m_edgeEvent.m_constrainedEdge->m_q
					&& ep == *tcx.m_edgeEvent.m_constrainedEdge->m_p) {
				t->MarkConstrainedEdge(&ep, &eq);
				ot.MarkConstrainedEdge(&ep, &eq);
				Legalize(tcx, *t);
				Legalize(tcx, ot);
			} else {
				// xx x: I think one of the triangles should be legalized here?
			}
		} else {
			Orientation o = Orient2d(eq, op, ep);
			t = &NextFlipTriangle(tcx, (int) o, *t, ot, p, op);
			FlipEdgeEvent(tcx, ep, eq, t, p);
		}
	} else {
		Point& newP = NextFlipPoint(ep, eq, ot, op);
		FlipScanEdgeEvent(tcx, ep, eq, *t, ot, newP);
		EdgeEvent(tcx, ep, eq, t, p);
	}
}

TriangulationTriangle& Sweep::NextFlipTriangle(SweepContext& tcx, int o,
		TriangulationTriangle& t, TriangulationTriangle& ot, Point& p, Point& op)
{
	if (o == CCW) {
		// ot is not crossing edge after flip
		int edge_index = ot.EdgeIndex(&p, &op);
		ot.m_delaunay_edge[edge_index] = true;
		Legalize(tcx, ot);
		ot.ClearDelunayEdges();
		return t;
	}

	// t is not crossing edge after flip
	int edge_index = t.EdgeIndex(&p, &op);

	t.m_delaunay_edge[edge_index] = true;
	Legalize(tcx, t);
	t.ClearDelunayEdges();
	return ot;
}

Point& Sweep::NextFlipPoint(Point& ep, Point& eq, TriangulationTriangle& ot, Point& op)
{
	Orientation o2d = Orient2d(eq, op, ep);
	if (o2d == CW) {
		// Right
		return *ot.PointCCW(op);
	} else if (o2d == CCW) {
		// Left
		return *ot.PointCW(op);
	} else {
		//throw new RuntimeException("[Unsupported] Opposing point on constrained edge");
		assert(0);
	}
}

void Sweep::FlipScanEdgeEvent(SweepContext& tcx, Point& ep, Point& eq,
		TriangulationTriangle& flip_triangle, TriangulationTriangle& t, Point& p)
{
	TriangulationTriangle& ot = t.NeighborAcross(p);
	Point& op = *ot.OppositePoint(t, p);

	if (&t.NeighborAcross(p) == NULL) {
		// If we want to integrate the fillEdgeEvent do it here
		// With current implementation we should never get here
		//throw new RuntimeException( "[BUG:FIXM E] FLIP failed due to missing triangle");
		assert(0);
	}

	if (InScanArea(eq, *flip_triangle.PointCCW(eq), *flip_triangle.PointCW(eq),
			op)) {
		// flip with new edge op->eq
		FlipEdgeEvent(tcx, eq, op, &ot, op);
		// To do: Actually I just figured out that it should be possible to
		//       improve this by getting the next ot and op before the the above
		//       flip and continue the flipScanEdgeEvent here
		// set new ot and op here and loop back to inScanArea test
		// also need to set a new flip_triangle first
		// Turns out at first glance that this is somewhat complicated
		// so it will have to wait.
	} else {
		Point& newP = NextFlipPoint(ep, eq, ot, op);
		FlipScanEdgeEvent(tcx, ep, eq, flip_triangle, ot, newP);
	}
}

Sweep::~Sweep()
{
	// Clean up memory
	for (unsigned int i = 0; i < m_nodes.size(); i++) {
		delete m_nodes[i];
	}

}

}

