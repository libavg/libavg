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

#include "IntTriple.h"

#include "MathHelper.h"
#include "StringHelper.h"

#if defined(__SSE__) || defined(_WIN32)
#include <xmmintrin.h>
#endif

#include <math.h>
#include <assert.h>
#include <float.h>

#include <string>

namespace avg {

IntTriple::IntTriple()
{
	x = 0;
	y = 0;
	z = 0;
}

IntTriple::IntTriple(int X, int Y, int Z)
{
    x = X;
    y = Y;
    z = Z;
}

IntTriple::IntTriple(const IntTriple& p)
{
    x = p.x;
    y = p.y;
    z = p.z;
}

    
IntTriple::IntTriple(const std::vector<int>& v)
{
    assert(v.size() == 3);
    x = v[0];
    y = v[1];
    z = v[2];
}

IntTriple::~IntTriple()
{
}

std::ostream& operator<<( std::ostream& os, const IntTriple &p)
{
    os << "(" << p.x << "," << p.y << "," << p.z << ")";
    return os;
}

std::istream& operator>>(std::istream& is, IntTriple& p)
{
    skipToken(is, '(');
    is >> p.x;
    skipToken(is, ',');
    is >> p.y;
    skipToken(is, ',');
    is >> p.z;
    skipToken(is, ')');
    return is;
}


}
