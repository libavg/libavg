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

int getFeatureID(const std::string& sFeature)
{
    if (sFeature == "brightness") {
        return FEATURE_BRIGHTNESS;
    } else if (sFeature == "exposure") {
        return FEATURE_EXPOSURE;
    } else if (sFeature == "sharpness") {
        return FEATURE_SHARPNESS;
    } else if (sFeature == "whitebalance") {
        return FEATURE_WHITE_BALANCE;
    } else if (sFeature == "hue") {
        return FEATURE_HUE;
    } else if (sFeature == "saturation") {
        return FEATURE_SATURATION;
    } else if (sFeature == "gamma") {
        return FEATURE_GAMMA;
    } else if (sFeature == "shutter") {
        return FEATURE_SHUTTER;
    } else if (sFeature == "gain") {
        return FEATURE_GAIN;
    } else if (sFeature == "iris") {
        return FEATURE_IRIS;
    } else if (sFeature == "focus") {
        return FEATURE_FOCUS;
    } else if (sFeature == "temperature") {
        return FEATURE_TEMPERATURE;
    } else if (sFeature == "trigger") {
        return FEATURE_TRIGGER;
    } else if (sFeature == "zoom") {
        return FEATURE_ZOOM;
    } else if (sFeature == "pan") {
        return FEATURE_PAN;
    } else if (sFeature == "tilt") {
        return FEATURE_TILT;
    } else if (sFeature == "optical_filter") {
        return FEATURE_OPTICAL_FILTER;
    } else if (sFeature == "capture_size") {
        return FEATURE_CAPTURE_SIZE;
    } else if (sFeature == "capture_quality") {
        return FEATURE_CAPTURE_QUALITY;
    }
    AVG_TRACE(Logger::WARNING, "getFeatureID: "+sFeature+" unknown.");
    return 0;
}

#else

dc1394video_mode_t getCamMode(string sMode) 
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
            return DC1394_VIDEO_MODE_640x480_RGB;
        }
    } else if (Size.x == 800 && Size.y == 600) {
        if (sPF == "MONO8") {
            return DC1394_VIDEO_MODE_800x600_MONO8;
        } else if (sPF == "MONO16") {
            return DC1394_VIDEO_MODE_800x600_MONO16;
        } else if (sPF == "YUV422") {
            return DC1394_VIDEO_MODE_800x600_YUV422;
        } else if (sPF == "RGB") {
            return DC1394_VIDEO_MODE_800x600_RGB;
        }
    } else if (Size.x == 1024 && Size.y == 768) {
        if (sPF == "MONO8") {
            return DC1394_VIDEO_MODE_1024x768_MONO8;
        } else if (sPF == "MONO16") {
            return DC1394_VIDEO_MODE_1024x768_MONO16;
        } else if (sPF == "YUV422") {
            return DC1394_VIDEO_MODE_1024x768_YUV422;
        } else if (sPF == "RGB") {
            return DC1394_VIDEO_MODE_1024x768_RGB;
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

dc1394feature_t getFeatureID(const std::string& sFeature)
{
    if (sFeature == "brightness") {
        return DC1394_FEATURE_BRIGHTNESS;
    } else if (sFeature == "exposure") {
        return DC1394_FEATURE_EXPOSURE;
    } else if (sFeature == "sharpness") {
        return DC1394_FEATURE_SHARPNESS;
    } else if (sFeature == "whitebalance") {
        return DC1394_FEATURE_WHITE_BALANCE;
    } else if (sFeature == "hue") {
        return DC1394_FEATURE_HUE;
    } else if (sFeature == "saturation") {
        return DC1394_FEATURE_SATURATION;
    } else if (sFeature == "gamma") {
        return DC1394_FEATURE_GAMMA;
    } else if (sFeature == "shutter") {
        return DC1394_FEATURE_SHUTTER;
    } else if (sFeature == "gain") {
        return DC1394_FEATURE_GAIN;
    } else if (sFeature == "iris") {
        return DC1394_FEATURE_IRIS;
    } else if (sFeature == "focus") {
        return DC1394_FEATURE_FOCUS;
    } else if (sFeature == "temperature") {
        return DC1394_FEATURE_TEMPERATURE;
    } else if (sFeature == "trigger") {
        return DC1394_FEATURE_TRIGGER;
    } else if (sFeature == "zoom") {
        return DC1394_FEATURE_ZOOM;
    } else if (sFeature == "pan") {
        return DC1394_FEATURE_PAN;
    } else if (sFeature == "tilt") {
        return DC1394_FEATURE_TILT;
    } else if (sFeature == "optical_filter") {
        return DC1394_FEATURE_OPTICAL_FILTER;
    } else if (sFeature == "capture_size") {
        return DC1394_FEATURE_CAPTURE_SIZE;
    } else if (sFeature == "capture_quality") {
        return DC1394_FEATURE_CAPTURE_QUALITY;
    }
    AVG_TRACE(Logger::WARNING, "getFeatureID: "+sFeature+" unknown.");
    return DC1394_FEATURE_MIN;
}

#endif


}
