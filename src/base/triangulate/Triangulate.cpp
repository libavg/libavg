#include "Triangulate.h"
#include "sweep/Sweep.h"
#include "sweep/SweepContext.h"

#include "common/Shapes.h"

using namespace std;

namespace avg {

std::vector<int> triangulatePolygon(const Vec2Vector& points, const std::vector<int>& holeIndexes)
{
	std::vector<Point*> polyline;
	unsigned int contourEnd;

	if (holeIndexes.size() > 0) {
		contourEnd = holeIndexes[0];
	} else {
		contourEnd = points.size();
	}

	for (unsigned int i = 0; i < contourEnd; i++) {
		polyline.push_back(new Point(points.at(i).x, points.at(i).y, i));
	}

	SweepContext* sweepContext = new SweepContext(polyline);
	Sweep* sweep = new Sweep;

	if (holeIndexes.size() > 0) {
		std::vector<Point*> holeLine;
		for (unsigned int i = 0; i < holeIndexes.size(); i++) {
            if ( i < holeIndexes.size()-1) {
			    for (unsigned int j = holeIndexes.at(i); j < points.size() && j < holeIndexes.at(i+1); j++) {
				    holeLine.push_back(new Point(points.at(j).x, points.at(j).y, j));
			    }
            } else {
                for (unsigned int j = holeIndexes.at(i); j < points.size(); j++) {
				    holeLine.push_back(new Point(points.at(j).x, points.at(j).y, j));
			    }
            }
			sweepContext->AddHole(holeLine);
			holeLine.clear();
		}
	}

	sweep->Triangulate(*sweepContext);

	std::vector<int> result;
	std::vector<avg::TriangulationTriangle*> triangles =  sweepContext->GetTriangles();
	for (unsigned int i = 0; i < triangles.size(); ++i) {
		result.push_back(triangles.at(i)->GetPoint(0)->m_index);
		result.push_back(triangles.at(i)->GetPoint(1)->m_index);
		result.push_back(triangles.at(i)->GetPoint(2)->m_index);
	}
    
    delete sweep;
    delete sweepContext;

	return result;
}

}
