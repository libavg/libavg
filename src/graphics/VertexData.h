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

#ifndef _VertexData_H_
#define _VertexData_H_

#include "../api.h"

#include "Pixel32.h"
#include "OGLHelper.h"

#include "../base/GLMHelper.h"

#include <boost/shared_ptr.hpp>

namespace avg {

struct Vertex {
    GLfloat m_Tex[2];
    GLfloat m_Pos[2];
    Pixel32 m_Color;
};

class VertexData;
typedef boost::shared_ptr<VertexData> VertexDataPtr;

#ifdef AVG_ENABLE_EGL
#define GL_INDEX_TYPE unsigned short 
#else
#define GL_INDEX_TYPE unsigned int 
#endif

class AVG_API VertexData {
public:
    VertexData(int reserveVerts = 0, int reserveIndexes = 0);
    virtual ~VertexData();

    void appendPos(const glm::vec2& pos, 
            const glm::vec2& texPos, const Pixel32& color = Pixel32(0,0,0,0));
    void appendTriIndexes(int v0, int v1, int v2);
    void appendQuadIndexes(int v0, int v1, int v2, int v3);
    void addLineData(Pixel32 color, const glm::vec2& p1, const glm::vec2& p2, 
            float width, float tc1=0, float tc2=1);
    void appendVertexData(const VertexDataPtr& pVertexes);
    bool hasDataChanged() const;
    void resetDataChanged();
    void reset();

    int getNumVerts() const;
    int getNumIndexes() const;
    void dump() const;
    void dump(unsigned startVertex, int numVerts, unsigned startIndex, int numIndexes) 
            const;

protected:
    int getReserveVerts() const;
    int getReserveIndexes() const;

    const Vertex * getVertexPointer() const;
    const GL_INDEX_TYPE * getIndexPointer() const;

    static const int MIN_VERTEXES;
    static const int MIN_INDEXES;

private:
    void grow();

    int m_NumVerts;
    int m_NumIndexes;
    int m_ReserveVerts;
    int m_ReserveIndexes;
    Vertex * m_pVertexData;
    GL_INDEX_TYPE * m_pIndexData;

    bool m_bDataChanged;
};

std::ostream& operator<<(std::ostream& os, const Vertex& v);

}

#endif
