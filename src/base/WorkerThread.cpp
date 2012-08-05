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

void setAffinityMask(bool bIsMainThread)
{
    // The main thread gets the first processor to itself. All other threads share the
    // rest of the processors available, unless, of course, there is only one processor
    // in the machine.
#ifdef linux
    static cpu_set_t allProcessors;
    static bool bInitialized = false;
    if (!bInitialized) {
        int rc = sched_getaffinity(0, sizeof(allProcessors), &allProcessors);
        AVG_ASSERT(rc == 0);
//        cerr << "All processors: ";
//        printAffinityMask(allProcessors);
        bInitialized = true;
    }
    cpu_set_t mask;
    if (bIsMainThread) {
        CPU_ZERO(&mask);
        CPU_SET(0, &mask);
//        cerr << "Main Thread: ";
    } else {
        mask = allProcessors;
        if (CPU_COUNT(&mask) > 1) {
            CPU_CLR(0, &mask);
        }
//        cerr << "Aux Thread: ";
    }
//    printAffinityMask(mask);
    int rc = sched_setaffinity(0, sizeof(mask), &mask);
    AVG_ASSERT(rc == 0);
#endif
}

}

