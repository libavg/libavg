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
#include "../base/WideLine.h"

#include <iostream>
#include <stddef.h>
#include <string.h>

using namespace std;
using namespace boost;

namespace avg {
    
thread_specific_ptr<vector<unsigned int> > VertexArray::s_pGLVertexBufferIDs;
thread_specific_ptr<vector<unsigned int> > VertexArray::s_pGLIndexBufferIDs;

VertexArray::VertexArray(int reserveVerts, int reserveIndexes)
    : m_NumVerts(0),
      m_NumIndexes(0),
      m_ReserveVerts(reserveVerts),
      m_ReserveIndexes(reserveIndexes),
      m_bDataChanged(true)
{
    if (m_ReserveVerts < 10) {
        m_ReserveVerts = 10;
    }
    if (m_ReserveIndexes < 20) {
        m_ReserveIndexes = 20;
    }
    m_pVertexData = new T2V3C4Vertex[m_ReserveVerts];
    m_pIndexData = new unsigned int[m_ReserveIndexes];

    initBufferCache();
    if (s_pGLVertexBufferIDs->empty() || m_ReserveVerts != 10 || 
            m_ReserveIndexes != 20)
    {
        glproc::GenBuffers(1, &m_GLVertexBufferID);
        glproc::GenBuffers(1, &m_GLIndexBufferID);
        setBufferSize();
    } else {
        m_GLVertexBufferID = s_pGLVertexBufferIDs->back();
        s_pGLVertexBufferIDs->pop_back();
        m_GLIndexBufferID = s_pGLIndexBufferIDs->back();
        s_pGLIndexBufferIDs->pop_back();
    }
}

VertexArray::~VertexArray()
{
    if (m_ReserveVerts == 10) {
        s_pGLVertexBufferIDs->push_back(m_GLVertexBufferID);
    } else {
        glproc::DeleteBuffers(1, &m_GLVertexBufferID);
    }
    if (m_ReserveIndexes == 20) {
        s_pGLIndexBufferIDs->push_back(m_GLIndexBufferID);
    } else {
        glproc::DeleteBuffers(1, &m_GLIndexBufferID);
    }
    delete[] m_pVertexData;
    delete[] m_pIndexData;
}

void VertexArray::appendPos(const DPoint& pos, 
        const DPoint& texPos, const Pixel32& color)
{
    if (m_NumVerts >= m_ReserveVerts-1) {
        grow();
    }
    T2V3C4Vertex* pVertex = &(m_pVertexData[m_NumVerts]);
    if (pVertex->m_Pos[0] != (GLfloat)pos.x || 
            pVertex->m_Pos[1] != (GLfloat)pos.y ||
            pVertex->m_Tex[0] != (GLfloat)texPos.x || 
            pVertex->m_Tex[1] != (GLfloat)texPos.y ||
            pVertex->m_Color != color)
    {
        T2V3C4Vertex* pVertex = &m_pVertexData[m_NumVerts];
        pVertex->m_Pos[0] = (GLfloat)pos.x;
        pVertex->m_Pos[1] = (GLfloat)pos.y;
        pVertex->m_Pos[2] = 0.0;
        pVertex->m_Tex[0] = (GLfloat)texPos.x;
        pVertex->m_Tex[1] = (GLfloat)texPos.y;
        pVertex->m_Color = color;
        m_bDataChanged = true;
    }
    m_NumVerts++;
}

void VertexArray::appendTriIndexes(int v0, int v1, int v2)
{
    if (m_NumIndexes >= m_ReserveIndexes-3) {
        grow();
    }
    m_pIndexData[m_NumIndexes] = v0;
    m_pIndexData[m_NumIndexes+1] = v1;
    m_pIndexData[m_NumIndexes+2] = v2;
    m_NumIndexes += 3;
}

void VertexArray::appendQuadIndexes(int v0, int v1, int v2, int v3)
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

void VertexArray::addLineData(Pixel32 color, const DPoint& p1, const DPoint& p2, 
        double width, double TC1, double TC2)
{
    WideLine wl(p1, p2, width);
    int curVertex = getCurVert();
    appendPos(wl.pl0, DPoint(TC1, 1), color);
    appendPos(wl.pr0, DPoint(TC1, 0), color);
    appendPos(wl.pl1, DPoint(TC2, 1), color);
    appendPos(wl.pr1, DPoint(TC2, 0), color);
    appendQuadIndexes(curVertex+1, curVertex, curVertex+3, curVertex+2); 
}

void VertexArray::reset()
{
    m_NumVerts = 0;
    m_NumIndexes = 0;
}

void VertexArray::update()
{
    if (m_bDataChanged) {
        glproc::BindBuffer(GL_ARRAY_BUFFER, m_GLVertexBufferID);
        glproc::BufferSubData(GL_ARRAY_BUFFER, 0, m_NumVerts*sizeof(T2V3C4Vertex),
                m_pVertexData);
        glproc::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_GLIndexBufferID);
        glproc::BufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, m_NumIndexes*sizeof(unsigned int),
                m_pIndexData);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "VertexArray::update");
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

