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

#include "WorkerThread.h"

#include "OSHelper.h"

#ifdef _WIN32
#include <Windows.h>
#endif

namespace avg {

using namespace std;

#ifdef linux
void printAffinityMask(cpu_set_t& mask)
{
    for (int i=0; i<32; ++i) {
        cerr << int(CPU_ISSET(i, &mask));
    }
    cerr << endl;
}
#endif

unsigned getLowestBitSet(unsigned val)
{
    AVG_ASSERT(val != 0); // Doh

    unsigned pos = 0;
    while (!(val & 1)) {
        val >>= 1;
        ++pos;
    }
    return pos;
}

}

