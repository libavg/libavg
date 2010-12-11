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

#include "LinuxMTHelper.h"

#include <linux/input.h>

#include <sstream>

namespace avg {

using namespace std;

string eventTypeToString(int type)
{
    switch(type) {
        case EV_SYN:
            return "EV_SYN";
        case EV_KEY:
            return "EV_KEY";
        case EV_REL:
            return "EV_REL";
        case EV_ABS:
            return "EV_ABS";
        case EV_MSC:
            return "EV_MSC";
        case EV_SW:
            return "EV_SW";
        case EV_LED:
            return "EV_LED";
        case EV_SND:
            return "EV_SND";
        case EV_REP:
            return "EV_REP";
        case EV_FF:
            return "EV_FF";
        case EV_PWR:
            return "EV_PWR";
        case EV_FF_STATUS:
            return "EV_FF_STATUS";
        default:
            stringstream ss;
            ss << type;
            return ss.str();
    }
}

string mtCodeToString(int code)
{
    switch (code) {
        case ABS_MT_SLOT:
            return "ABS_MT_SLOT";
        case ABS_MT_TOUCH_MAJOR:
            return "ABS_MT_TOUCH_MAJOR";
        case ABS_MT_TOUCH_MINOR:
            return "ABS_MT_TOUCH_MINOR";
        case ABS_MT_WIDTH_MAJOR:
            return "ABS_MT_WIDTH_MAJOR";
        case ABS_MT_WIDTH_MINOR:
            return "ABS_MT_WIDTH_MINOR";
        case ABS_MT_ORIENTATION:
            return "ABS_MT_ORIENTATION";
        case ABS_MT_POSITION_X:
            return "ABS_MT_POSITION_X";
        case ABS_MT_POSITION_Y:
            return "ABS_MT_POSITION_Y";
        case ABS_MT_TOOL_TYPE:
            return "ABS_MT_TOOL_TYPE";
        case ABS_MT_BLOB_ID:
            return "ABS_MT_BLOB_ID";
        case ABS_MT_TRACKING_ID:
            return "ABS_MT_TRACKING_ID";
        case ABS_MT_PRESSURE:
            return "ABS_MT_PRESSURE";
        default:
            stringstream ss;
            ss << code;
            return ss.str();
    }
}

string synCodeToString(int code)
{
    switch (code) {
        case SYN_REPORT:
            return "SYN_REPORT";
        case SYN_CONFIG:
            return "SYN_CONFIG";
        case SYN_MT_REPORT:
            return "SYN_MT_REPORT";
        default:
            stringstream ss;
            ss << code;
            return ss.str();
    }
}
   
}
