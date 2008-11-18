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

#include "Triangle.h"

namespace avg {

Triangle::Triangle(const DPoint& P0, const DPoint& P1, const DPoint& P2)
    : p0(P0),
      p1(P1),
      p2(P2)
{
}

bool Triangle::operator ==(const Triangle & tri) const
{
  return (p0 == tri.p0 && p1 == tri.p1 &&  p2 == tri.p2);
}

std::ostream& operator<<(std::ostream& os, const Triangle& tri)
{
    os << "(" << tri.p0 << "," << tri.p1 << "," << tri.p2 << ")";
    return os;
}


}

