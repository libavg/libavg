//
//  libavg - Media Playback Engine.
//  Copyright (C) 2003-2013 Ulrich von Zadow
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

#include "ThreadHelper.h"
#include "OSHelper.h"

#ifdef _WIN32
#include <Windows.h>
#endif

namespace avg {

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
#elif defined _WIN32
    DWORD processAffinityMask;
    DWORD systemAffinityMask;
    BOOL rc = GetProcessAffinityMask(GetCurrentProcess(), &processAffinityMask,
            &systemAffinityMask);
    AVG_ASSERT(rc == TRUE);
    DWORD mainThreadMask = 1 << getLowestBitSet(processAffinityMask);
    DWORD mask;
    if (bIsMainThread) {
        mask = mainThreadMask;
    } else {
        mask = processAffinityMask & ~mainThreadMask;
        if (mask == 0) {
            mask = processAffinityMask;
        }
    }
    DWORD_PTR pPrevMask = SetThreadAffinityMask(GetCurrentThread(), mask);
    AVG_ASSERT_MSG(pPrevMask != 0, getWinErrMsg(GetLastError()).c_str());
#endif
}

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
