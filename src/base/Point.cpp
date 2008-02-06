//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
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

#include "Point.h"

#include <math.h>

namespace avg {

bool almostEqual(const DPoint& pt1, const DPoint& pt2)
{
    return (fabs(pt1.x-pt2.x)+fabs(pt1.y-pt2.y)) < 0.0001;
}

DPoint rotate(const DPoint& point, double angle, const DPoint& pivot)
{
    double cosVal = cos(angle);
    double sinVal = sin(angle);

    // translate pivot to origin
    DPoint translated = point - pivot;
    
    // calculate rotated coordinates about the origin
    DPoint rotated(translated.x * cosVal - translated.y * sinVal,
        translated.x * sinVal + translated.y * cosVal);

    // re-translate pivot to original position
    rotated += pivot;

    return rotated;
}

}
