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

VertexData::VertexData(int numVerts, int numIndexes, int reserveVerts, int reserveIndexes)
    : m_NumVerts(numVerts),
      m_NumIndexes(numIndexes),
      m_ReserveVerts(reserveVerts),
      m_ReserveIndexes(reserveIndexes)
{
    if (m_NumVerts > m_ReserveVerts) {
        m_ReserveVerts = m_NumVerts;
    }
    if (m_NumIndexes > m_ReserveIndexes) {
        m_ReserveIndexes = m_NumIndexes;
    }
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

void VertexData::setTriIndexes(int i, int v0, int v1, int v2)
{
    m_pIndexData[i] = v0;
    m_pIndexData[i+1] = v1;
    m_pIndexData[i+2] = v2;
}

void VertexData::setVertexData(int vertexIndex, int indexIndex, 
        const VertexDataPtr& pVertexes)
{
    int numVerts = pVertexes->getNumVerts();
    int numIndexes = pVertexes->getNumIndexes();
    assert(vertexIndex+numVerts<=m_NumVerts);
    assert(indexIndex+numIndexes<=m_NumIndexes);

    memcpy(m_pVertexData+vertexIndex, pVertexes->getVertexData(), 
            numVerts*sizeof(T2V3C4Vertex));
    memcpy(m_pIndexData+indexIndex, pVertexes->getIndexData(),
            numIndexes*sizeof(unsigned int));
    unsigned int * pCurIndex = &(m_pIndexData[indexIndex]);
    for (int i=0; i<numIndexes; ++i) {
        *pCurIndex += vertexIndex;
        ++pCurIndex;
    }
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

void VertexData::dump() const
{
    cerr << "Vertexes: " << endl;
    for (int i=0; i<m_NumVerts; ++i) {
        GLfloat* pos = m_pVertexData[i].m_Pos;
        cerr << "  (" << pos[0] << ", " << pos[1] << ", " << pos[2] << ")" << endl;
    }
    cerr << "Indexes: " << endl;
    for (int i=0; i<m_NumIndexes; ++i) {
        if (i%3 == 0) {
            cerr << " " << endl;
        }
        cerr << " " << m_pIndexData[i];
    }
    cerr << endl;
}

int VertexData::getReservedVerts() const 
{
    return m_ReserveVerts;
}
 
int VertexData::getReservedIndexes() const 
{
    return m_ReserveIndexes;
}
 
T2V3C4Vertex* VertexData::getVertexData() 
{
    return m_pVertexData;
}
 
unsigned int* VertexData::getIndexData() 
{
    return m_pIndexData;
}
 

}
