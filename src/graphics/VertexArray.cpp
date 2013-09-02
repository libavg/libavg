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

#include "VertexArray.h"

#include "GLContext.h"
#include "SubVertexArray.h"

#include "../base/Exception.h"
#include "../base/WideLine.h"
#include "../base/ObjectCounter.h"

#include <iostream>
#include <stddef.h>
#include <string.h>

using namespace std;
using namespace boost;

namespace avg {

const unsigned VertexArray::TEX_INDEX = 0;
const unsigned VertexArray::POS_INDEX = 1;
const unsigned VertexArray::COLOR_INDEX = 2;

VertexArray::VertexArray(int reserveVerts, int reserveIndexes)
    : VertexData(reserveVerts, reserveIndexes)
{
    GLContext* pContext = GLContext::getCurrent();
    if (getReserveVerts() != MIN_VERTEXES || getReserveIndexes() != MIN_INDEXES) {
        glproc::GenBuffers(1, &m_GLVertexBufferID);
        glproc::GenBuffers(1, &m_GLIndexBufferID);
    } else {
        m_GLVertexBufferID = pContext->getVertexBufferCache().getBuffer();
        m_GLIndexBufferID = pContext->getIndexBufferCache().getBuffer();
    }
    m_bUseMapBuffer = (!pContext->isGLES());
}

VertexArray::~VertexArray()
{
    GLContext* pContext = GLContext::getCurrent();
    if (pContext) {
        if (getReserveVerts() == MIN_VERTEXES) {
            pContext->getVertexBufferCache().returnBuffer(m_GLVertexBufferID);
        } else {
            glproc::DeleteBuffers(1, &m_GLVertexBufferID);
        }
        if (getReserveIndexes() == MIN_INDEXES) {
            pContext->getIndexBufferCache().returnBuffer(m_GLIndexBufferID);
        } else {
            glproc::DeleteBuffers(1, &m_GLIndexBufferID);
        }
    }
}

void VertexArray::update()
{
    if (hasDataChanged()) {
        transferBuffer(GL_ARRAY_BUFFER, m_GLVertexBufferID, 
                getReserveVerts()*sizeof(Vertex), 
                getNumVerts()*sizeof(Vertex), getVertexPointer());
#ifdef AVG_ENABLE_EGL        
        transferBuffer(GL_ELEMENT_ARRAY_BUFFER, m_GLIndexBufferID, 
                getReserveIndexes()*sizeof(unsigned short),
                getNumIndexes()*sizeof(unsigned short), getIndexPointer());
#else
        transferBuffer(GL_ELEMENT_ARRAY_BUFFER, m_GLIndexBufferID, 
                getReserveIndexes()*sizeof(unsigned int),
                getNumIndexes()*sizeof(unsigned int), getIndexPointer());
#endif
        GLContext::checkError("VertexArray::update()");
    }
    resetDataChanged();
}

void VertexArray::activate()
{
    glproc::BindBuffer(GL_ARRAY_BUFFER, m_GLVertexBufferID);
    glproc::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_GLIndexBufferID);
    glproc::VertexAttribPointer(TEX_INDEX, 2, GL_SHORT, GL_FALSE,
            sizeof(Vertex), (void *)(offsetof(Vertex, m_Tex)));
    glproc::VertexAttribPointer(POS_INDEX, 2, GL_FLOAT, GL_FALSE, 
            sizeof(Vertex), (void *)(offsetof(Vertex, m_Pos)));
    glproc::VertexAttribPointer(COLOR_INDEX, 4, GL_UNSIGNED_BYTE, GL_TRUE, 
            sizeof(Vertex), (void *)(offsetof(Vertex, m_Color)));
    glproc::EnableVertexAttribArray(TEX_INDEX);
    glproc::EnableVertexAttribArray(POS_INDEX);
    glproc::EnableVertexAttribArray(COLOR_INDEX);
    GLContext::checkError("VertexArray::activate()");
}

void VertexArray::draw()
{
    update();
    activate();
#ifdef AVG_ENABLE_EGL        
    glDrawElements(GL_TRIANGLES, getNumIndexes(), GL_UNSIGNED_SHORT, 0);
#else
    glDrawElements(GL_TRIANGLES, getNumIndexes(), GL_UNSIGNED_INT, 0);
#endif
    GLContext::checkError("VertexArray::draw()");
}

void VertexArray::draw(unsigned startIndex, unsigned numIndexes, unsigned startVertex,
        unsigned numVertexes)
{
#ifdef AVG_ENABLE_EGL        
    glDrawElements(GL_TRIANGLES, numIndexes, GL_UNSIGNED_SHORT, 
            (void *)(startIndex*sizeof(unsigned short)));
#else
    glDrawElements(GL_TRIANGLES, numIndexes, GL_UNSIGNED_INT, 
            (void *)(startIndex*sizeof(unsigned int)));
#endif
//    XXX: Theoretically faster, but broken on Linux/Intel N10 graphics, Ubuntu 12/04
//    glproc::DrawRangeElements(GL_TRIANGLES, startVertex, startVertex+numVertexes, 
//            numIndexes, GL_UNSIGNED_SHORT, (void *)(startIndex*sizeof(unsigned short)));
    GLContext::checkError("VertexArray::draw()");
}

void VertexArray::startSubVA(SubVertexArray& subVA)
{
    subVA.init(this, getNumVerts(), getNumIndexes());
}

void VertexArray::transferBuffer(GLenum target, unsigned bufferID, unsigned reservedSize, 
        unsigned usedSize, const void* pData)
{
    glproc::BindBuffer(target, bufferID);
    if (m_bUseMapBuffer) {
        glproc::BufferData(target, reservedSize, 0, GL_STREAM_DRAW);
        void * pBuffer = glproc::MapBuffer(target, GL_WRITE_ONLY);
        memcpy(pBuffer, pData, usedSize);
        glproc::UnmapBuffer(target);
    } else {
        glproc::BufferData(target, usedSize, pData, GL_STREAM_DRAW);
    }
}

}

