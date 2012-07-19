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

#ifndef ADVANCED_FRONT_H
#define ADVANCED_FRONT_H

#include "Shapes.h"

namespace avg {

struct Node;

struct Node
{
  Point* m_Point;
  TriangulationTriangle* m_Triangle;

  Node* m_Next;
  Node* m_Prev;

  double m_Value;

  Node() : m_Point(NULL), m_Triangle(NULL), m_Next(NULL), m_Prev(NULL), m_Value(0)
  {}

  Node(Point& p) : m_Point(&p), m_Triangle(NULL), m_Next(NULL), m_Prev(NULL), m_Value(p.m_X)
  {}

  Node(Point& p, TriangulationTriangle& t) : m_Point(&p), m_Triangle(&t), m_Next(NULL), m_Prev(NULL), m_Value(p.m_X)
  {}

};


class AdvancingFront
{

public:

AdvancingFront(Node& head, Node& tail);

~AdvancingFront();

Node* head();
void setHead(Node* node);
Node* tail();
void setTail(Node* node);
Node* search();
void setSearch(Node* node);

/// Locate insertion point along advancing front
Node* locateNode(const double& x);

Node* locatePoint(const Point* point);

private:

Node* m_Head, *m_Tail, *m_SearchNode;

Node* findSearchNode(const double& x);
};


inline Node* AdvancingFront::head()
{
  return m_Head;
}
inline void AdvancingFront::setHead(Node* node)
{
  m_Head = node;
}

inline Node* AdvancingFront::tail()
{
  return m_Tail;
}
inline void AdvancingFront::setTail(Node* node)
{
  m_Tail = node;
}

inline Node* AdvancingFront::search()
{
  return m_SearchNode;
}

inline void AdvancingFront::setSearch(Node* node)
{
  m_SearchNode = node;
}

}

#endif
