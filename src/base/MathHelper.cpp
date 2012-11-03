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

#include "MathHelper.h"

#include <math.h>
#include <iostream>

using namespace std;

namespace avg {

bool ispow2(int n)
{
    return ((n & (n-1)) == 0);
}

int nextpow2(int n)
{
    n--;
    n |= n >> 1;  // handle  2 bit numbers
    n |= n >> 2;  // handle  4 bit numbers
    n |= n >> 4;  // handle  8 bit numbers
    n |= n >> 8;  // handle 16 bit numbers
    n |= n >> 16; // handle 32 bit numbers
    n++;
    return n;
}

int safeCeil(float d) 
{
    if (fabs(d-int(d)) < EPSILON) {
        return int(d);
    } else {
        return int(d)+1;
    }
}

float invSqrt(float x)
{
#if 0
    // TODO: This gives incorrect results on Athlon X2, gcc 4.2.
    float xhalf = 0.5f*x;
    int i = *(int*)&x;         // get bits for floating value
    i = 0x5f3759d5 - (i>>1);   // give initial guess y0
    x = *(float*)&i;          // convert bits back to float
    x *= 1.5f - xhalf*x*x;     // newton step, repeating this step
                               // increases accuracy
    x *= 1.5f - xhalf*x*x;
    return x;
#endif
    return 1/sqrt(x);
}

bool almostEqual(float d1, float d2, float epsilon)
{
    return (fabs(d1-d2)<epsilon);
}

}
