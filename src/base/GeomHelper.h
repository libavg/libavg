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

#ifndef _GeomHelper_H_
#define _GeomHelper_H_

#include "../api.h"
#include "Point.h"

#include <vector>

namespace avg {

struct DLineSegment {
public:
    DLineSegment(const DPoint& pt0, const DPoint& pt1);
    DPoint p0;
    DPoint p1;

    bool isPointOver(const DPoint& pt);
};

bool lineSegmentsIntersect(const DLineSegment& l0, const DLineSegment& l1);

bool pointInPolygon(const DPoint& pt, const std::vector<DPoint>& poly); 

DPoint getLineLineIntersection(const DPoint& p1, const DPoint& v1, const DPoint& p2, 
        const DPoint& v2);

}
#endif
 
