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

#include "OGLHelper.h"
#include "../base/Point.h"

namespace avg {

struct T2V3Vertex {
    GLfloat m_Tex[2];
    GLfloat m_Pos[3];
    GLfloat m_Dummy[3];
};

class VertexArray {
public:
    VertexArray(int NumQuads);
    virtual ~VertexArray();

    void setPos(int QuadIndex, int VertexIndex, const DPoint& Pos, 
            const DPoint& TexPos);
    void draw();

private:
    int m_NumQuads;
    T2V3Vertex * m_pVertexData;
    bool m_bDataChanged;

    unsigned int m_VBOArrayID;
};

}
