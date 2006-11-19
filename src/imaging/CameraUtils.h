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

#ifndef _CameraUtils_H_
#define _CameraUtils_H_

#include "../avgconfig.h"
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION

#include "../graphics/Point.h"

#ifdef AVG_ENABLE_1394
#include <libraw1394/raw1394.h>
#include <libdc1394/dc1394_control.h>
#endif
#ifdef AVG_ENABLE_1394_2
#include <dc1394/control.h>
#endif

#include <string>

namespace avg {

#ifdef AVG_ENABLE_1394
int getCamMode(std::string sMode);
IntPoint getCamImgSize(int Mode);
int getFrameRateConst(double FrameRate);
int getFeatureID(const std::string& sFeature);
#else
dc1394video_mode_t getCamMode(std::string sMode);
IntPoint getCamImgSize(dc1394video_mode_t Mode);
dc1394framerate_t getFrameRateConst(double FrameRate);
#endif


}

#endif
