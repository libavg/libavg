// Original code by John W. Ratcliff presumed to be in the public domain. Found 
// at http://www.flipcode.com/archives/Efficient_Polygon_Triangulation.shtml. 

#include "Polygon.h"
#include "Exception.h"

#include "../tess/glutess.h"
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

void Polygon::triangulate(std::vector<int>& resultIndexes, Vec2Vector& extraPts)
{
    m_pIndexes = &resultIndexes;
    m_pIndexes->clear();
    m_pExtraPts = &extraPts;

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

/*
    gluTessCallback(pTess, GLU_TESS_ERROR,   errorCB);
*/
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
}

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

}

