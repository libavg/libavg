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
//  Original author of this file is Nick Hebner (hebnern@gmail.com).
//

#include "CMUCameraUtils.h"

#include "../base/Logger.h"

namespace avg {

using namespace std;

#define FORMAT_0 0
// Format 0 Modes
#define MODE_320x240_YUV422  1
#define MODE_640x480_MONO    5
#define MODE_640x480_MONO16  6
#define MODE_640x480_YUV411  2
#define MODE_640x480_YUV422  3
#define MODE_640x480_RGB     4

#define FORMAT_1 1
// Format 1 Modes
#define MODE_800x600_MONO    2
#define MODE_800x600_MONO16  6
#define MODE_800x600_YUV422  0
#define MODE_800x600_RGB     1
#define MODE_1024x768_MONO   5
#define MODE_1024x768_MONO16 6
#define MODE_1024x768_YUV422 3
#define MODE_1024x768_RGB    4

// Framerates
#define FRAMERATE_1_875 0
#define FRAMERATE_3_75  1
#define FRAMERATE_7_5   2
#define FRAMERATE_15    3
#define FRAMERATE_30    4
#define FRAMERATE_60    5

void getVideoFormatAndMode(IntPoint& Size, std::string& sPF, 
                           unsigned long* pVideoFormat, 
                           unsigned long* pVideoMode) 
{
    if (Size.x == 320 && Size.y == 240) {
        *pVideoFormat = FORMAT_0;
        if (sPF == "MONO8") {
            *pVideoMode = MODE_320x240_YUV422;
        } else if (sPF == "YUV422") {
            *pVideoMode = MODE_320x240_YUV422;
        }
    } else if (Size.x == 640 && Size.y == 480) {
        *pVideoFormat = FORMAT_0;
        if (sPF == "MONO8") {
            *pVideoMode = MODE_640x480_MONO;
        } else if (sPF == "MONO16") {
            *pVideoMode = MODE_640x480_MONO16;
        } else if (sPF == "YUV411") {
            *pVideoMode = MODE_640x480_YUV411;
        } else if (sPF == "YUV422") {
            *pVideoMode = MODE_640x480_YUV422;
        } else if (sPF == "RGB" || sPF == "BGR") {
            *pVideoMode = MODE_640x480_RGB;
        }
    } else if (Size.x == 800 && Size.y == 600) {
        *pVideoFormat = FORMAT_1;
        if (sPF == "MONO8") {
            *pVideoMode = MODE_800x600_MONO;
        } else if (sPF == "MONO16") {
            *pVideoMode = MODE_800x600_MONO16;
        } else if (sPF == "YUV422") {
            *pVideoMode = MODE_800x600_YUV422;
        } else if (sPF == "RGB" || sPF == "BGR") {
            *pVideoMode = MODE_800x600_RGB;
        }
    } else if (Size.x == 1024 && Size.y == 768) {
        *pVideoFormat = FORMAT_1;
        if (sPF == "MONO8" || sPF == "BY8_GBRG") {
            *pVideoMode = MODE_1024x768_MONO;
        } else if (sPF == "MONO16") {
            *pVideoMode = MODE_1024x768_MONO16;
        } else if (sPF == "YUV422") {
            *pVideoMode = MODE_1024x768_YUV422;
        } else if (sPF == "RGB" || sPF == "BGR") {
            *pVideoMode = MODE_1024x768_RGB;
        }
    } else {
        AVG_TRACE (Logger::WARNING,
                std::string("Unsupported or illegal value for camera mode."));
        *pVideoFormat = FORMAT_0;
        *pVideoMode = MODE_640x480_RGB;
    }
}

unsigned long getFrameRateConst(double FrameRate)
{
    if (FrameRate == 1.875) {
        return FRAMERATE_1_875;
    } else if (FrameRate == 3.75) {
        return FRAMERATE_3_75;
    } else if (FrameRate == 7.5) {
        return FRAMERATE_7_5;
    } else if (FrameRate == 15) {
        return FRAMERATE_15;
    } else if (FrameRate == 30) {
        return FRAMERATE_30;
    } else if (FrameRate == 60) {
        return FRAMERATE_60;
    } else {
        AVG_TRACE (Logger::WARNING,
                std::string("Unsupported or illegal value for camera framerate."));
        return FRAMERATE_15; 
    }
}

CAMERA_FEATURE getFeatureID(CameraFeature Feature)
{
    switch(Feature) {
        case CAM_FEATURE_BRIGHTNESS:
            return FEATURE_BRIGHTNESS;
        case CAM_FEATURE_EXPOSURE:
            return FEATURE_AUTO_EXPOSURE;
        case CAM_FEATURE_SHARPNESS:
            return FEATURE_SHARPNESS;
        case CAM_FEATURE_WHITE_BALANCE:
            return FEATURE_WHITE_BALANCE;
        case CAM_FEATURE_HUE:
            return FEATURE_HUE;
        case CAM_FEATURE_SATURATION:
            return FEATURE_SATURATION;
        case CAM_FEATURE_GAMMA:
            return FEATURE_GAMMA;
        case CAM_FEATURE_SHUTTER:
            return FEATURE_SHUTTER;
        case CAM_FEATURE_GAIN:
            return FEATURE_GAIN;
        case CAM_FEATURE_IRIS:
            return FEATURE_IRIS;
        case CAM_FEATURE_FOCUS:
            return FEATURE_FOCUS;
        case CAM_FEATURE_TEMPERATURE:
            return FEATURE_TEMPERATURE;
        case CAM_FEATURE_TRIGGER:
            return FEATURE_TRIGGER_MODE;
        case CAM_FEATURE_ZOOM:
            return FEATURE_ZOOM;
        case CAM_FEATURE_PAN:
            return FEATURE_PAN;
        case CAM_FEATURE_TILT:
            return FEATURE_TILT;
        case CAM_FEATURE_OPTICAL_FILTER:
            return FEATURE_OPTICAL_FILTER;
        case CAM_FEATURE_CAPTURE_SIZE:
            return FEATURE_CAPTURE_SIZE;
        case CAM_FEATURE_CAPTURE_QUALITY:
            return FEATURE_CAPTURE_QUALITY;
        default:
            return FEATURE_INVALID_FEATURE;
    }
}

}
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
//  Original author of this file is Nick Hebner (hebnern@gmail.com).
//

