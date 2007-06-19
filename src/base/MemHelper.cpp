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

#include "MemHelper.h"

#include <assert.h>
#include <iostream>

#include <unistd.h>
#include <sys/types.h>
#include <sys/sysctl.h>

#ifdef __APPLE__ 
#include <mach/mach.h>
#include <mach/task.h>
#include <mach/mach_init.h>
#endif

namespace avg {

using namespace std;

unsigned getMemUsed() 
{
    pid_t PID = getpid();
#ifdef __APPLE__ 
    kern_return_t rc;

    mach_port_t task;
    rc = task_for_pid(mach_task_self(), PID, &task);
    assert(rc == KERN_SUCCESS);
    
    mach_msg_type_number_t Count = TASK_BASIC_INFO_COUNT;
    task_basic_info taskInfo;
    rc = task_info(task, TASK_BASIC_INFO, (task_info_t)&taskInfo, &Count);
    assert(rc == KERN_SUCCESS);

    return taskInfo.resident_size; 
#endif

}

}
