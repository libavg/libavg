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

#ifndef _CameraCmd_H_
#define _CameraCmd_H_

#include "../avgconfig.h"
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION

#ifdef AVG_ENABLE_1394
#include <libdc1394/dc1394_control.h>
#endif
#ifdef AVG_ENABLE_1394_2
#include <dc1394/control.h>
#endif



namespace avg {

#ifdef AVG_ENABLE_1394
typedef int dc1394feature_t;
#define DC1394_FEATURE_MIN -1
#endif

struct CameraCmd {
    typedef enum { STOP, FEATURE } CmdType;
    CameraCmd(CmdType Cmd, dc1394feature_t Feature = DC1394_FEATURE_MIN, int Value = -1);
    CmdType m_Cmd;
    dc1394feature_t m_Feature;
    int m_Value;
};

}

#endif
