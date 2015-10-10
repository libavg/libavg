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

bool snip(const Vec2Vector &pts,int u,int v,int w,int n,int *V)
{
    int p;
    Triangle tri;
    tri.p0 = pts[V[u]];
    tri.p1 = pts[V[v]];
    tri.p2 = pts[V[w]];

//    float area = tri.getArea();

    if (tri.isClockwise()) {
        return false;
    }

    for (p=0; p<n; p++) {
        if( (p == u) || (p == v) || (p == w) ) {
            continue;
        }
        if (tri.isInside(pts[V[p]])) {
            return false;
        }
    }

    return true;
}

void Polygon::triangulate(std::vector<int>& resultIndexes)
{
    /* allocate and initialize list of Vertices in polygon */

    int n = m_Pts.size();
    AVG_ASSERT(n>2);

    int *V = new int[n];

    // we want a counter-clockwise polygon in V. 
    if (0.0 < getArea()) {
        for (int v=0; v<n; v++) {
            V[v] = v;
        }
    } else {
        for(int v=0; v<n; v++) {
            V[v] = (n-1)-v;
        }
    }

    int nv = n;

    /*  remove nv-2 Vertices, creating 1 triangle every time */
    int count = 2*nv;   /* error detection */

    for(int m=0, v=nv-1; nv>2; )
    {
        if (count <= 0) {
            delete V;
            throw Exception(AVG_ERR_INVALID_ARGS, 
                    "Non-simple polygon: Self-intersecting polygons or degenerate polygons are not supported.");
        }
        count--;

        /* three consecutive vertices in current polygon, <u,v,w> */
        int u = v; 
        if (nv <= u) { 
            u = 0;     /* previous */
        }
        v = u+1;
        if (nv <= v) {
            v = 0;
        }
        int w = v+1; 
        if (nv <= w) {
            w = 0;
        }

        if (snip(m_Pts,u,v,w,nv,V))
        {
            int a,b,c,s,t;

            /* true names of the vertices */
            a = V[u]; b = V[v]; c = V[w];

            /* output Triangle */

            resultIndexes.push_back(a);
            resultIndexes.push_back(b);
            resultIndexes.push_back(c);

            m++;

            /* remove v from remaining polygon */
            for(s=v,t=v+1; t<nv; s++,t++) {
                V[s] = V[t]; 
            }
            nv--;

            /* resest error detection counter */
            count = 2*nv;

        }
    }

    delete[] V;
}

}

