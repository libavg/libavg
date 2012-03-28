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

// Include guard
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
	double m_x, m_y;
	int m_index;

	/// The edges this point constitutes an upper ending point
	std::vector<Edge*> edge_list;

	/// Default constructor does nothing (for performance).
	Point() {
		m_x = 0.0;
		m_y = 0.0;
		m_index = 0;
	}

	/// Construct using coordinates.
	Point(double x, double y, int index) :
			m_x(x), m_y(y), m_index(index) {}

	/// Set this point to all zeros.
	void set_zero()
	{
		m_x = 0.0;
		m_y = 0.0;
	}

	/// Set this point to some specified coordinates.
	void set(double x_, double y_)
	{
		m_x = x_;
		m_y = y_;
	}

	/// Negate this point.
	Point operator -() const
	{
		Point v;
		v.set(-m_x, -m_y);
		return v;
	}

	/// Add a point to this point.
	void operator +=(const Point& v)
	{
		m_x += v.m_x;
		m_y += v.m_y;
	}

	/// Subtract a point from this point.
	void operator -=(const Point& v)
	{
		m_x -= v.m_x;
		m_y -= v.m_y;
	}

	/// Multiply this point by a scalar.
	void operator *=(double a)
	{
		m_x *= a;
		m_y *= a;
	}

	/// Get the length of this point (the norm).
	double Length() const
	{
		return sqrt(m_x * m_x + m_y * m_y);
	}

	/// Convert this point into a unit point. Returns the Length.
	double Normalize()
	{
		double len = Length();
		m_x /= len;
		m_y /= len;
		return len;
	}

};


// Represents a simple polygon's edge
struct Edge
{
	Point* m_p, *m_q;

	/// Constructor
	Edge(Point& p1, Point& p2) :m_p(&p1), m_q(&p2)
	{
		if (p1.m_y > p2.m_y) {
			m_q = &p1;
			m_p = &p2;
		} else if (p1.m_y == p2.m_y) {
			if (p1.m_x > p2.m_x) {
				m_q = &p1;
				m_p = &p2;
			} else if (p1.m_x == p2.m_x) {
				// Repeat points
				assert(false);
			}
		}
		m_q->edge_list.push_back(this);
	}
};


// Triangle-based data structures are know to have better performance than quad-edge structures
// See: J. Shewchuk, "Triangle: Engineering a 2D Quality Mesh Generator and Delaunay Triangulator"
//      "Triangulations in CGAL"
class TriangulationTriangle
{

public:

/// Constructor
	TriangulationTriangle(Point& a, Point& b, Point& c);

/// Flags to determine if an edge is a Constrained edge
	bool m_constrained_edge[3];
/// Flags to determine if an edge is a Delauney edge
	bool m_delaunay_edge[3];

	Point* GetPoint(const int& index);
	Point* PointCW(Point& point);
	Point* PointCCW(Point& point);
	Point* OppositePoint(TriangulationTriangle& t, Point& p);

	TriangulationTriangle* GetNeighbor(const int& index);
	void MarkNeighbor(Point* p1, Point* p2, TriangulationTriangle* t);
	void MarkNeighbor(TriangulationTriangle& t);

	void MarkConstrainedEdge(const int index);
	void MarkConstrainedEdge(Edge& edge);
	void MarkConstrainedEdge(Point* p, Point* q);

	unsigned int Index(const Point* p);
	unsigned int EdgeIndex(const Point* p1, const Point* p2);

	TriangulationTriangle* NeighborCW(Point& point);
	TriangulationTriangle* NeighborCCW(Point& point);
	bool GetConstrainedEdgeCCW(Point& p);
	bool GetConstrainedEdgeCW(Point& p);
	void SetConstrainedEdgeCCW(Point& p, bool ce);
	void SetConstrainedEdgeCW(Point& p, bool ce);
	bool GetDelunayEdgeCCW(Point& p);
	bool GetDelunayEdgeCW(Point& p);
	void SetDelunayEdgeCCW(Point& p, bool e);
	void SetDelunayEdgeCW(Point& p, bool e);

	bool Contains(Point* p);
	bool Contains(const Edge& e);
	bool Contains(Point* p, Point* q);
	void Legalize(Point& point);
	void Legalize(Point& opoint, Point& npoint);
	/**
	 * Clears all references to all other triangles and points
	 */
	void Clear();
	void ClearNeighbor(TriangulationTriangle *triangle);
	void ClearNeighbors();
	void ClearDelunayEdges();

	inline bool IsInterior();
	inline void IsInterior(bool b);

	TriangulationTriangle& NeighborAcross(Point& opoint);

	void DebugPrint();

private:

/// Triangle points
	Point* m_points[3];
/// Neighbor list
	TriangulationTriangle* m_neighbors[3];

/// Has this triangle been marked as an interior triangle?
	bool m_interior;
};


inline bool cmp(const Point* a, const Point* b)
{
	if (a->m_y < b->m_y) {
		return true;
	} else if (a->m_y == b->m_y) {
		// Make sure q is point with greater x value
		if (a->m_x < b->m_x) {
			return true;
		}
	}
	return false;
}
/*
 /// Add two points_ component-wise.
 inline Point operator +(const Point& a, const Point& b)
 {
 return Point(a.x + b.x, a.y + b.y);
 }

 /// Subtract two points_ component-wise.
 inline Point operator -(const Point& a, const Point& b)
 {
 return Point(a.x - b.x, a.y - b.y);
 }

 /// Multiply point by scalar
 inline Point operator *(double s, const Point& a)
 {
 return Point(s * a.x, s * a.y, a.index);
 } */

inline bool operator ==(const Point& a, const Point& b)
{
	return a.m_x == b.m_x && a.m_y == b.m_y;
}

inline bool operator !=(const Point& a, const Point& b)
{
	return a.m_x != b.m_x && a.m_y != b.m_y;
}

/// Peform the dot product on two vectors.
inline double Dot(const Point& a, const Point& b)
{
	return a.m_x * b.m_x + a.m_y * b.m_y;
}

/// Perform the cross product on two vectors. In 2D this produces a scalar.
inline double Cross(const Point& a, const Point& b)
{
	return a.m_x * b.m_y - a.m_y * b.m_x;
}

/// Perform the cross product on a point and a scalar. In 2D this produces
/// a point.
inline Point Cross(const Point& a, double s)
{
	return Point(s * a.m_y, -s * a.m_x, a.m_index);
}

/// Perform the cross product on a scalar and a point. In 2D this produces
/// a point.
inline Point Cross(const double s, const Point& a)
{
	return Point(-s * a.m_y, s * a.m_x, a.m_index);
}

inline Point* TriangulationTriangle::GetPoint(const int& index)
{
	return m_points[index];
}

inline TriangulationTriangle* TriangulationTriangle::GetNeighbor(
		const int& index)
{
	return m_neighbors[index];
}

inline bool TriangulationTriangle::Contains(Point* p)
{
	return p == m_points[0] || p == m_points[1] || p == m_points[2];
}

inline bool TriangulationTriangle::Contains(const Edge& e)
{
	return Contains(e.m_p) && Contains(e.m_q);
}

inline bool TriangulationTriangle::Contains(Point* p, Point* q)
{
	return Contains(p) && Contains(q);
}

inline bool TriangulationTriangle::IsInterior()
{
	return m_interior;
}

inline void TriangulationTriangle::IsInterior(bool b)
{
	m_interior = b;
}

}

#endif

