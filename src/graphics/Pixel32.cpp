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

#include "Pixel32.h"

#include "../base/Exception.h"

#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <stdio.h>

namespace avg {

using namespace std;

void Pixel32::toHSL(double& h, double& s, double& l)
{
    double r = double(m_Data[REDPOS])/255;
    double g = double(m_Data[GREENPOS])/255;
    double b = double(m_Data[BLUEPOS])/255;
    double maxComp = max(r, max(g, b));
    double minComp = min(r, min(g, b));
    l = (maxComp+minComp)/2.0;
    if (maxComp == minComp) {
        s = 0.0;
        h = 0.0;
    } else {
        double delta = maxComp-minComp;
        if (l < 0.5) {
            s = delta/(maxComp+minComp);
        } else {
            s = delta/(2.0-(maxComp+minComp));
        }
        if (r == maxComp) {
            h = (g-b)/delta;
            if (h < 0.0) {
                h += 6.0;
            }
        } else if (g == maxComp) {
            h = 2.0+(b-r)/delta;
        } else if (b == maxComp) {
            h = 4.0+(r-g)/delta;
        }
        h *= 60.0;
    } 
}

std::string Pixel32::getColorString() const
{
    stringstream s;
    s.fill('0');
    s << hex << setw(2) << (int)(m_Data[0]) << setw(2) << (int)(m_Data[1]) <<
            setw(2) << (int)(m_Data[2]) << setw(2) << (int)(m_Data[3]);
    return s.str();
}

Pixel32 colorStringToColor(const string & s)
{
    int r,g,b;
    if ((s.length() != 6) || (sscanf(s.c_str(), "%2x%2x%2x", &r, &g, &b) != 3)) {
        throw(Exception (AVG_ERR_INVALID_ARGS, "colorstring cannot be parsed."));
    }
    return Pixel32(r,g,b);
}

std::ostream& operator <<(std::ostream& os, const Pixel32& pix)
{
    os << pix.getColorString();
    return os;
}


}