int VertexArray::getCurVert() const
{
    return m_NumVerts;
}

int VertexArray::getCurIndex() const
{
    return m_NumIndexes;
}

void VertexArray::grow()
{
    if (m_NumVerts >= m_ReserveVerts-1) {
        int oldReserveVerts = m_ReserveVerts;
        m_ReserveVerts = int(m_ReserveVerts*1.5);
        if (m_ReserveVerts < m_NumVerts) {
            m_ReserveVerts = m_NumVerts;
        }
        T2V3C4Vertex* pVertexData = m_pVertexData;
        m_pVertexData = new T2V3C4Vertex[m_ReserveVerts];
        memcpy(m_pVertexData, pVertexData, sizeof(T2V3C4Vertex)*oldReserveVerts);
        delete[] pVertexData;
    }
    if (m_NumIndexes >= m_ReserveIndexes-6) {
        int oldReserveIndexes = m_ReserveIndexes;
        m_ReserveIndexes = int(m_ReserveIndexes*1.5);
        if (m_ReserveIndexes < m_NumIndexes) {
            m_ReserveIndexes = m_NumIndexes;
        }
        unsigned int * pIndexData = m_pIndexData;
        m_pIndexData = new unsigned int[m_ReserveIndexes];
        memcpy(m_pIndexData, pIndexData, sizeof(unsigned int)*oldReserveIndexes);
        delete[] pIndexData;
    }
    setBufferSize();
    m_bDataChanged = true;
}

void VertexArray::setBufferSize() 
{
    glproc::BindBuffer(GL_ARRAY_BUFFER, m_GLVertexBufferID);
    glproc::BufferData(GL_ARRAY_BUFFER, m_ReserveVerts*sizeof(T2V3C4Vertex), 0, 
            GL_STREAM_DRAW);
    glproc::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_GLIndexBufferID);
    glproc::BufferData(GL_ELEMENT_ARRAY_BUFFER, 
            m_ReserveIndexes*sizeof(unsigned int), 0, GL_STREAM_DRAW);
}

void VertexArray::initBufferCache()
{
    if (s_pGLVertexBufferIDs.get() == 0) {
        s_pGLVertexBufferIDs.reset(new vector<unsigned int>);
    }
    if (s_pGLIndexBufferIDs.get() == 0) {
        s_pGLIndexBufferIDs.reset(new vector<unsigned int>);
    }
}

void VertexArray::deleteBufferCache()
{
    if (s_pGLVertexBufferIDs.get() != 0) {
        for (unsigned i=0; i<s_pGLVertexBufferIDs->size(); ++i) {
            glproc::DeleteBuffers(1, &((*s_pGLVertexBufferIDs)[i]));
        }
        s_pGLVertexBufferIDs->clear();
    }
    if (s_pGLIndexBufferIDs.get() != 0) {
        for (unsigned i=0; i<s_pGLIndexBufferIDs->size(); ++i) {
            glproc::DeleteBuffers(1, &((*s_pGLIndexBufferIDs)[i]));
        }
        s_pGLIndexBufferIDs->clear();
    }
}

}