#include "CMUCameraUtils.h"

#include "../base/Logger.h"

namespace avg {

using namespace std;

#define FORMAT_0 0
// Format 0 Modes
#define MODE_320x240_YUV422  1
#define MODE_640x480_MONO    5
#define MODE_640x480_MONO16  6
#define MODE_640x480_YUV411  2
#define MODE_640x480_YUV422  3
#define MODE_640x480_RGB     4

#define FORMAT_1 1
// Format 1 Modes
#define MODE_800x600_MONO    2
#define MODE_800x600_MONO16  6
#define MODE_800x600_YUV422  0
#define MODE_800x600_RGB     1
#define MODE_1024x768_MONO   5
#define MODE_1024x768_MONO16 6
#define MODE_1024x768_YUV422 3
#define MODE_1024x768_RGB    4

// Framerates
#define FRAMERATE_1_875 0
#define FRAMERATE_3_75  1
#define FRAMERATE_7_5   2
#define FRAMERATE_15    3
#define FRAMERATE_30    4
#define FRAMERATE_60    5

void getVideoFormatAndMode(IntPoint& Size, std::string& sPF, 
                           unsigned long* pVideoFormat, 
                           unsigned long* pVideoMode) 
{
    if (Size.x == 320 && Size.y == 240) {
        *pVideoFormat = FORMAT_0;
        if (sPF == "MONO8") {
            *pVideoMode = MODE_320x240_YUV422;
        } else if (sPF == "YUV422") {
            *pVideoMode = MODE_320x240_YUV422;
        }
    } else if (Size.x == 640 && Size.y == 480) {
        *pVideoFormat = FORMAT_0;
        if (sPF == "MONO8") {
            *pVideoMode = MODE_640x480_MONO;
        } else if (sPF == "MONO16") {
            *pVideoMode = MODE_640x480_MONO16;
        } else if (sPF == "YUV411") {
            *pVideoMode = MODE_640x480_YUV411;
        } else if (sPF == "YUV422") {
            *pVideoMode = MODE_640x480_YUV422;
        } else if (sPF == "RGB" || sPF == "BGR") {
            *pVideoMode = MODE_640x480_RGB;
        }
    } else if (Size.x == 800 && Size.y == 600) {
        *pVideoFormat = FORMAT_1;
        if (sPF == "MONO8") {
            *pVideoMode = MODE_800x600_MONO;
        } else if (sPF == "MONO16") {
            *pVideoMode = MODE_800x600_MONO16;
        } else if (sPF == "YUV422") {
            *pVideoMode = MODE_800x600_YUV422;
        } else if (sPF == "RGB" || sPF == "BGR") {
            *pVideoMode = MODE_800x600_RGB;
        }
    } else if (Size.x == 1024 && Size.y == 768) {
        *pVideoFormat = FORMAT_1;
        if (sPF == "MONO8" || sPF == "BY8_GBRG") {
            *pVideoMode = MODE_1024x768_MONO;
        } else if (sPF == "MONO16") {
            *pVideoMode = MODE_1024x768_MONO16;
        } else if (sPF == "YUV422") {
            *pVideoMode = MODE_1024x768_YUV422;
        } else if (sPF == "RGB" || sPF == "BGR") {
            *pVideoMode = MODE_1024x768_RGB;
        }
    } else {
        AVG_TRACE (Logger::WARNING,
                std::string("Unsupported or illegal value for camera mode."));
        *pVideoFormat = FORMAT_0;
        *pVideoMode = MODE_640x480_RGB;
    }
}

unsigned long getFrameRateConst(double FrameRate)
{
    if (FrameRate == 1.875) {
        return FRAMERATE_1_875;
    } else if (FrameRate == 3.75) {
        return FRAMERATE_3_75;
    } else if (FrameRate == 7.5) {
        return FRAMERATE_7_5;
    } else if (FrameRate == 15) {
        return FRAMERATE_15;
    } else if (FrameRate == 30) {
        return FRAMERATE_30;
    } else if (FrameRate == 60) {
        return FRAMERATE_60;
    } else {
        AVG_TRACE (Logger::WARNING,
                std::string("Unsupported or illegal value for camera framerate."));
        return FRAMERATE_15; 
    }
}

CAMERA_FEATURE getFeatureID(CameraFeature Feature)
{
    switch(Feature) {
        case CAM_FEATURE_BRIGHTNESS:
            return FEATURE_BRIGHTNESS;
        case CAM_FEATURE_EXPOSURE:
            return FEATURE_AUTO_EXPOSURE;
        case CAM_FEATURE_SHARPNESS:
            return FEATURE_SHARPNESS;
        case CAM_FEATURE_WHITE_BALANCE:
            return FEATURE_WHITE_BALANCE;
        case CAM_FEATURE_HUE:
            return FEATURE_HUE;
        case CAM_FEATURE_SATURATION:
            return FEATURE_SATURATION;
        case CAM_FEATURE_GAMMA:
            return FEATURE_GAMMA;
        case CAM_FEATURE_SHUTTER:
            return FEATURE_SHUTTER;
        case CAM_FEATURE_GAIN:
            return FEATURE_GAIN;
        case CAM_FEATURE_IRIS:
            return FEATURE_IRIS;
        case CAM_FEATURE_FOCUS:
            return FEATURE_FOCUS;
        case CAM_FEATURE_TEMPERATURE:
            return FEATURE_TEMPERATURE;
        case CAM_FEATURE_TRIGGER:
            return FEATURE_TRIGGER_MODE;
        case CAM_FEATURE_ZOOM:
            return FEATURE_ZOOM;
        case CAM_FEATURE_PAN:
            return FEATURE_PAN;
        case CAM_FEATURE_TILT:
            return FEATURE_TILT;
        case CAM_FEATURE_OPTICAL_FILTER:
            return FEATURE_OPTICAL_FILTER;
        case CAM_FEATURE_CAPTURE_SIZE:
            return FEATURE_CAPTURE_SIZE;
        case CAM_FEATURE_CAPTURE_QUALITY:
            return FEATURE_CAPTURE_QUALITY;
        default:
            return FEATURE_INVALID_FEATURE;
    }
}

}
