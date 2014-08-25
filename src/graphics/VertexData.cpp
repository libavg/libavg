//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2014 Ulrich von Zadow
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

#include "GLContext.h"

#include "../base/Exception.h"
#include "../base/WideLine.h"
#include "../base/ObjectCounter.h"

#include <iostream>
#include <stddef.h>
#include <string.h>

using namespace std;
using namespace boost;

namespace avg {
    
const int VertexData::MIN_VERTEXES = 100;
const int VertexData::MIN_INDEXES = 100;

VertexData::VertexData(int reserveVerts, int reserveIndexes)
    : m_NumVerts(0),
      m_NumIndexes(0),
      m_ReserveVerts(reserveVerts),
      m_ReserveIndexes(reserveIndexes),
      m_bDataChanged(true)
{
    ObjectCounter::get()->incRef(&typeid(*this));
    if (m_ReserveVerts < MIN_VERTEXES) {
        m_ReserveVerts = MIN_VERTEXES;
    }
    if (m_ReserveIndexes < MIN_INDEXES) {
        m_ReserveIndexes = MIN_INDEXES;
    }
    
    m_pVertexData = new Vertex[m_ReserveVerts];
    m_pIndexData = new GL_INDEX_TYPE[m_ReserveIndexes];

}

VertexData::~VertexData()
{
    delete[] m_pVertexData;
    delete[] m_pIndexData;
    ObjectCounter::get()->decRef(&typeid(*this));
}

void VertexData::appendPos(const glm::vec2& pos, const glm::vec2& texPos,
        const Pixel32& color)
{
    if (m_NumVerts >= m_ReserveVerts-1) {
        grow();
    }
    Vertex* pVertex = &(m_pVertexData[m_NumVerts]);
    pVertex->m_Pos[0] = (GLfloat)(pos.x);
    pVertex->m_Pos[1] = (GLfloat)(pos.y);
    pVertex->m_Tex[0] = (GLfloat)(texPos.x);
    pVertex->m_Tex[1] = (GLfloat)(texPos.y);
    pVertex->m_Color = color;
    m_bDataChanged = true;
    m_NumVerts++;
}

void VertexData::appendTriIndexes(int v0, int v1, int v2)
{
    if (m_NumIndexes >= m_ReserveIndexes-3) {
        grow();
    }
    m_pIndexData[m_NumIndexes] = v0;
    m_pIndexData[m_NumIndexes+1] = v1;
    m_pIndexData[m_NumIndexes+2] = v2;
    m_NumIndexes += 3;
}

void VertexData::appendQuadIndexes(int v0, int v1, int v2, int v3)
{
    if (m_NumIndexes >= m_ReserveIndexes-6) {
        grow();
    }
    m_pIndexData[m_NumIndexes] = v0;
    m_pIndexData[m_NumIndexes+1] = v1;
    m_pIndexData[m_NumIndexes+2] = v2;
    m_pIndexData[m_NumIndexes+3] = v1;
    m_pIndexData[m_NumIndexes+4] = v2;
    m_pIndexData[m_NumIndexes+5] = v3;
    m_NumIndexes += 6;
}

void VertexData::addLineData(Pixel32 color, const glm::vec2& p1, const glm::vec2& p2, 
        float width, float tc1, float tc2)
{
    WideLine wl(p1, p2, width);
    int curVertex = getNumVerts();
    appendPos(wl.pl0, glm::vec2(tc1, 1), color);
    appendPos(wl.pr0, glm::vec2(tc1, 0), color);
    appendPos(wl.pl1, glm::vec2(tc2, 1), color);
    appendPos(wl.pr1, glm::vec2(tc2, 0), color);
    appendQuadIndexes(curVertex+1, curVertex, curVertex+3, curVertex+2); 
}

void VertexData::appendVertexData(const VertexDataPtr& pVertexes)
{
    int oldNumVerts = m_NumVerts;
    int oldNumIndexes = m_NumIndexes;
    m_NumVerts += pVertexes->getNumVerts();
    m_NumIndexes += pVertexes->getNumIndexes();
    if (m_NumVerts > m_ReserveVerts || m_NumIndexes > m_ReserveIndexes) {
        grow();
    }

    memcpy(&(m_pVertexData[oldNumVerts]), pVertexes->m_pVertexData, 
            pVertexes->getNumVerts()*sizeof(Vertex));
    int numIndexes = pVertexes->getNumIndexes();
    for (int i=0; i<numIndexes; ++i) {
        m_pIndexData[oldNumIndexes+i] = pVertexes->m_pIndexData[i] + oldNumVerts;
    }
    m_bDataChanged = true;
}

bool VertexData::hasDataChanged() const
{
    return m_bDataChanged;
}

void VertexData::resetDataChanged()
{
    m_bDataChanged = false;
}

void VertexData::reset()
{
    m_NumVerts = 0;
    m_NumIndexes = 0;
    m_bDataChanged = false;
}

int VertexData::getNumVerts() const
{
    return m_NumVerts;
}

int VertexData::getNumIndexes() const
{
    return m_NumIndexes;
}

void VertexData::dump() const
{
    dump(0, m_NumVerts, 0, m_NumIndexes);
}

void VertexData::dump(unsigned startVertex, int numVerts, unsigned startIndex, 
        int numIndexes) const
{
    cerr << numVerts << " vertexes: ";
    for (unsigned i=startVertex; i<startVertex+numVerts; ++i) {
        cerr << m_pVertexData[i] << endl;
    }
    cerr << endl;
    cerr << numIndexes << " indexes: ";
    for (unsigned i=startIndex; i<startIndex+numIndexes; ++i) {
        cerr << m_pIndexData[i] << " ";
    }
    cerr << endl;
}

void VertexData::grow()
{
    bool bChanged = false;
    if (m_NumVerts >= m_ReserveVerts-1) {
        bChanged = true;
        int oldReserveVerts = m_ReserveVerts;
        m_ReserveVerts = int(m_ReserveVerts*1.5);
#ifdef AVG_ENABLE_EGL
        if (m_ReserveVerts > 65535) {
            throw Exception(AVG_ERR_UNSUPPORTED, 
                    "Global maximum number of vertexes reached (65535).");
        }
#endif
        if (m_ReserveVerts < m_NumVerts) {
            m_ReserveVerts = m_NumVerts;
        }
        Vertex* pVertexData = m_pVertexData;
        m_pVertexData = new Vertex[m_ReserveVerts];
        memcpy(m_pVertexData, pVertexData, sizeof(Vertex)*oldReserveVerts);
        delete[] pVertexData;
    }
    if (m_NumIndexes >= m_ReserveIndexes-6) {
        bChanged = true;
        int oldReserveIndexes = m_ReserveIndexes;
        m_ReserveIndexes = int(m_ReserveIndexes*1.5);
        if (m_ReserveIndexes < m_NumIndexes) {
            m_ReserveIndexes = m_NumIndexes;
        }
        GL_INDEX_TYPE * pIndexData = m_pIndexData;
        m_pIndexData = new GL_INDEX_TYPE[m_ReserveIndexes];
        memcpy(m_pIndexData, pIndexData, sizeof(GL_INDEX_TYPE)*oldReserveIndexes);
        delete[] pIndexData;
    }
    if (bChanged) {
        m_bDataChanged = true;
    }
}

int VertexData::getReserveVerts() const
{
    return m_ReserveVerts;
}

int VertexData::getReserveIndexes() const
{
    return m_ReserveIndexes;
}

const Vertex * VertexData::getVertexPointer() const
{
    return m_pVertexData;
}

const GL_INDEX_TYPE * VertexData::getIndexPointer() const
{
    return m_pIndexData;
}

std::ostream& operator<<(std::ostream& os, const Vertex& v)
{
    os << "  ((" << v.m_Pos[0] << ", " << v.m_Pos[1] << "), (" 
            << v.m_Tex[0] << ", " << v.m_Tex[1] << "))";
    return os;
}

}

