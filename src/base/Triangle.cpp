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

#include "Triangle.h"

#include <math.h>

namespace avg {

Triangle::Triangle(const DPoint& P0, const DPoint& P1, const DPoint& P2)
    : p0(P0),
      p1(P1),
      p2(P2)
{
}

Triangle::Triangle()
{
}

bool Triangle::operator ==(const Triangle & tri) const
{
    return (p0 == tri.p0 && p1 == tri.p1 &&  p2 == tri.p2);
}

bool Triangle::isInside(const DPoint& pt) const
{
/* Slower func that only works for cw triangles.

    DPoint a = p2-p1;
    DPoint bp = pt-p1;
    double aCROSSbp = a.x*bp.y - a.y*bp.x;
    if (aCROSSbp < 0.0) {
        return false;
    }
    
    DPoint b = p0-p2;
    DPoint cp = pt-p2;
    double bCROSScp = b.x*cp.y - b.y*cp.x;
    if (bCROSScp < 0.0) {
        return false;
    }

    DPoint c = p1-p0;
    DPoint ap = pt-p0;
    double cCROSSap = c.x*ap.y - c.y*ap.x;
    return cCROSSap >= 0.0;
*/
    DPoint v0 = p2 - p0;
    DPoint v1 = p1 - p0;
    DPoint v2 = pt - p0;

    double dot00 = dotProduct(v0, v0);
    double dot01 = dotProduct(v0, v1);
    double dot02 = dotProduct(v0, v2);
    double dot11 = dotProduct(v1, v1);
    double dot12 = dotProduct(v1, v2);

    double invDenom = 1 / (dot00 * dot11 - dot01 * dot01);
    double u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    double v = (dot00 * dot12 - dot01 * dot02) * invDenom;

    return (u > 0) && (v > 0) && (u + v < 1);

}

double Triangle::getArea() const
{
    return fabs((((p1.x-p0.x)*(p2.y-p0.y)) - ((p1.y-p0.y)*(p2.x-p0.x)))/2);
}

bool Triangle::isClockwise() const
{
    return ((p1.x-p0.x)*(p2.y-p0.y)) - ((p1.y-p0.y)*(p2.x-p0.x)) < 0;
}

std::ostream& operator<<(std::ostream& os, const Triangle& tri)
{
    os << "(" << tri.p0 << "," << tri.p1 << "," << tri.p2 << ")";
    return os;
}


}

