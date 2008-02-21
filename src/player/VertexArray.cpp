//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
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

namespace avg {

    VertexArray::VertexArray(int NumQuads)
        : m_NumQuads(NumQuads),
          m_bDataChanged(true)
    {
        glproc::GenBuffers(1, &m_VBOArrayID);
        m_pVertexData = new T2V3Vertex[NumQuads*4];
    }

    VertexArray::~VertexArray()
    {
        delete[] m_pVertexData;
        glproc::DeleteBuffers(1, &m_VBOArrayID);
    }

    void VertexArray::setPos(int QuadIndex, int VertexIndex, const DPoint& Pos, 
            const IntPoint TexPos)
    {
        assert(QuadIndex < m_NumQuads);
        T2V3Vertex* pVertex = &(m_pVertexData[QuadIndex*4+VertexIndex]);
        pVertex->m_Pos[0] = Pos.x;
        pVertex->m_Pos[1] = Pos.y;
        pVertex->m_Pos[2] = 0.0;
        pVertex->m_Tex[0] = TexPos.x;
        pVertex->m_Tex[1] = TexPos.y;
        m_bDataChanged = true;
    }

    void VertexArray::draw()
    {
        glproc::BindBuffer(GL_ARRAY_BUFFER, m_VBOArrayID);
        if (m_bDataChanged) {
            glproc::BufferData(GL_ARRAY_BUFFER, m_NumQuads*4*sizeof(T2V3Vertex),
                    m_pVertexData, GL_STREAM_DRAW);
        }
        glInterleavedArrays(GL_T2F_V3F, sizeof(T2V3Vertex), 0);
        
        glDrawArrays(GL_QUADS, 0, 4);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "VertexArray::draw()");

        m_bDataChanged = false;
    }

}

