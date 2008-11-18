// Original code by John W. Ratcliff presumed to be in the public domain. Found 
// at http://www.flipcode.com/archives/Efficient_Polygon_Triangulation.shtml. 

#ifndef _Triangulate_H_
#define _Triangulate_H_

#include "Point.h"
#include "Triangle.h"

#include <vector>

namespace avg {

class Triangulate
{
public:

  // triangulate a contour/polygon, places results in STL vector
  // as series of triangles.
  static bool Process(const DPointVector &contour,
                      TriangleVector &result);

  // compute area of a contour/polygon
  static float Area(const DPointVector &contour);

  // decide if point Px/Py is inside triangle defined by
  // (Ax,Ay) (Bx,By) (Cx,Cy)
  static bool InsideTriangle(float Ax, float Ay,
                      float Bx, float By,
                      float Cx, float Cy,
                      float Px, float Py);

private:
  static bool Snip(const DPointVector &contour,int u,int v,int w,int n,int *V);

};

}

#endif

