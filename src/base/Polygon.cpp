// Original code by John W. Ratcliff presumed to be in the public domain. Found 
// at http://www.flipcode.com/archives/Efficient_Polygon_Triangulation.shtml. 

#include "Polygon.h"
#include "Exception.h"

#include "../tess/tesselator.h"
#include "Exception.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

namespace avg {

using namespace std;

Polygon::Polygon()
{
}

Polygon::Polygon(const Vec2Vector& pts)
    : m_Pts(pts)
{
}

const Vec2Vector& Polygon::getPts() const
{
    return m_Pts;
}

float Polygon::getArea()
{
    int n = m_Pts.size();

    float A = 0.0;

    for (int p=n-1,q=0; q<n; p=q++) {
        A += m_Pts[p].x*m_Pts[q].y - m_Pts[q].x*m_Pts[p].y;
    }
    return A*0.5f;
}

void Polygon::triangulate(Vec2Vector& resultVertexes, vector<int>& resultIndexes)
{
    TESStesselator* pTess = tessNewTess(0);

    tessAddContour(pTess, 2, (void*)&(m_Pts[0]), sizeof(m_Pts[0]), m_Pts.size());
    tessTesselate(pTess, TESS_WINDING_NONZERO, TESS_POLYGONS, 3, 2, 0);

    resultVertexes.clear();
    resultIndexes.clear();
    int nVerts = tessGetVertexCount(pTess);
    const float* pVerts = tessGetVertices(pTess);
    for (int i=0; i<nVerts; ++i) {
        resultVertexes.push_back(glm::vec2(pVerts[i*2], pVerts[i*2+1]));
    }
    const int* pTriIndexes = tessGetElements(pTess);
    // We've limited polygon size to 3, so each "Element" is a triangle.
    for (int i=0; i<tessGetElementCount(pTess)*3; ++i) {
        resultIndexes.push_back(pTriIndexes[i]);
    }

    tessDeleteTess(pTess);
}

}

