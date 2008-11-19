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
    : VertexData(numVerts, numIndexes, reserveVerts, reserveIndexes),
      m_bDataChanged(true)
{
    glproc::GenBuffers(1, &m_GLVertexBufferID);
    glproc::GenBuffers(1, &m_GLIndexBufferID);
    setBufferSize();
}

VertexArray::~VertexArray()
{
    glproc::DeleteBuffers(1, &m_GLVertexBufferID);
    glproc::DeleteBuffers(1, &m_GLIndexBufferID);
}

void VertexArray::setPos(int vertexIndex, const DPoint& pos, 
        const DPoint& texPos, const Pixel32& color)
{
    assert(vertexIndex < getNumVerts());
    T2V3C4Vertex* pVertex = &(getVertexData()[vertexIndex]);
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

void VertexArray::setVertexData(int vertexIndex, int indexIndex, 
        const VertexDataPtr& pVertexes)
{
    VertexData::setVertexData(vertexIndex, indexIndex, pVertexes);
    m_bDataChanged = true;
}

void VertexArray::changeSize(int numVerts, int numIndexes)
{
    int oldReserveVerts = getReservedVerts();
    int oldReserveIndexes = getReservedIndexes();
    VertexData::changeSize(numVerts, numIndexes);
    if (oldReserveVerts != getReservedVerts() || 
                oldReserveIndexes != getReservedIndexes())
    {
        setBufferSize();
    }
    m_bDataChanged = true;
}

void VertexArray::update()
{
    if (m_bDataChanged) {
        glproc::BindBuffer(GL_ARRAY_BUFFER, m_GLVertexBufferID);
        glproc::BufferData(GL_ARRAY_BUFFER, getReservedVerts()*sizeof(T2V3C4Vertex), 0, 
                GL_STREAM_DRAW);
        void * pOGLBuffer = glproc::MapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        memcpy(pOGLBuffer, getVertexData(), getNumVerts()*sizeof(T2V3C4Vertex));
        glproc::UnmapBuffer(GL_ARRAY_BUFFER);
        glproc::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_GLIndexBufferID);
        glproc::BufferData(GL_ELEMENT_ARRAY_BUFFER, 
                getReservedIndexes()*sizeof(unsigned int), 0, GL_STREAM_DRAW);
        pOGLBuffer = glproc::MapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
        memcpy(pOGLBuffer, getIndexData(), getNumIndexes()*sizeof(unsigned int));
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
    glDrawElements(GL_TRIANGLES, getNumIndexes(), GL_UNSIGNED_INT, 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "VertexArray::draw():2");
}

void VertexArray::setBufferSize() 
{
    glproc::BindBuffer(GL_ARRAY_BUFFER, m_GLVertexBufferID);
    glproc::BufferData(GL_ARRAY_BUFFER, getReservedVerts()*sizeof(T2V3C4Vertex), 0, 
            GL_STREAM_DRAW);
    glproc::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_GLVertexBufferID);
    glproc::BufferData(GL_ELEMENT_ARRAY_BUFFER, 
            getReservedIndexes()*sizeof(unsigned int), 0, GL_STREAM_DRAW);
}

}

