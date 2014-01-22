//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2014 Ulrich von Zadow
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

#include "AppleDisplay.h"
#include "../base/Logger.h"

#include <ApplicationServices/ApplicationServices.h>

#include <iostream>

using namespace std;

namespace avg {

AppleDisplay::AppleDisplay()
{
}

AppleDisplay::~AppleDisplay()
{
}

float AppleDisplay::queryPPMM()
{
    CGSize size = CGDisplayScreenSize(CGMainDisplayID());
    return getScreenResolution().x/size.width;
}

float AppleDisplay::queryRefreshRate()
{
    float refreshRate;
#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_6
    CGDisplayModeRef mode = CGDisplayCopyDisplayMode(CGMainDisplayID());
    refreshRate = CGDisplayModeGetRefreshRate(mode);
    if (refreshRate < 1.0) {
        AVG_TRACE(Logger::category::CONFIG, Logger::severity::INFO,
                "This seems to be a TFT screen, assuming 60 Hz refresh rate.");
        refreshRate = 60;
    }
    CGDisplayModeRelease(mode);
#else
    CFDictionaryRef modeInfo = CGDisplayCurrentMode(CGMainDisplayID());
    AVG_ASSERT(modeInfo);
    CFNumberRef value = (CFNumberRef) CFDictionaryGetValue(modeInfo, 
            kCGDisplayRefreshRate);
    AVG_ASSERT(value);
    CFNumberGetValue(value, kCFNumberIntType, &refreshRate);
    if (refreshRate < 1.0) {
        AVG_TRACE(Logger::category::CONFIG, Logger::severity::INFO,
                "This seems to be a TFT screen, assuming 60 Hz refresh rate.");
        refreshRate = 60;
    }
#endif
    return refreshRate;
}

}
