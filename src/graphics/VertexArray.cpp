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

#include "VertexArray.h"
#include "../base/Exception.h"

#include <assert.h>

#include <iostream>
#include <stddef.h>
#include <string.h>

using namespace std;

namespace avg {

VertexArray::VertexArray(int numVerts, int numIndexes, int reserveVerts, 
        int reserveIndexes)
    : m_NumVerts(numVerts),
      m_ReserveVerts(reserveVerts),
      m_NumIndexes(numIndexes),
      m_ReserveIndexes(reserveIndexes),
      m_bDataChanged(true)
{
    if (m_NumVerts > m_ReserveVerts) {
        m_ReserveVerts = m_NumVerts;
    }
    if (m_NumIndexes > m_ReserveIndexes) {
        m_ReserveIndexes = m_NumIndexes;
    }
    glproc::GenBuffers(1, &m_GLVertexBufferID);
    glproc::GenBuffers(1, &m_GLIndexBufferID);
    m_pVertexData = new T2V3C4Vertex[m_ReserveVerts];
    m_pIndexData = new unsigned int[m_ReserveIndexes];
    setBufferSize();
}

VertexArray::~VertexArray()
{
    delete[] m_pVertexData;
    delete[] m_pIndexData;
    glproc::DeleteBuffers(1, &m_GLVertexBufferID);
    glproc::DeleteBuffers(1, &m_GLIndexBufferID);
}

void VertexArray::setPos(int vertexIndex, const DPoint& pos, 
        const DPoint& texPos, const Pixel32& color)
{
    assert(vertexIndex < m_NumVerts);
    T2V3C4Vertex* pVertex = &m_pVertexData[vertexIndex];
    if (pVertex->m_Pos[0] != (GLfloat)pos.x || 
            pVertex->m_Pos[1] != (GLfloat)pos.y ||
            pVertex->m_Tex[0] != (GLfloat)texPos.x || 
            pVertex->m_Tex[1] != (GLfloat)texPos.y ||
            pVertex->m_Color != color)
    {
        pVertex->m_Pos[0] = (GLfloat)pos.x;
        pVertex->m_Pos[1] = (GLfloat)pos.y;
        pVertex->m_Pos[2] = 0.0;
        pVertex->m_Tex[0] = (GLfloat)texPos.x;
        pVertex->m_Tex[1] = (GLfloat)texPos.y;
        pVertex->m_Color = color;
        m_bDataChanged = true;
    }
}

void VertexArray::setIndex(int i, int vertexIndex)
{
    m_pIndexData[i] = vertexIndex;
}

void VertexArray::setVertexData(int vertexIndex, int indexIndex, 
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
    m_bDataChanged = true;
}

void VertexArray::changeSize(int numVerts, int numIndexes)
{
    int oldNumVerts = m_NumVerts;
    m_NumVerts = numVerts;
    m_NumIndexes = numIndexes;
    bool bBufferSizeChanged = false;
    if (m_NumVerts > m_ReserveVerts) {
        m_ReserveVerts = int(m_ReserveVerts*1.5);
        if (m_ReserveVerts < m_NumVerts) {
            m_ReserveVerts = m_NumVerts;
        }
        T2V3C4Vertex * pOldVertexes = m_pVertexData;
        m_pVertexData = new T2V3C4Vertex[m_ReserveVerts];
        memcpy(m_pVertexData, pOldVertexes, sizeof(T2V3C4Vertex)*oldNumVerts);
        delete[] pOldVertexes;
        bBufferSizeChanged = true;
    }
    if (m_NumIndexes > m_ReserveIndexes) {
        m_ReserveIndexes = int(m_ReserveIndexes*1.5);
        if (m_ReserveIndexes < m_NumIndexes) {
            m_ReserveIndexes = m_NumIndexes;
        }
        unsigned int * pOldIndexes = m_pIndexData;
        m_pIndexData = new unsigned int[m_ReserveIndexes];
        memcpy(m_pIndexData, pOldIndexes, sizeof(unsigned int)*oldNumVerts);
        delete[] pOldIndexes;
        bBufferSizeChanged = true;
    }
    if (bBufferSizeChanged) {
        setBufferSize();
    }
    m_bDataChanged = true;
}

void VertexArray::update()
{
    if (m_bDataChanged) {
        glproc::BindBuffer(GL_ARRAY_BUFFER, m_GLVertexBufferID);
        glproc::BufferData(GL_ARRAY_BUFFER, m_ReserveVerts*sizeof(T2V3C4Vertex), 0, 
                GL_STREAM_DRAW);
        void * pOGLBuffer = glproc::MapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        memcpy(pOGLBuffer, m_pVertexData, m_NumVerts*sizeof(T2V3C4Vertex));
        glproc::UnmapBuffer(GL_ARRAY_BUFFER);
        glproc::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_GLIndexBufferID);
        glproc::BufferData(GL_ELEMENT_ARRAY_BUFFER, m_ReserveIndexes*sizeof(unsigned int), 0, 
                GL_STREAM_DRAW);
        pOGLBuffer = glproc::MapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
        memcpy(pOGLBuffer, m_pIndexData, m_NumIndexes*sizeof(unsigned int));
        glproc::UnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    }
    m_bDataChanged = false;
}

void VertexArray::draw()
{
    if (m_bDataChanged) {
        update();
    }
    glproc::BindBuffer(GL_ARRAY_BUFFER, m_GLVertexBufferID);
    glTexCoordPointer(2, GL_FLOAT, sizeof(T2V3C4Vertex), 0);
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(T2V3C4Vertex), 
            (void *)(offsetof(T2V3C4Vertex, m_Color)));
    glVertexPointer(3, GL_FLOAT, sizeof(T2V3C4Vertex),
            (void *)(offsetof(T2V3C4Vertex, m_Pos)));
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "VertexArray::draw:1");

    glproc::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_GLIndexBufferID);
    glDrawElements(GL_TRIANGLES, m_NumIndexes, GL_UNSIGNED_INT, 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "VertexArray::draw():2");
}

void VertexArray::setBufferSize() 
{
    glproc::BindBuffer(GL_ARRAY_BUFFER, m_GLVertexBufferID);
    glproc::BufferData(GL_ARRAY_BUFFER, m_ReserveVerts*sizeof(T2V3C4Vertex), 0, 
            GL_STREAM_DRAW);
    glproc::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_GLVertexBufferID);
    glproc::BufferData(GL_ELEMENT_ARRAY_BUFFER, m_ReserveIndexes*sizeof(unsigned int), 0, 
            GL_STREAM_DRAW);
}

}

