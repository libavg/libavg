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

#include "GeomHelper.h"

#include <math.h>
#include <iostream>

using namespace std;

namespace avg {

DLineSegment::DLineSegment(const DPoint& pt0, const DPoint& pt1)
    : p0(pt0),
      p1(pt1)
{
}

bool DLineSegment::isPointOver(const DPoint& pt)
{
    DPoint c = pt - p0;   // DPoint from a to Point
    DPoint v = (p1 - p0);
    double d = v.getNorm(); // Length of the line segment
    v /= d; // Unit DPoint from a to b
    double t = dotProduct(v, c);  // Intersection point Distance from a

    return (t >= 0 && t <= d);
}

// Code adapted from Antonio, Franklin, "Faster Line Segment Intersection,"
// Graphics Gems III (David Kirk, ed.), Academic Press, pp. 199-202, 1992.
bool lineSegmentsIntersect(const DLineSegment& l0, const DLineSegment& l1)
{
    double xdiff0 = l0.p1.x-l0.p0.x;
    double xdiff1 = l1.p0.x-l1.p1.x;
    
    double x1lo, x1hi;

    /* X bound box test*/
    if (xdiff0 < 0) {
        x1lo=l0.p1.x; 
        x1hi=l0.p0.x;
    } else {
        x1hi=l0.p1.x; 
        x1lo=l0.p0.x;
    }
    if (xdiff1 > 0) {
        if (x1hi < l1.p1.x || l1.p0.x < x1lo) {
            return false;
        }
    } else {
        if (x1hi < l1.p0.x || l1.p1.x < x1lo) {
            return false;
        }
    }

    double ydiff0 = l0.p1.y-l0.p0.y;
    double ydiff1 = l1.p0.y-l1.p1.y;

    double y1lo, y1hi;

    /* Y bound box test*/
    if (ydiff0 < 0) {
        y1lo=l0.p1.y; 
        y1hi=l0.p0.y;
    } else {
        y1hi=l0.p1.y; 
        y1lo=l0.p0.y;
    }
    if (ydiff1 > 0) {
        if (y1hi < l1.p1.y || l1.p0.y < y1lo) {
            return false;
        }
    } else {
        if (y1hi < l1.p0.y || l1.p1.y < y1lo) {
            return false;
        }
    }

    double Cx = l0.p0.x-l1.p0.x;
    double Cy = l0.p0.y-l1.p0.y;
    double d = ydiff1*Cx - xdiff1*Cy;                  /* alpha numerator*/
    double f = ydiff0*xdiff1 - xdiff0*ydiff1;          /* both denominator*/
    if (f > 0) {                                       /* alpha tests*/
        if (d < 0 || d > f) {
            return false;
        }
    } else {
        if (d > 0 || d < f) {
            return false;
        }
    }

    double e = xdiff0*Cy - ydiff0*Cx;                  /* beta numerator*/
    if(f > 0) {                                        /* beta tests*/
        if (e < 0 || e > f) {
            return false;
        }
    } else {
        if (e > 0 || e < f) {
            return false;
        }
    }

    if (f == 0) {
        // Theoretically, lines could still intersect in this case, but we don't care
        // because given numerical inaccuracies, the result is random anyway :-).
        return false;
    }
    
//    /*compute intersection coordinates*/
//    double num = d*xdiff0;                     /* numerator */
//    offset = SAME_SIGNS(num,f) ? f/2 : -f/2;   /* round direction*/
//    *x = x1 + (num+offset) / f;                /* intersection x */
//
//    num = d*ydiff0;
//    offset = SAME_SIGNS(num,f) ? f/2 : -f/2;
//    *y = y1 + (num+offset) / f;                /* intersection y */

    return true;
}

// Original code from: 
//     http://www.ecse.rpi.edu/Homepages/wrf/Research/Short_Notes/pnpoly.html.
// Precomputing a bounding box for the polygon would speed this up a lot,
// but the function hasn't shown up on any profiles so far.
bool pointInPolygon(const DPoint& pt, const vector<DPoint>& poly)
{
    if (poly.size() < 3) {
        return false;
    }
    bool bPtInPoly = false;
    for (unsigned i = 0, j = poly.size()-1; i < poly.size(); j = i++) {
        if (((poly[i].y > pt.y) != (poly[j].y > pt.y)) &&
                (pt.x < (poly[j].x-poly[i].x)*(pt.y-poly[i].y) / (poly[j].y-poly[i].y)
                 +poly[i].x))
        {
            bPtInPoly = !bPtInPoly;
        }
    }
    return bPtInPoly;
}
 
DPoint getLineLineIntersection(const DPoint& p1, const DPoint& v1, const DPoint& p2, 
        const DPoint& v2)
{
    double denom = v2.y*v1.x-v2.x*v1.y;
    if (fabs(denom) < 0.0000001) {
        // If the lines are parallel or coincident, we just return p2!
        return p2;
    }
    double numer = v2.x*(p1.y-p2.y) - v2.y*(p1.x-p2.x);
    double ua = numer/denom;

    return p1+ua*v1;

}


}
