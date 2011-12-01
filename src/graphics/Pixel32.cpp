//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2011 Ulrich von Zadow
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

void Pixel32::toHSL(float& h, float& s, float& l)
{
    float r = float(m_Data[REDPOS])/255.f;
    float g = float(m_Data[GREENPOS])/255.f;
    float b = float(m_Data[BLUEPOS])/255.f;
    float maxComp = max(r, max(g, b));
    float minComp = min(r, min(g, b));
    l = (maxComp+minComp)/2;
    if (maxComp == minComp) {
        s = 0;
        h = 0;
    } else {
        float delta = maxComp-minComp;
        if (l < 0.5f) {
            s = delta/(maxComp+minComp);
        } else {
            s = delta/(2-(maxComp+minComp));
        }
        if (r > g && r > b) {
            h = (g-b)/delta;
            if (h < 0.0f) {
                h += 6;
            }
        } else if (g > b) {
            h = 2+(b-r)/delta;
        } else {
            h = 4+(r-g)/delta;
        }
        h *= 60;
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

void YUVtoBGR32Pixel(Pixel32* pDest, int y, int u, int v)
{
    // u = Cb, v = Cr
    int u1 = u - 128;
    int v1 = v - 128;
    int tempy = 298*(y-16);
    int b = (tempy + 516 * u1           ) >> 8;
    int g = (tempy - 100 * u1 - 208 * v1) >> 8;
    int r = (tempy            + 409 * v1) >> 8;

    if (b<0) b = 0;
    if (b>255) b= 255;
    if (g<0) g = 0;
    if (g>255) g= 255;
    if (r<0) r = 0;
    if (r>255) r= 255;
    pDest->set(b,g,r,255);
}

void YUVJtoBGR32Pixel(Pixel32* pDest, int y, int u, int v)
{
    // u = Cb, v = Cr
    int u1 = u - 128;
    int v1 = v - 128;
    int tempy = 256*y;
    int b = (tempy + 452 * u1           ) >> 8;
    int g = (tempy -  88 * u1 - 182 * v1) >> 8;
    int r = (tempy            + 358 * v1) >> 8;

    if (b<0) b = 0;
    if (b>255) b= 255;
    if (g<0) g = 0;
    if (g>255) g= 255;
    if (r<0) r = 0;
    if (r>255) r= 255;
    pDest->set(b,g,r,255);
}

std::ostream& operator <<(std::ostream& os, const Pixel32& pix)
{
    os << pix.getColorString();
    return os;
}


}
