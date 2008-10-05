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

#ifndef _VertexArray_H_
#define _VertexArray_H_

#include "../base/Point.h"
#include "../graphics/Pixel32.h"
#include "../graphics/OGLHelper.h"

#include <boost/shared_ptr.hpp>

namespace avg {

struct T2V3C4Vertex {
    GLfloat m_Tex[2];
    Pixel32 m_Color;
    GLfloat m_Pos[3];
};

class VertexArray {
public:
    VertexArray(int numVerts, int numIndexes, int reserveVerts = 0, 
            int reserveIndexes = 0);
    virtual ~VertexArray();

    void setPos(int vertexIndex, const DPoint& pos, 
            const DPoint& texPos, const Pixel32& color = Pixel32(0,0,0,0));
    void setIndex(int i, int vertexIndex);
    void changeSize(int numVerts, int numIndexes);

    void update();
    void draw();

private:
    void setBufferSize();

    int m_NumVerts;
    int m_ReserveVerts;
    int m_NumIndexes;
    int m_ReserveIndexes;
    T2V3C4Vertex * m_pVertexData;
    unsigned int * m_pIndexData;
    bool m_bDataChanged;

    unsigned int m_GLVertexBufferID;
    unsigned int m_GLIndexBufferID;
};

typedef boost::shared_ptr<VertexArray> VertexArrayPtr;

}

#endif
