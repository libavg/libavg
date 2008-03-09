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

#include "MathHelper.h"

#include <math.h>

namespace avg {

bool ispow2(int n) {
    return ((n & (n-1)) == 0);
}

int nextpow2(int n) {
    int ret=1;
    while (ret<n) {
        ret *= 2;
    }
    return ret;
/* TODO: Fix this fast version :-).
    int RetVal = 1;
    __asm__ __volatile__(
        "xorl %%ecx, %%ecx\n\t"
        "bsrl %1, %%ecx\n\t"
        "incl %%ecx\n\t"
        "shlb %%cl, %0\n\t"
        : "=m" (RetVal)
        : "m" (n)
        : "cc", "ecx"
        );
    return RetVal;
*/    
}

int safeCeil(double d) 
{
    if (fabs(d-int(d)) < EPSILON) {
        return d;
    } else {
        return int(d)+1;
    }
}

}
