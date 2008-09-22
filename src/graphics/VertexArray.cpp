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

    VertexArray::VertexArray(int NumQuads)
        : m_NumQuads(NumQuads),
          m_bDataChanged(true)
    {
        glproc::GenBuffers(1, &m_VBOArrayID);
        m_pVertexData = new T2V3C4Vertex[NumQuads*4];
        glproc::BindBuffer(GL_ARRAY_BUFFER, m_VBOArrayID);
        glproc::BufferData(GL_ARRAY_BUFFER, m_NumQuads*4*sizeof(T2V3C4Vertex), 0, 
                GL_STREAM_DRAW);

    }

    VertexArray::~VertexArray()
    {
        delete[] m_pVertexData;
        glproc::DeleteBuffers(1, &m_VBOArrayID);
    }

    void VertexArray::setPos(int quadIndex, int vertexIndex, const DPoint& pos, 
            const DPoint& texPos, const Pixel32& color)
    {
        assert(quadIndex < m_NumQuads);
        T2V3C4Vertex* pVertex = &(m_pVertexData[quadIndex*4+vertexIndex]);
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

    void VertexArray::update()
    {
        // TODO: In case of performance issues, start using glMapBuffer.
        if (m_bDataChanged) {
            glproc::BindBuffer(GL_ARRAY_BUFFER, m_VBOArrayID);
            glproc::BufferSubData(GL_ARRAY_BUFFER, 0, m_NumQuads*4*sizeof(T2V3C4Vertex),
                    m_pVertexData);
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
        glVertexPointer(3, GL_FLOAT, sizeof(T2V3C4Vertex),
                (void *)(offsetof(T2V3C4Vertex, m_Pos)));
        glTexCoordPointer(2, GL_FLOAT, sizeof(T2V3C4Vertex), 0);
        glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(T2V3C4Vertex), 0);
        
        glDrawArrays(GL_QUADS, 0, 4*m_NumQuads);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "VertexArray::draw()");
    }

}

