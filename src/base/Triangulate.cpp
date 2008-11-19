// Original code by John W. Ratcliff presumed to be in the public domain. Found 
// at http://www.flipcode.com/archives/Efficient_Polygon_Triangulation.shtml. 

#include "Triangulate.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <iostream>

namespace avg {

using namespace std;

double getPolygonArea(const DPointVector &contour)
{

  int n = contour.size();

  double A=0.0;

  for (int p=n-1,q=0; q<n; p=q++) {
    A += contour[p].x*contour[q].y - contour[q].x*contour[p].y;
  }
  return A*0.5;
}


bool Snip(const DPointVector &contour,int u,int v,int w,int n,int *V)
{
    int p;
    Triangle tri;
    tri.p0 = contour[V[u]];
    tri.p1 = contour[V[v]];
    tri.p2 = contour[V[w]];

    if ( EPSILON > (((tri.p1.x-tri.p0.x)*(tri.p2.y-tri.p0.y)) - 
            ((tri.p1.y-tri.p0.y)*(tri.p2.x-tri.p0.x))) ) 
    {
        return false;
    }

    for (p=0; p<n; p++) {
        if( (p == u) || (p == v) || (p == w) ) {
            continue;
        }
        if (tri.isInside(contour[V[p]])) {
            return false;
        }
    }

    return true;
}

void triangulatePolygon(const DPointVector &contour, TriangleVector &result)
{
    /* allocate and initialize list of Vertices in polygon */

    int n = contour.size();
    assert(n>2);

    int *V = new int[n];

    /* we want a counter-clockwise polygon in V */

    if (0.0 < getPolygonArea(contour)) {
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
        assert(count>0);  // Bad (non-simple) input polygon.
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

        cerr << u << ", " << v << ", " << w << endl;
        if ( Snip(contour,u,v,w,nv,V) )
        {
            cerr << "snip" << endl;
            int a,b,c,s,t;

            /* true names of the vertices */
            a = V[u]; b = V[v]; c = V[w];

            /* output Triangle */

            result.push_back(Triangle(contour[a], contour[b], contour[c]));

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

    delete V;
}

}

