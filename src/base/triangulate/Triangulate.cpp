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

//
// Based on Poly2Tri algorithm.
// Poly2Tri Copyright (c) 2009-2010, Poly2Tri Contributors
// http://code.google.com/p/poly2tri/
//

#include "Triangulate.h"
#include "Sweep.h"
#include "SweepContext.h"

#include "Shapes.h"

using namespace std;

namespace avg {

std::vector<unsigned int> triangulatePolygon(const Vec2Vector& points,
        const std::vector<unsigned int>& holeIndexes)
{
    std::vector<Point*> polyline;
    std::vector<Point*> holeLine;
    unsigned int contourEnd;

    if (holeIndexes.size() > 0) {
        contourEnd = holeIndexes[0];
    } else {
        contourEnd = points.size();
    }

    for (unsigned int i = 0; i < contourEnd; i++) {
        polyline.push_back(new Point(points[i].x, points[i].y, i));
    }

    SweepContext* sweepContext = new SweepContext(polyline);
    Sweep* sweep = new Sweep;

    if (holeIndexes.size() > 0) {
        for (unsigned int i = 0; i < holeIndexes.size(); i++) {
            if ( i < holeIndexes.size()-1) {
                for (unsigned int j = holeIndexes[i]; j < points.size() && j <
                        holeIndexes[i+1]; j++)
                {
                    holeLine.push_back(new Point(points[j].x, points[j].y, j));
                }
            } else {
                for (unsigned int j = holeIndexes[i]; j < points.size(); j++) {
                    holeLine.push_back(new Point(points[j].x, points[j].y, j));
                }
            }
            sweepContext->addHole(holeLine);
            holeLine.clear();
        }
    }

    sweep->Triangulate(*sweepContext);

    std::vector<unsigned int> result;
    std::vector<avg::TriangulationTriangle*>& triangles =  sweepContext->getTriangles();
    for (unsigned int i = 0; i < triangles.size(); ++i) {
        result.push_back(triangles[i]->getPoint(0)->m_Index);
        result.push_back(triangles[i]->getPoint(1)->m_Index);
        result.push_back(triangles[i]->getPoint(2)->m_Index);
    }
    
    delete sweep;
    delete sweepContext;

    for (unsigned int i = 0; i < polyline.size(); i++) {
        delete polyline[i];
    }

    return result;
}

}
