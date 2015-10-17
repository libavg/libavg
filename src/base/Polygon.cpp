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
    const int* pElems = tessGetElements(pTess);
    // We've limited polygon size to 3, so each "Element" is a triangle.
    for (int i=0; i<tessGetElementCount(pTess)*3; ++i) {
        resultIndexes.push_back(pTriIndexes[i]);
    }

    tessDeleteTess(pTess);
/*
    m_pIndexes = &resultIndexes;
    m_pIndexes->clear();
    m_pExtraPts = &extraPts;
    m_pExtraPts->clear();

    vector<glm::dvec3> coords;
    for (unsigned i=0; i<m_Pts.size(); ++i) {
        coords.push_back(glm::dvec3(m_Pts[i].x, m_Pts[i].y, 0));
    }

    // create tessellator
    GLUtesselator *pTess = gluNewTess();
    gluTessCallback(pTess, GLU_TESS_VERTEX_DATA, (_GLUfuncptr)Polygon::vertexCallback);
    gluTessCallback(pTess, GLU_TESS_COMBINE_DATA, (_GLUfuncptr)Polygon::combineCallback);
    gluTessCallback(pTess, GLU_TESS_EDGE_FLAG, (_GLUfuncptr)Polygon::edgeCallback);
    gluTessNormal(pTess, 0.0, 0.0, 1.0 );
    gluTessProperty(pTess, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_NONZERO);

    // describe non-convex polygon
    gluTessBeginPolygon(pTess, this);
    // first contour
    gluTessBeginContour(pTess);
    for (size_t i=0; i<coords.size(); ++i) {
        gluTessVertex(pTess, glm::value_ptr(coords[i]), (void*)i);
    }

    gluTessEndContour(pTess);
    gluTessEndPolygon(pTess);

    gluDeleteTess(pTess);
*/
}

/*
void Polygon::edgeCallback(bool bEdge)
{
    // Only exists to prevent the tesselator from returning fans or strips.
}

void Polygon::vertexCallback(void* pVertexData, void* pPolygonData)
{
    Polygon* pThis = (Polygon *)pPolygonData;
    size_t i = (size_t)pVertexData;
    pThis->m_pIndexes->push_back(i);
}

void Polygon::combineCallback(double coords[3], void *vertex_data[4], 
        float weight[4], void **ppOutData, void *pPolygonData)
{
    Polygon* pThis = (Polygon *)pPolygonData;
    pThis->m_pExtraPts->push_back(glm::vec2(coords[0], coords[1]));
    size_t numPts = pThis->m_Pts.size() + pThis->m_pExtraPts->size() - 1;
    *ppOutData = (void*)numPts;
}
*/
}

