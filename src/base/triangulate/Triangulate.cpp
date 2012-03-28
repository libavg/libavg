#include "Triangulate.h"
#include "sweep/Sweep.h"
#include "sweep/SweepContext.h"

#include "common/Shapes.h" //!!!!!!

namespace avg{

	std::vector<int> triangulatePolygon(const Vec2Vector& points, const std::vector<int>& holeIndexes){
		//ToDo mgf:vec2 verwenden wrapper entfernen | #include "common/Shapes.h"
		std::vector<Point*> polyline;
		for (unsigned int i = 0; i < points.size(); i++) {
			polyline.push_back(new Point(points[i].x, points[i].y, i));
		}

		SweepContext* sweepContext  = new SweepContext(polyline);
		Sweep* sweep = new Sweep;

		//sweepContext.addHoles

		sweep->Triangulate(*sweepContext);

		//ToDo indexes besser rausloesen
		std::vector<int> result;
		std::vector<avg::TriangulationTriangle*> triangles =  sweepContext->GetTriangles();
		for (unsigned int i = 0; i < triangles.size(); ++i){
			result.push_back(triangles[i]->GetPoint(0)->index);
			result.push_back(triangles[i]->GetPoint(1)->index);
			result.push_back(triangles[i]->GetPoint(2)->index);
		}

		return result;
	}

}
