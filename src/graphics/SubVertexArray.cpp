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

#include "SubVertexArray.h"

#include "VertexArray.h"
#include "GLContext.h"

#include "../base/Exception.h"
#include "../base/WideLine.h"
#include "../base/ObjectCounter.h"

#include <iostream>
#include <stddef.h>
#include <string.h>

using namespace std;
using namespace boost;

namespace avg {

SubVertexArray::SubVertexArray()
{
}

SubVertexArray::~SubVertexArray()
{
}

void SubVertexArray::init(VertexArray* pVertexArray, unsigned startVertex,
        unsigned startIndex)
{
    m_pVA = pVertexArray;
    m_StartVertex = startVertex;
    m_StartIndex = startIndex;
    m_NumVerts = 0;
    m_NumIndexes = 0;
}

void SubVertexArray::appendTriIndexes(int v0, int v1, int v2)
{
    m_pVA->appendTriIndexes(v0+m_StartVertex, v1+m_StartVertex, v2+m_StartVertex);
    m_NumIndexes += 3;
}

void SubVertexArray::appendQuadIndexes(int v0, int v1, int v2, int v3)
{
    m_pVA->appendQuadIndexes(v0+m_StartVertex, v1+m_StartVertex, v2+m_StartVertex, 
            v3+m_StartVertex);
    m_NumIndexes += 6;
}

void SubVertexArray::addLineData(Pixel32 color, const glm::vec2& p1, const glm::vec2& p2, 
        float width, float tc1, float tc2)
{
    m_pVA->addLineData(color, p1, p2, width, tc1, tc2);
    m_NumIndexes += 6;
}

void SubVertexArray::appendVertexData(VertexDataPtr pVertexes)
{
    m_pVA->appendVertexData(pVertexes);
    m_NumVerts += pVertexes->getNumVerts();
    m_NumIndexes += pVertexes->getNumIndexes();
}

int SubVertexArray::getNumVerts() const
{
    return m_NumVerts;
}

void SubVertexArray::draw()
{
    m_pVA->draw(m_StartIndex, m_NumIndexes, m_StartVertex, m_StartIndex);
}

void SubVertexArray::dump() const
{
    cerr << "SubVertexArray: m_StartVertex=" << m_StartVertex << ", " 
            << ", m_StartIndex=" << m_StartIndex << endl;
    m_pVA->dump(m_StartVertex, m_NumVerts, m_StartIndex, m_NumIndexes);
}

}

