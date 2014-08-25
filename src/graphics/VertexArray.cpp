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

#include "VertexArray.h"

#include "GLContext.h"
#include "GLContextManager.h"
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
    m_bUseMapBuffer = (!pContext->isGLES());
}

void VertexArray::initForGLContext()
{
    GLContext* pContext = GLContext::getCurrent();
    unsigned vertexBufferID;
    unsigned indexBufferID;
    AVG_ASSERT(m_VertexBufferIDMap.count(pContext) == 0);
    AVG_ASSERT(m_IndexBufferIDMap.count(pContext) == 0);
    glproc::GenBuffers(1, &vertexBufferID);
    m_VertexBufferIDMap[pContext] = vertexBufferID;
    glproc::GenBuffers(1, &indexBufferID);
    m_IndexBufferIDMap[pContext] = indexBufferID;
}

VertexArray::~VertexArray()
{
    GLContextManager::get()->deleteBuffers(m_VertexBufferIDMap);
    GLContextManager::get()->deleteBuffers(m_IndexBufferIDMap);
}

void VertexArray::update()
{
    AVG_ASSERT(!m_VertexBufferIDMap.empty());
    if (hasDataChanged()) {
        GLContext* pContext = GLContext::getCurrent();
        unsigned vertexBufferID = m_VertexBufferIDMap[pContext];
        transferBuffer(GL_ARRAY_BUFFER, vertexBufferID, 
                getReserveVerts()*sizeof(Vertex), 
                getNumVerts()*sizeof(Vertex), getVertexPointer());
        unsigned indexBufferID = m_IndexBufferIDMap[pContext];
#ifdef AVG_ENABLE_EGL        
        transferBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID, 
                getReserveIndexes()*sizeof(unsigned short),
                getNumIndexes()*sizeof(unsigned short), getIndexPointer());
#else
        transferBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID, 
                getReserveIndexes()*sizeof(unsigned int),
                getNumIndexes()*sizeof(unsigned int), getIndexPointer());
#endif
        GLContext::checkError("VertexArray::update()");
    }
}

void VertexArray::activate()
{
    AVG_ASSERT(!m_VertexBufferIDMap.empty());
    GLContext* pContext = GLContext::getCurrent();
    unsigned vertexBufferID = m_VertexBufferIDMap[pContext];
    unsigned indexBufferID = m_IndexBufferIDMap[pContext];
    glproc::BindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glproc::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
    glproc::VertexAttribPointer(TEX_INDEX, 2, GL_FLOAT, GL_FALSE,
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

