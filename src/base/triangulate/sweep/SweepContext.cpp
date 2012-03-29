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
#include "SweepContext.h"
#include <algorithm>
#include "AdvancingFront.h"

namespace avg {

SweepContext::SweepContext(std::vector<Point*> polyline)
{
	m_basin = Basin();
	m_edgeEvent = EdgeEvent();

	m_points = polyline;

	InitEdges(m_points);
}

void SweepContext::AddHole(std::vector<Point*> polyline)
{
	InitEdges(polyline);
	for (unsigned int i = 0; i < polyline.size(); i++) {
		m_points.push_back(polyline[i]);
	}
}

void SweepContext::AddPoint(Point* point)
{
	m_points.push_back(point);
}

std::vector<TriangulationTriangle*> SweepContext::GetTriangles()
{
	return m_triangles;
}

void SweepContext::InitTriangulation()
{
	double xmax(m_points[0]->m_x), xmin(m_points[0]->m_x);
	double ymax(m_points[0]->m_y), ymin(m_points[0]->m_y);

	// Calculate bounds.
	for (unsigned int i = 0; i < m_points.size(); i++) {
		Point& p = *m_points[i];
		if (p.m_x > xmax)
			xmax = p.m_x;
		if (p.m_x < xmin)
			xmin = p.m_x;
		if (p.m_y > ymax)
			ymax = p.m_y;
		if (p.m_y < ymin)
			ymin = p.m_y;
	}

	double dx = kAlpha * (xmax - xmin);
	double dy = kAlpha * (ymax - ymin);
	m_head = new Point(xmax + dx, ymin - dy, 0); //!!!!!!!!!!!!!
	m_tail = new Point(xmin - dx, ymin - dy, 0); //!!!!!!!!!!!!!

	// Sort points along y-axis
	std::sort(m_points.begin(), m_points.end(), cmp);

}

void SweepContext::InitEdges(std::vector<Point*> polyline)
{
	int num_points = polyline.size();
	for (int i = 0; i < num_points; i++) {
		int j = i < num_points - 1 ? i + 1 : 0;

		m_edgeList.push_back(new Edge(*polyline[i], *polyline[j]));
	}
}

Point* SweepContext::GetPoint(const int& index)
{
	return m_points[index];
}

void SweepContext::AddToMap(TriangulationTriangle* triangle)
{
	m_map.push_back(triangle);
}

Node& SweepContext::LocateNode(Point& point)
{
	// TO DO implement search tree
	return *m_front->LocateNode(point.m_x);
}

void SweepContext::CreateAdvancingFront(std::vector<Node*> nodes)
{

	(void) nodes; //!!!!!!!
	// Initial triangle
	TriangulationTriangle* triangle = new TriangulationTriangle(*m_points[0], *m_tail,
			*m_head);

	m_map.push_back(triangle);

	m_afHead = new Node(*triangle->GetPoint(1), *triangle);
	m_afMiddle = new Node(*triangle->GetPoint(0), *triangle);
	m_afTail = new Node(*triangle->GetPoint(2));
	m_front = new AdvancingFront(*m_afHead, *m_afTail);

	// to do: More intuitive if head is middles next and not previous?
	//       so swap head and tail
	m_afHead->m_next = m_afMiddle;
	m_afMiddle->m_next = m_afTail;
	m_afMiddle->m_prev = m_afHead;
	m_afTail->m_prev = m_afMiddle;
}

void SweepContext::RemoveNode(Node* node)
{
	delete node;
}

void SweepContext::MapTriangleToNodes(TriangulationTriangle& t)
{
	for (int i = 0; i < 3; i++) {
		if (!t.GetNeighbor(i)) {
			Node* n = m_front->LocatePoint(t.PointCW(*t.GetPoint(i)));
			if (n)
				n->m_triangle = &t;
		}
	}
}

void SweepContext::RemoveFromMap(TriangulationTriangle* triangle)
{
	m_map.remove(triangle);
}

void SweepContext::MeshClean(TriangulationTriangle& triangle)
{
	if (&triangle != NULL && !triangle.IsInterior()) {
		triangle.IsInterior(true);
		m_triangles.push_back(&triangle);
		for (int i = 0; i < 3; i++) {
			if (!triangle.m_constrained_edge[i])
				MeshClean(*triangle.GetNeighbor(i));
		}
	}
}

SweepContext::~SweepContext()
{

	// Clean up memory

	delete m_head;
	delete m_tail;
	delete m_front;
	delete m_afHead;
	delete m_afMiddle;
	delete m_afTail;

	typedef std::list<TriangulationTriangle*> type_list;

	for (type_list::iterator iter = m_map.begin(); iter != m_map.end(); ++iter) {
		TriangulationTriangle* ptr = *iter;
		delete ptr;
	}

	for (unsigned int i = 0; i < m_edgeList.size(); i++) {
		delete m_edgeList[i];
	}

}

}
