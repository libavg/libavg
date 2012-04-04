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

/// Constructor
	SweepContext(std::vector<Point*> polyline);
/// Destructor
	~SweepContext();

	void setHead(Point* p1);

	Point* head();

	void setTail(Point* p1);

	Point* tail();

	int pointCount();

	Node& LocateNode(Point& point);

	void RemoveNode(Node* node);

	void CreateAdvancingFront(std::vector<Node*> nodes);

/// Try to map a node to all sides of this triangle that don't have a neighbor
	void MapTriangleToNodes(TriangulationTriangle& t);

	void AddToMap(TriangulationTriangle* triangle);

	Point* GetPoint(const int& index);

	Point* GetPoints();

	void RemoveFromMap(TriangulationTriangle* triangle);

	void AddHole(std::vector<Point*> polyline);

	void AddPoint(Point* point);

	AdvancingFront* front();

	void MeshClean(TriangulationTriangle& triangle);

	std::vector<TriangulationTriangle*> GetTriangles();

	std::vector<Edge*> m_edgeList;

	struct Basin
	{
		Node* m_leftNode;
		Node* m_bottomNode;
		Node* m_rightNode;
		double m_width;
		bool m_leftHighest;

		Basin()
		{
			Clear();
		}

		void Clear() {
			m_leftNode = NULL;
			m_bottomNode = NULL;
			m_rightNode = NULL;
			m_width = 0.0;
			m_leftHighest = false;
		}
	};

	struct EdgeEvent
	{
		Edge* m_constrainedEdge;
		bool m_right;

		EdgeEvent() :
				m_constrainedEdge(NULL), m_right(false) {
		}
	};

	Basin m_basin;
	EdgeEvent m_edgeEvent;

private:

	friend class Sweep;

	std::vector<TriangulationTriangle*> m_triangles;
	std::list<TriangulationTriangle*> m_map;
	std::vector<Point*> m_points;

// Advancing front
	AdvancingFront* m_front;
// head point used with advancing front
	Point* m_head;
// tail point used with advancing front
	Point* m_tail;

	Node *m_afHead, *m_afMiddle, *m_afTail;

	void InitTriangulation();
	void InitEdges(std::vector<Point*> polyline);

};

inline AdvancingFront* SweepContext::front()
{
	return m_front;
}

inline int SweepContext::pointCount()
{
	return m_points.size();
}

inline void SweepContext::setHead(Point* p1)
{
	m_head = p1;
}

inline Point* SweepContext::head()
{
	return m_head;
}

inline void SweepContext::setTail(Point* p1)
{
	m_tail = p1;
}

inline Point* SweepContext::tail()
{
	return m_tail;
}

}

#endif
