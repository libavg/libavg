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

#ifndef _SubVertexArray_H_
#define _SubVertexArray_H_

#include "../api.h"

#include "VertexArray.h"
#include "Pixel32.h"
#include "OGLHelper.h"

#include "../base/GLMHelper.h"

#include <boost/shared_ptr.hpp>

namespace avg {

class AVG_API SubVertexArray {
public:
    SubVertexArray();
    ~SubVertexArray();
    void init(VertexArray* pVertexArray, unsigned startVertex, unsigned startIndex);

    void appendPos(const glm::vec2& pos, 
            const glm::vec2& texPos, const Pixel32& color = Pixel32(0,0,0,0));
    void appendTriIndexes(int v0, int v1, int v2);
    void appendQuadIndexes(int v0, int v1, int v2, int v3);
    void addLineData(Pixel32 color, const glm::vec2& p1, const glm::vec2& p2, 
            float width, float tc1=0, float tc2=1);
    void appendVertexData(VertexDataPtr pVertexes);
    int getNumVerts() const;

    void draw();
    void dump() const;

private:
    VertexArray* m_pVA;
        
    unsigned m_StartVertex;
    unsigned m_StartIndex;
    int m_NumVerts;
    int m_NumIndexes;
};

inline void SubVertexArray::appendPos(const glm::vec2& pos, 
        const glm::vec2& texPos, const Pixel32& color)
{
    m_pVA->appendPos(pos, texPos, color);
    m_NumVerts++;
}

typedef boost::shared_ptr<SubVertexArray> SubVertexArrayPtr;

}

#endif
