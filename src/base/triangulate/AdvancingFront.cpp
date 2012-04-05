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

#include "AdvancingFront.h"

namespace avg {

AdvancingFront::AdvancingFront(Node& head, Node& tail)
{
    m_Head = &head;
    m_Tail = &tail;
    m_SearchNode = &head;
}

Node* AdvancingFront::locateNode(const double& x)
{
    Node* node = m_SearchNode;

    if (x < node->m_Value) {
        while ((node = node->m_Prev) != NULL) {
            if (x >= node->m_Value) {
                m_SearchNode = node;
                return node;
            }
        }
    } else {
        while ((node = node->m_Next) != NULL) {
            if (x < node->m_Value) {
                m_SearchNode = node->m_Prev;
                return node->m_Prev;
            }
        }
    }
    return NULL;
}

Node* AdvancingFront::findSearchNode(const double& x)
{
    // TO DO: implement BST index
    return m_SearchNode;
}

Node* AdvancingFront::locatePoint(const Point* point)
{
    const double px = point->m_X;
    Node* node = findSearchNode(px);
    const double nx = node->m_Point->m_X;

    if (px == nx) {
        if (point != node->m_Point) {
            // We might have two nodes with same x value for a short time
            if (point == node->m_Prev->m_Point) {
                node = node->m_Prev;
            } else if (point == node->m_Next->m_Point) {
                node = node->m_Next;
            } else {
                assert(0);
            }
        }
    } else if (px < nx) {
        while ((node = node->m_Prev) != NULL) {
            if (point == node->m_Point) {
                break;
            }
        }
    } else {
        while ((node = node->m_Next) != NULL) {
            if (point == node->m_Point)
                break;
        }
    }
    if (node)
        m_SearchNode = node;
    return node;
}

AdvancingFront::~AdvancingFront() {}

}

