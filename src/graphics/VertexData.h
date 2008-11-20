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

#ifndef _VertexData_H_
#define _VertexData_H_

#include "../api.h"
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


class AVG_API VertexData;
typedef boost::shared_ptr<VertexData> VertexDataPtr;

class AVG_API VertexData {
public:
    VertexData(int numVerts=0, int numIndexes=0, int reserveVerts=10, 
            int reserveIndexes=20);
    virtual ~VertexData();

    virtual void setPos(int vertexIndex, const DPoint& pos, 
            const DPoint& texPos, const Pixel32& color = Pixel32(0,0,0,0));
    void setIndex(int i, int vertexIndex);
    void setTriIndexes(int i, int v0, int v1, int v2);
    virtual void setVertexData(int vertexIndex, int indexIndex, 
        const VertexDataPtr& pVertexes);
    virtual void changeSize(int numVerts, int numIndexes);

    int getNumVerts() const;
    int getNumIndexes() const;
    const T2V3C4Vertex * getVertexData() const;
    const unsigned int * getIndexData() const;
    void dump() const;

protected:
    int getReservedVerts() const;
    int getReservedIndexes() const;
    T2V3C4Vertex* getVertexData();
    unsigned int* getIndexData();

private:
    int m_NumVerts;
    int m_NumIndexes;
    int m_ReserveVerts;
    int m_ReserveIndexes;
    T2V3C4Vertex * m_pVertexData;
    unsigned int * m_pIndexData;
};

}

#endif
