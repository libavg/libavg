// Original code by John W. Ratcliff presumed to be in the public domain. Found 
// at http://www.flipcode.com/archives/Efficient_Polygon_Triangulation.shtml. 

#ifndef _Triangulate_H_
#define _Triangulate_H_

#include "Triangle.h"
#include "GLMHelper.h"

#include <vector>

namespace avg {

// Result type is suitable for use in a Triangle Vertex Array.
void triangulatePolygon(const Vec2Vector &contour, std::vector<int> &resultIndexes);

float getPolygonArea(const Vec2Vector &contour);

}

#endif

