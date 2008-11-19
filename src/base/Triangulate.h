// Original code by John W. Ratcliff presumed to be in the public domain. Found 
// at http://www.flipcode.com/archives/Efficient_Polygon_Triangulation.shtml. 

#ifndef _Triangulate_H_
#define _Triangulate_H_

#include "Point.h"
#include "Triangle.h"

#include <vector>

namespace avg {

void triangulatePolygon(const DPointVector &contour, TriangleVector &result);

double getPolygonArea(const DPointVector &contour);

}

#endif

