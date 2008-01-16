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

#include "FWCameraUtils.h"

#include "../base/Logger.h"

namespace avg {

using namespace std;

#ifdef AVG_ENABLE_1394
int getCamMode(IntPoint Size, std::string sPF) 
{
    if (Size.x == 320 && Size.y == 240 && sPF == "YUV422") {
        return MODE_320x240_YUV422;
    } else if (Size.x == 640 && Size.y == 480) {
        if (sPF == "MONO8") {
            return MODE_640x480_MONO;
        } else if (sPF == "MONO16") {
            return MODE_640x480_MONO16;
        } else if (sPF == "YUV411") {
            return MODE_640x480_YUV411;
        } else if (sPF == "YUV422") {
            return MODE_640x480_YUV422;
        } else if (sPF == "RGB") {
            return MODE_640x480_RGB;
        }
    } else if (Size.x == 800 && Size.y == 600) {
        if (sPF == "MONO8") {
            return MODE_800x600_MONO;
        } else if (sPF == "MONO16") {
            return MODE_800x600_MONO16;
        } else if (sPF == "YUV422") {
            return MODE_800x600_YUV422;
        } else if (sPF == "RGB") {
            return MODE_800x600_RGB;
        }
    } else if (Size.x == 1024 && Size.y == 768) {
        if (sPF == "MONO8") {
            return MODE_1024x768_MONO;
        } else if (sPF == "MONO16") {
            return MODE_1024x768_MONO16;
        } else if (sPF == "YUV422") {
            return MODE_1024x768_YUV422;
        } else if (sPF == "RGB") {
            return MODE_1024x768_RGB;
        }
    }
    AVG_TRACE (Logger::WARNING,
            std::string("getCamMode: Unsupported or illegal value for camera mode: (")
            << Size.x << ", " << Size.y << "), " << sPF << ".");
    return MODE_640x480_RGB;
}

int getFrameRateConst(double FrameRate)
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
        AVG_TRACE (Logger::ERROR,
                std::string("Unsupported or illegal value for camera framerate."));
        return -1; 
    }
}

int getFeatureID(CameraFeature Feature)
{
    switch(Feature) {
        case CAM_FEATURE_BRIGHTNESS:
            return FEATURE_BRIGHTNESS;
        case CAM_FEATURE_EXPOSURE:
            return FEATURE_EXPOSURE;
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
            return FEATURE_TRIGGER;
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
            AVG_TRACE(Logger::WARNING, "getFeatureID: " 
                    << cameraFeatureToString(Feature) << " unknown.");
            return 0;
    }
}

#else

dc1394video_mode_t getCamMode(IntPoint Size, std::string sPF) 
{
    if (Size.x == 320 && Size.y == 240 && sPF == "YUV422") {
        return DC1394_VIDEO_MODE_320x240_YUV422;
    } else if (Size.x == 640 && Size.y == 480) {
        if (sPF == "MONO8") {
            return DC1394_VIDEO_MODE_640x480_MONO8;
        } else if (sPF == "MONO16") {
            return DC1394_VIDEO_MODE_640x480_MONO16;
        } else if (sPF == "YUV411") {
            return DC1394_VIDEO_MODE_640x480_YUV411;
        } else if (sPF == "YUV422") {
            return DC1394_VIDEO_MODE_640x480_YUV422;
        } else if (sPF == "RGB") {
            return DC1394_VIDEO_MODE_640x480_RGB8;
        }
    } else if (Size.x == 800 && Size.y == 600) {
        if (sPF == "MONO8") {
            return DC1394_VIDEO_MODE_800x600_MONO8;
        } else if (sPF == "MONO16") {
            return DC1394_VIDEO_MODE_800x600_MONO16;
        } else if (sPF == "YUV422") {
            return DC1394_VIDEO_MODE_800x600_YUV422;
        } else if (sPF == "RGB") {
            return DC1394_VIDEO_MODE_800x600_RGB8;
        }
    } else if (Size.x == 1024 && Size.y == 768) {
        if (sPF == "MONO8") {
            return DC1394_VIDEO_MODE_1024x768_MONO8;
        } else if (sPF == "MONO16") {
            return DC1394_VIDEO_MODE_1024x768_MONO16;
        } else if (sPF == "YUV422") {
            return DC1394_VIDEO_MODE_1024x768_YUV422;
        } else if (sPF == "RGB") {
            return DC1394_VIDEO_MODE_1024x768_RGB8;
        }
    }
    AVG_TRACE (Logger::WARNING,
            std::string("getCamMode: Unsupported or illegal value for camera mode (") 
            << Size.x << ", " << Size.y << "), " << sPF << ".");
    return DC1394_VIDEO_MODE_640x480_RGB8;
}

