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

using namespace std;

namespace avg {

VertexArray::VertexArray(int vertexesPerPrimitive, int numPrimitives, 
        int reservePrimitives)
    : m_VertexesPerPrimitive(vertexesPerPrimitive),
      m_NumPrimitives(numPrimitives),
      m_ReservePrimitives(reservePrimitives),
      m_bDataChanged(true)
{
    assert(vertexesPerPrimitive == 3 || vertexesPerPrimitive == 4);
    if (m_NumPrimitives > m_ReservePrimitives) {
        m_ReservePrimitives = m_NumPrimitives;
    }
    glproc::GenBuffers(1, &m_VBOArrayID);
    m_pVertexData = new T2V3C4Vertex[m_ReservePrimitives*m_VertexesPerPrimitive];
    setBufferSize();
}

VertexArray::~VertexArray()
{
    delete[] m_pVertexData;
    glproc::DeleteBuffers(1, &m_VBOArrayID);
}

void VertexArray::setPos(int primitiveIndex, int vertexIndex, const DPoint& pos, 
        const DPoint& texPos, const Pixel32& color)
{
    assert(primitiveIndex < m_NumPrimitives);
    T2V3C4Vertex* pVertex = &(m_pVertexData[primitiveIndex*m_VertexesPerPrimitive+
            vertexIndex]);
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

void VertexArray::changeSize(int numPrimitives)
{
    int oldNumPrimitives = m_NumPrimitives;
    m_NumPrimitives = numPrimitives;
    if (m_NumPrimitives > m_ReservePrimitives) {
        m_ReservePrimitives = int (m_ReservePrimitives*1.5);
        if (m_ReservePrimitives < m_NumPrimitives) {
            m_ReservePrimitives = m_NumPrimitives;
        }
        T2V3C4Vertex * pOldVertexes = m_pVertexData;
        m_pVertexData = new T2V3C4Vertex[m_ReservePrimitives*m_VertexesPerPrimitive];
        memcpy(m_pVertexData, pOldVertexes, 
                sizeof(T2V3C4Vertex)*oldNumPrimitives*m_VertexesPerPrimitive);
        delete[] pOldVertexes;
        setBufferSize();
    }
    m_bDataChanged = true;
}

void VertexArray::update()
{
    if (m_bDataChanged) {
        glproc::BindBuffer(GL_ARRAY_BUFFER, m_VBOArrayID);
        glproc::BufferData(GL_ARRAY_BUFFER, 
                m_ReservePrimitives*m_VertexesPerPrimitive*sizeof(T2V3C4Vertex), 0, 
                GL_STREAM_DRAW);
        void * pOGLBuffer = glproc::MapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        memcpy(pOGLBuffer, m_pVertexData, 
                m_NumPrimitives*m_VertexesPerPrimitive*sizeof(T2V3C4Vertex));
        glproc::UnmapBuffer(GL_ARRAY_BUFFER);
//        glproc::BufferSubData(GL_ARRAY_BUFFER, 0,
//                m_NumPrimitives*m_VertexesPerPrimitive*sizeof(T2V3C4Vertex),
//                m_pVertexData);
    }
    m_bDataChanged = false;
}

void VertexArray::draw()
{
    if (m_bDataChanged) {
        update();
    } else {
        glproc::BindBuffer(GL_ARRAY_BUFFER, m_VBOArrayID);
    }
    glTexCoordPointer(2, GL_FLOAT, sizeof(T2V3C4Vertex), 0);
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(T2V3C4Vertex), 
            (void *)(offsetof(T2V3C4Vertex, m_Color)));
    glVertexPointer(3, GL_FLOAT, sizeof(T2V3C4Vertex),
            (void *)(offsetof(T2V3C4Vertex, m_Pos)));

    if (m_VertexesPerPrimitive==3) {
        glDrawArrays(GL_TRIANGLES, 0, m_VertexesPerPrimitive*m_NumPrimitives);
    } else {
        glDrawArrays(GL_QUADS, 0, m_VertexesPerPrimitive*m_NumPrimitives);
    }
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "VertexArray::draw()");
}

void VertexArray::setBufferSize() 
{
    glproc::BindBuffer(GL_ARRAY_BUFFER, m_VBOArrayID);
    glproc::BufferData(GL_ARRAY_BUFFER, 
            m_ReservePrimitives*m_VertexesPerPrimitive*sizeof(T2V3C4Vertex), 0, 
            GL_STREAM_DRAW);
}

}

