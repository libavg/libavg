/* 
 * Poly2Tri Copyright (c) 2009-2010, Poly2Tri Contributors
 * http://code.google.com/p/poly2tri/
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * * Neither the name of Poly2Tri nor the names of its contributors may be
 *   used to endorse or promote products derived from this software without specific
 *   prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "cdt.h"
#include <algorithm>
#include <iostream>

namespace avg {

CDT::CDT(Vec2Vector contour)
{
  std::vector<Point*> polyline;
  for (unsigned int i = 0; i < contour.size(); i++)
  {
	  polyline.push_back(new Point(contour[i].x, contour[i].y, i));
  }

  sweep_context_ = new SweepContext(polyline);
  sweep_ = new Sweep;
}

void CDT::AddHole(std::vector<Point*> polyline) ////remove
{
	sweep_context_->AddHole(polyline);
}

void CDT::AddHole(Vec2Vector hole)
{
	std::vector<Point*> polyline;
	for (unsigned int i = 0; i < hole.size(); i++)
	{
		polyline.push_back(new Point(hole[i].x, hole[i].y, i));
	}
	sweep_context_->AddHole(polyline);
}

void CDT::AddPoint(Point* point) {
	sweep_context_->AddPoint(point);
}

void CDT::Triangulate()
{
	sweep_->Triangulate(*sweep_context_);
}

std::vector<int> CDT::getIndexes()
{
	std::vector<int> result;
	std::vector<avg::TriangulationTriangle*> triangles =  sweep_context_->GetTriangles();
	for (unsigned int i = 0; i < triangles.size(); ++i){
		result.push_back(triangles[i]->GetPoint(0)->index);
		result.push_back(triangles[i]->GetPoint(1)->index);
		result.push_back(triangles[i]->GetPoint(2)->index);
	}
	return result;
}

CDT::~CDT()
{
  delete sweep_context_;
  delete sweep_;
}

}

