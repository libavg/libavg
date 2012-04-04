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

#ifndef ADVANCED_FRONT_H
#define ADVANCED_FRONT_H

#include "Shapes.h"

namespace avg {

struct Node;

// Advancing front node
struct Node
{
  Point* m_point;
  TriangulationTriangle* m_triangle;

  Node* m_next;
  Node* m_prev;

  double m_value;

  Node(Point& p) : m_point(&p), m_triangle(NULL), m_next(NULL), m_prev(NULL), m_value(p.m_x)
  {}

  Node(Point& p, TriangulationTriangle& t) : m_point(&p), m_triangle(&t), m_next(NULL), m_prev(NULL), m_value(p.m_x)
  {}

};

// Advancing front
class AdvancingFront
{

public:

AdvancingFront(Node& head, Node& tail);
// Destructor
~AdvancingFront();

Node* head();
void setHead(Node* node);
Node* tail();
void setTail(Node* node);
Node* search();
void setSearch(Node* node);

/// Locate insertion point along advancing front
Node* LocateNode(const double& x);

Node* LocatePoint(const Point* point);

private:

Node* m_head, *m_tail, *m_search_node;

Node* FindSearchNode(const double& x);
};


inline Node* AdvancingFront::head()
{
  return m_head;
}
inline void AdvancingFront::setHead(Node* node)
{
  m_head = node;
}

inline Node* AdvancingFront::tail()
{
  return m_tail;
}
inline void AdvancingFront::setTail(Node* node)
{
  m_tail = node;
}

inline Node* AdvancingFront::search()
{
  return m_search_node;
}

inline void AdvancingFront::setSearch(Node* node)
{
  m_search_node = node;
}

}

#endif
