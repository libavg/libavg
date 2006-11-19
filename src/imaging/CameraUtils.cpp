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

#include "CameraUtils.h"

#include "../base/Logger.h"

namespace avg {

using namespace std;

#ifdef AVG_ENABLE_1394
int getCamMode(string sMode) 
{
/*    if (sMode == "160x120_YUV444") {
         return MODE_160x120_YUV444;
    } else if (sMode == "320x240_YUV422") {
         return MODE_320x240_YUV422;
    } else
*/
    if (sMode == "640x480_MONO8") {
         return MODE_640x480_MONO;
    } else if (sMode == "640x480_YUV411") {
         return MODE_640x480_YUV411;
    } else if (sMode == "640x480_YUV422") {
         return MODE_640x480_YUV422;
    } else if (sMode == "640x480_RGB") {
         return MODE_640x480_RGB;
/*    } else if (sMode == "640x480_MONO16") {
         return MODE_640x480_MONO16;
*/        
    } else if (sMode == "1024x768_RGB") {
         return MODE_1024x768_RGB;
    } else if (sMode == "1024x768_YUV422") {
         return MODE_1024x768_YUV422;
    } else {
        AVG_TRACE (Logger::WARNING,
                std::string("Unsupported or illegal value for camera mode \"") 
                + sMode + std::string("\"."));
         return MODE_640x480_RGB;
    }
}

IntPoint getCamImgSize(int Mode)
{
    switch(Mode) {
        case MODE_640x480_MONO:
        case MODE_640x480_YUV411:
        case MODE_640x480_YUV422:
        case MODE_640x480_RGB:
            return IntPoint(640, 480);
        case MODE_1024x768_RGB:
        case MODE_1024x768_YUV422:
            return IntPoint(1024, 768);
        default:
            AVG_TRACE (Logger::WARNING,
                    std::string("getImgSize: Unsupported or illegal value for camera mode "));
            return IntPoint(0,0);
    }
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
        AVG_TRACE (Logger::WARNING,
                std::string("Unsupported or illegal value for camera framerate."));
        return FRAMERATE_30;
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
/*    if (sMode == "160x120_YUV444") {
         return DC1394_VIDEO_MODE_160x120_YUV444;
    } else if (sMode == "320x240_YUV422") {
         return DC1394_VIDEO_MODE_320x240_YUV422;
    } else
*/
    if (sMode == "640x480_MONO8") {
         return DC1394_VIDEO_MODE_640x480_MONO8;
    } else if (sMode == "640x480_YUV411") {
         return DC1394_VIDEO_MODE_640x480_YUV411;
    } else if (sMode == "640x480_YUV422") {
         return DC1394_VIDEO_MODE_640x480_YUV422;
    } else if (sMode == "640x480_RGB") {
         return DC1394_VIDEO_MODE_640x480_RGB8;
/*  } else if (sMode == "640x480_MONO16") {
         return DC1394_VIDEO_MODE_640x480_MONO16;
*/        
    } else if (sMode == "1024x768_RGB") {
         return DC1394_VIDEO_MODE_1024x768_RGB8;
    } else if (sMode == "1024x768_YUV422") {
         return DC1394_VIDEO_MODE_1024x768_YUV422;
    } else {
        AVG_TRACE (Logger::WARNING,
                std::string("getMode: Unsupported or illegal value for camera mode \"") 
                + sMode + std::string("\"."));
        return DC1394_VIDEO_MODE_640x480_RGB8;
    }
}

IntPoint getCamImgSize(dc1394video_mode_t Mode)
{
    switch(Mode) {
        case DC1394_VIDEO_MODE_640x480_MONO8:
        case DC1394_VIDEO_MODE_640x480_YUV411:
        case DC1394_VIDEO_MODE_640x480_YUV422:
        case DC1394_VIDEO_MODE_640x480_RGB8:
            return IntPoint(640, 480);
        case DC1394_VIDEO_MODE_1024x768_RGB8:
        case DC1394_VIDEO_MODE_1024x768_YUV422:
            return IntPoint(1024, 768);
        default:
            AVG_TRACE (Logger::WARNING,
                    std::string("getImgSize: Unsupported or illegal value for camera mode "));
            return IntPoint(0,0);
    }
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
        return DC1394_FRAMERATE_30;
    }
}

#endif


}
