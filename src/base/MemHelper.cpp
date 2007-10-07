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
#include "FileHelper.h"

#include <assert.h>
#include <iostream>
#include <string>

#ifndef _WIN32
#include <unistd.h>
#include <sys/sysctl.h>
#endif
#include <sys/types.h>

#ifdef __APPLE__ 
#include <mach/mach.h>
#include <mach/task.h>
#include <mach/mach_init.h>
#else
#include <sstream>
#endif

namespace avg {

using namespace std;

string getNextLine(string& sBuf) 
{
    string::size_type pos = sBuf.find('\n');
    string sRet;
    if (pos == sBuf.npos) {
        sRet = sBuf;
        sBuf = "";
    } else {
        sRet = sBuf.substr(0, pos);
        sBuf = sBuf.erase(0, pos+1);
    }
    return sRet;
}

unsigned getMemUsed() 
{
#ifdef __APPLE__ 
    pid_t PID = getpid();
    kern_return_t rc;

    mach_port_t task;
    rc = task_for_pid(mach_task_self(), PID, &task);
    assert(rc == KERN_SUCCESS);
    
    mach_msg_type_number_t Count = TASK_BASIC_INFO_COUNT;
    task_basic_info taskInfo;
    rc = task_info(task, TASK_BASIC_INFO, (task_info_t)&taskInfo, &Count);
    assert(rc == KERN_SUCCESS);

    return taskInfo.resident_size;
#else
#ifdef _WIN32
    return 0;
#else
    pid_t PID = getpid();
    stringstream ss;
    ss << "/proc/" << PID << "/status";
    string sFName = ss.str();
    string sBuf;
    readWholeFile(sFName, sBuf);

    string sLine = getNextLine(sBuf);
    unsigned rss;
    while (sLine != "") {
        if (sLine.find("VmRSS") != sLine.npos) {
            rss = atoi(sLine.substr(6, 9).c_str());
            if (sLine.find("kB") != sLine.npos) {
                rss *= 1024;
            } else if (sLine.find("mB") != sLine.npos) { 
                rss *= 1024*1024;
            }
            return rss;
        }
        sLine = getNextLine(sBuf);
    }
    return 0;
#endif
#endif

}

}
