//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2008 Ulrich von Zadow
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

#include "VertexData.h"
#include "../base/Exception.h"

#include <assert.h>

#include <iostream>
#include <stddef.h>
#include <string.h>

using namespace std;

namespace avg {

VertexData::VertexData()
    : m_NumVerts(0),
      m_ReserveVerts(10),
      m_NumIndexes(0),
      m_ReserveIndexes(20)
{
    m_pVertexData = new T2V3C4Vertex[m_ReserveVerts];
    m_pIndexData = new unsigned int[m_ReserveIndexes];
}

VertexData::~VertexData()
{
    delete[] m_pVertexData;
    delete[] m_pIndexData;
}

void VertexData::setPos(int vertexIndex, const DPoint& pos, 
        const DPoint& texPos, const Pixel32& color)
{
    assert(vertexIndex < m_NumVerts);
    T2V3C4Vertex* pVertex = &m_pVertexData[vertexIndex];
    pVertex->m_Pos[0] = (GLfloat)pos.x;
    pVertex->m_Pos[1] = (GLfloat)pos.y;
    pVertex->m_Pos[2] = 0.0;
    pVertex->m_Tex[0] = (GLfloat)texPos.x;
    pVertex->m_Tex[1] = (GLfloat)texPos.y;
    pVertex->m_Color = color;
}

void VertexData::setIndex(int i, int vertexIndex)
{
    m_pIndexData[i] = vertexIndex;
}

void VertexData::changeSize(int numVerts, int numIndexes)
{
    m_NumVerts = numVerts;
    m_NumIndexes = numIndexes;
    if (m_NumVerts > m_ReserveVerts) {
        m_ReserveVerts = int(m_ReserveVerts*1.5);
        if (m_ReserveVerts < m_NumVerts) {
            m_ReserveVerts = m_NumVerts;
        }
        delete m_pVertexData;
        m_pVertexData = new T2V3C4Vertex[m_ReserveVerts];
    }
    if (m_NumIndexes > m_ReserveIndexes) {
        m_ReserveIndexes = int(m_ReserveIndexes*1.5);
        if (m_ReserveIndexes < m_NumIndexes) {
            m_ReserveIndexes = m_NumIndexes;
        }
        delete m_pIndexData;
        m_pIndexData = new unsigned int[m_ReserveIndexes];
    }
}

int VertexData::getNumVerts() const
{
    return m_NumVerts;
}

int VertexData::getNumIndexes() const
{
    return m_NumIndexes;
}

const T2V3C4Vertex * VertexData::getVertexData() const
{
    return m_pVertexData;
}

const unsigned int * VertexData::getIndexData() const
{
    return m_pIndexData;
}

}
