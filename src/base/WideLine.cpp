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

#include "WideLine.h"

namespace avg {

WideLine::WideLine(const DPoint& p0, const DPoint& p1, double width)
    : pt0(p0),
      pt1(p1)
{
    DPoint m = (pt1-pt0);
    m.normalize();
    DPoint w = DPoint(m.y, -m.x)*width/2;
    pl0 = p0-w;
    pr0 = p0+w;
    pl1 = p1-w;
    pr1 = p1+w;
    dir = DPoint(w.y, -w.x); 
}

double WideLine::getLen() const
{
    return calcDist(pt0, pt1);
}

std::ostream& operator<<(std::ostream& os, const WideLine& line)
{
    os << "(" << line.pt0 << "," << line.pt1 << ")";
    return os;
}

}

