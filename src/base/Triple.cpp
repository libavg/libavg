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

#include "Triple.h"

#include "MathHelper.h"
#include "StringHelper.h"
#include "Exception.h"

#if defined(__SSE__) || defined(_WIN32)
#include <xmmintrin.h>
#endif

#include <math.h>
#include <float.h>

#include <string>

namespace avg {

template<class NUM>
Triple<NUM>::Triple()
{
    x = 0;
    y = 0;
    z = 0;
}

template<class NUM>
Triple<NUM>::Triple(NUM X, NUM Y, NUM Z)
{
    x = X;
    y = Y;
    z = Z;
}

template<class NUM>
Triple<NUM>::Triple(const Triple<NUM>& p)
{
    x = p.x;
    y = p.y;
    z = p.z;
}

    
template<class NUM>
Triple<NUM>::Triple(const std::vector<NUM>& v)
{
    AVG_ASSERT(v.size() == 3);
    x = v[0];
    y = v[1];
    z = v[2];
}

template<class NUM>
Triple<NUM>::~Triple()
{
}

template<class NUM>
std::ostream& operator<<(std::ostream& os, const Triple<NUM> &p)
{
    os << "(" << p.x << "," << p.y << "," << p.z << ")";
    return os;
}

template<class NUM>
std::istream& operator>>(std::istream& is, Triple<NUM>& p)
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

// Explicit instantiations.
template class Triple<double>;
template std::ostream& operator<<(std::ostream& os, const Triple<double> &p);
template std::istream& operator>>(std::istream& is, Triple<double>& p);

template class Triple<int>;
template std::ostream& operator<<(std::ostream& os, const Triple<int> &p);
template std::istream& operator>>(std::istream& is, Triple<int>& p);

}