dc1394framerate_t getFrameRateConst(double FrameRate)
{
    if (FrameRate == 1.875) {
        return DC1394_FRAMERATE_1_875;
    } else if (FrameRate == 3.75) {
        return DC1394_FRAMERATE_3_75;
    } else if (FrameRate == 7.5) {
        return DC1394_FRAMERATE_7_5;
    } else if (FrameRate == 15) {
        return DC1394_FRAMERATE_15;
    } else if (FrameRate == 30) {
        return DC1394_FRAMERATE_30;
    } else if (FrameRate == 60) {
        return DC1394_FRAMERATE_60;
    } else if (FrameRate == 120) {
        return DC1394_FRAMERATE_120;
    } else if (FrameRate == 240) {
        return DC1394_FRAMERATE_240;
    } else {
        AVG_TRACE (Logger::WARNING,
                std::string("Unsupported or illegal value for camera framerate."));
        return (dc1394framerate_t)-1;
    }
}

dc1394feature_t getFeatureID(CameraFeature Feature)
{
    switch(Feature) {
        case CAM_FEATURE_BRIGHTNESS:
            return DC1394_FEATURE_BRIGHTNESS;
        case CAM_FEATURE_EXPOSURE:
            return DC1394_FEATURE_EXPOSURE;
        case CAM_FEATURE_SHARPNESS:
            return DC1394_FEATURE_SHARPNESS;
        case CAM_FEATURE_WHITE_BALANCE:
            return DC1394_FEATURE_WHITE_BALANCE;
        case CAM_FEATURE_HUE:
            return DC1394_FEATURE_HUE;
        case CAM_FEATURE_SATURATION:
            return DC1394_FEATURE_SATURATION;
        case CAM_FEATURE_GAMMA:
            return DC1394_FEATURE_GAMMA;
        case CAM_FEATURE_SHUTTER:
            return DC1394_FEATURE_SHUTTER;
        case CAM_FEATURE_GAIN:
            return DC1394_FEATURE_GAIN;
        case CAM_FEATURE_IRIS:
            return DC1394_FEATURE_IRIS;
        case CAM_FEATURE_FOCUS:
            return DC1394_FEATURE_FOCUS;
        case CAM_FEATURE_TEMPERATURE:
            return DC1394_FEATURE_TEMPERATURE;
        case CAM_FEATURE_TRIGGER:
            return DC1394_FEATURE_TRIGGER;
        case CAM_FEATURE_ZOOM:
            return DC1394_FEATURE_ZOOM;
        case CAM_FEATURE_PAN:
            return DC1394_FEATURE_PAN;
        case CAM_FEATURE_TILT:
            return DC1394_FEATURE_TILT;
        case CAM_FEATURE_OPTICAL_FILTER:
            return DC1394_FEATURE_OPTICAL_FILTER;
        case CAM_FEATURE_CAPTURE_SIZE:
            return DC1394_FEATURE_CAPTURE_SIZE;
        case CAM_FEATURE_CAPTURE_QUALITY:
            return DC1394_FEATURE_CAPTURE_QUALITY;
        default:
            AVG_TRACE(Logger::WARNING, "getFeatureID: " << 
                    cameraFeatureToString(Feature) << " unknown.");
            return DC1394_FEATURE_MIN;
    }
}

#endif


}
