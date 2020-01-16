// Original code by John W. Ratcliff presumed to be in the public domain. Found 
// at http://www.flipcode.com/archives/Efficient_Polygon_Triangulation.shtml. 

#ifndef _Triangulate_H_
#define _Triangulate_H_

#include "Triangle.h"
#include "GLMHelper.h"

#include <vector>

namespace avg {

class Polygon {
public:
    Polygon();
    Polygon(const Vec2Vector& pts);

    const Vec2Vector& getPts() const;
    float getArea();
    void triangulate(Vec2Vector& resultVertices, std::vector<int>& resultIndexes);

private:
    Vec2Vector m_Pts;
};

}

#endif
