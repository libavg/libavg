//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2011 Ulrich von Zadow
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
#include "../base/Exception.h"
#include "../base/StringHelper.h"

namespace avg {

using namespace std;

dc1394video_mode_t getCamMode(IntPoint size, PixelFormat pf) 
{
    if (size.x == 320 && size.y == 240 && pf == YCbCr422) {
        return DC1394_VIDEO_MODE_320x240_YUV422;
    } else if (size.x == 640 && size.y == 480) {
        switch (pf) {
            case I8:
            case BAYER8:
                return DC1394_VIDEO_MODE_640x480_MONO8;
            case I16:
                return DC1394_VIDEO_MODE_640x480_MONO16;
            case YCbCr411:
                return DC1394_VIDEO_MODE_640x480_YUV411;
            case YCbCr422:
                return DC1394_VIDEO_MODE_640x480_YUV422;
            case R8G8B8:
                return DC1394_VIDEO_MODE_640x480_RGB8;
            default:
                break;
        }
    } else if (size.x == 800 && size.y == 600) {
        switch (pf) {
            case I8:
            case BAYER8:
                return DC1394_VIDEO_MODE_800x600_MONO8;
            case I16:
                return DC1394_VIDEO_MODE_800x600_MONO16;
            case YCbCr422:
                return DC1394_VIDEO_MODE_800x600_YUV422;
            case R8G8B8:
                return DC1394_VIDEO_MODE_800x600_RGB8;
            default:
                break;
        }
    } else if (size.x == 1024 && size.y == 768) {
        switch (pf) {
            case I8:
            case BAYER8:
                return DC1394_VIDEO_MODE_1024x768_MONO8;
            case I16:
                return DC1394_VIDEO_MODE_1024x768_MONO16;
            case YCbCr422:
                return DC1394_VIDEO_MODE_1024x768_YUV422;
            case R8G8B8:
                return DC1394_VIDEO_MODE_1024x768_RGB8;
            default:
                break;
        }
    } else if (size.x == 1280 && size.y == 960) {
        switch (pf) {
            case I8:
            case BAYER8:
                return DC1394_VIDEO_MODE_1280x960_MONO8;
            case I16:
                return DC1394_VIDEO_MODE_1280x960_MONO16;
            case YCbCr422:
                return DC1394_VIDEO_MODE_1280x960_YUV422;
            case R8G8B8:
                return DC1394_VIDEO_MODE_1280x960_RGB8;
            default:
                break;
        }
    } else if (size.x == 1600 && size.y == 1200) {
        switch (pf) {
            case I8:
            case BAYER8:
                return DC1394_VIDEO_MODE_1600x1200_MONO8;
            case I16:
                return DC1394_VIDEO_MODE_1600x1200_MONO16;
            case YCbCr422:
                return DC1394_VIDEO_MODE_1600x1200_YUV422;
            case R8G8B8:
                return DC1394_VIDEO_MODE_1600x1200_RGB8;
            default:
                break;
        }
    }
    throw Exception(AVG_ERR_CAMERA_FATAL,
            "Unsupported or illegal value ("+toString(size.x)+", "+toString(size.y)+
            "), "+getPixelFormatString(pf)+"\" for camera mode.");
}

dc1394framerate_t getFrameRateConst(double frameRate)
{
    if (frameRate == 1.875) {
        return DC1394_FRAMERATE_1_875;
    } else if (frameRate == 3.75) {
        return DC1394_FRAMERATE_3_75;
    } else if (frameRate == 7.5) {
        return DC1394_FRAMERATE_7_5;
    } else if (frameRate == 15) {
        return DC1394_FRAMERATE_15;
    } else if (frameRate == 30) {
        return DC1394_FRAMERATE_30;
    } else if (frameRate == 60) {
        return DC1394_FRAMERATE_60;
    } else if (frameRate == 120) {
        return DC1394_FRAMERATE_120;
    } else if (frameRate == 240) {
        return DC1394_FRAMERATE_240;
    } else {
        throw Exception(AVG_ERR_CAMERA_FATAL, string("Illegal value ")
                +toString(frameRate)+" for camera framerate.");
    }
}
CameraFeature featureIDToEnum(dc1394feature_t feature)
{
    switch (feature) {
        case DC1394_FEATURE_BRIGHTNESS:
            return CAM_FEATURE_BRIGHTNESS;
        case DC1394_FEATURE_EXPOSURE:
            return CAM_FEATURE_EXPOSURE;
        case DC1394_FEATURE_SHARPNESS:
            return CAM_FEATURE_SHARPNESS;
        case DC1394_FEATURE_WHITE_BALANCE:
            return CAM_FEATURE_WHITE_BALANCE;
        case DC1394_FEATURE_HUE:
            return CAM_FEATURE_HUE;
        case DC1394_FEATURE_SATURATION:
            return CAM_FEATURE_SATURATION;
        case DC1394_FEATURE_GAMMA:
            return CAM_FEATURE_GAMMA;
        case DC1394_FEATURE_SHUTTER:
            return CAM_FEATURE_SHUTTER;
        case DC1394_FEATURE_GAIN:
            return CAM_FEATURE_GAIN;
        case DC1394_FEATURE_IRIS:
            return CAM_FEATURE_IRIS;
        case DC1394_FEATURE_FOCUS:
            return CAM_FEATURE_FOCUS;
        case DC1394_FEATURE_TEMPERATURE:
            return CAM_FEATURE_TEMPERATURE;
        case DC1394_FEATURE_TRIGGER:
            return CAM_FEATURE_TRIGGER;
        case DC1394_FEATURE_TRIGGER_DELAY:
            return CAM_FEATURE_TRIGGER_DELAY;
        case DC1394_FEATURE_WHITE_SHADING:
            return CAM_FEATURE_WHITE_SHADING;
        case DC1394_FEATURE_ZOOM:
            return CAM_FEATURE_ZOOM;
        case DC1394_FEATURE_PAN:
            return CAM_FEATURE_PAN;
        case DC1394_FEATURE_TILT:
            return CAM_FEATURE_TILT;
        case DC1394_FEATURE_OPTICAL_FILTER:
            return CAM_FEATURE_OPTICAL_FILTER;
        case DC1394_FEATURE_CAPTURE_SIZE:
            return CAM_FEATURE_CAPTURE_SIZE;
        case DC1394_FEATURE_CAPTURE_QUALITY:
            return CAM_FEATURE_CAPTURE_QUALITY;
        default:
            return CAM_FEATURE_UNSUPPORTED;
    }
}

dc1394feature_t getFeatureID(CameraFeature feature)
{
    switch (feature) {
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
        case CAM_FEATURE_TRIGGER_DELAY:
            return DC1394_FEATURE_TRIGGER_DELAY;
        case CAM_FEATURE_WHITE_SHADING:
            return DC1394_FEATURE_WHITE_SHADING;
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
            AVG_ASSERT(false);
            return dc1394feature_t(0);
    }
}

IntPoint getFrameSizeFromVideoMode(dc1394video_mode_t mode)
{
    IntPoint point = IntPoint();
    point.x = -1;
    point.y = -1;
    switch (mode) {
        case DC1394_VIDEO_MODE_160x120_YUV444: {
            point.x = 160;
            point.y = 120;
            return point;
        }
        case DC1394_VIDEO_MODE_320x240_YUV422: {
            point.x = 320;
            point.y = 240;
            return point;
        }
        case DC1394_VIDEO_MODE_640x480_YUV411:
        case DC1394_VIDEO_MODE_640x480_YUV422:
        case DC1394_VIDEO_MODE_640x480_RGB8:
        case DC1394_VIDEO_MODE_640x480_MONO8:
        case DC1394_VIDEO_MODE_640x480_MONO16: {
            point.x = 640;
            point.y = 480;
            return point;
        }
        case DC1394_VIDEO_MODE_800x600_YUV422:
        case DC1394_VIDEO_MODE_800x600_RGB8:
        case DC1394_VIDEO_MODE_800x600_MONO8:
        case DC1394_VIDEO_MODE_800x600_MONO16: {
            point.x = 800;
            point.y = 600;
            return point;
        }
        case DC1394_VIDEO_MODE_1024x768_YUV422:
        case DC1394_VIDEO_MODE_1024x768_RGB8:
        case DC1394_VIDEO_MODE_1024x768_MONO8:
        case DC1394_VIDEO_MODE_1024x768_MONO16: {
            point.x = 1024;
            point.y = 768;
            return point;
        }


        case DC1394_VIDEO_MODE_1280x960_YUV422:
        case DC1394_VIDEO_MODE_1280x960_RGB8:
        case DC1394_VIDEO_MODE_1280x960_MONO8:
        case DC1394_VIDEO_MODE_1280x960_MONO16: {
            point.x = 1280;
            point.y = 960;
            return point;
        }
        case DC1394_VIDEO_MODE_1600x1200_YUV422:
        case DC1394_VIDEO_MODE_1600x1200_RGB8:
        case DC1394_VIDEO_MODE_1600x1200_MONO8:
        case DC1394_VIDEO_MODE_1600x1200_MONO16: {
            point.x = 1600;
            point.y = 1200;
            return point;
        }
        default:
            AVG_ASSERT(false);
            return point;
    }
}

PixelFormat getPFFromVideoMode(dc1394video_mode_t mode)
{
    switch (mode) {
        case DC1394_VIDEO_MODE_640x480_YUV411:
            return YCbCr411;
        case DC1394_VIDEO_MODE_320x240_YUV422:
        case DC1394_VIDEO_MODE_640x480_YUV422:
        case DC1394_VIDEO_MODE_800x600_YUV422:
        case DC1394_VIDEO_MODE_1024x768_YUV422:
        case DC1394_VIDEO_MODE_1280x960_YUV422:
        case DC1394_VIDEO_MODE_1600x1200_YUV422:
            return YCbCr422;
        case DC1394_VIDEO_MODE_640x480_RGB8:
        case DC1394_VIDEO_MODE_800x600_RGB8:
        case DC1394_VIDEO_MODE_1024x768_RGB8:
        case DC1394_VIDEO_MODE_1280x960_RGB8:
        case DC1394_VIDEO_MODE_1600x1200_RGB8:
            return R8G8B8;
        case DC1394_VIDEO_MODE_640x480_MONO8:
        case DC1394_VIDEO_MODE_800x600_MONO8:
        case DC1394_VIDEO_MODE_1024x768_MONO8:
        case DC1394_VIDEO_MODE_1280x960_MONO8:
        case DC1394_VIDEO_MODE_1600x1200_MONO8:
            return I8;
        case DC1394_VIDEO_MODE_640x480_MONO16:
        case DC1394_VIDEO_MODE_800x600_MONO16:
        case DC1394_VIDEO_MODE_1024x768_MONO16:
        case DC1394_VIDEO_MODE_1280x960_MONO16:
        case DC1394_VIDEO_MODE_1600x1200_MONO16:
            return I16;
        default:
            AVG_ASSERT(false);
            return PixelFormat(0);
    }
}

float framerateToFloat(dc1394framerate_t framerate)
{
    switch (framerate) {
        case DC1394_FRAMERATE_1_875:
            return 1.875;
        case DC1394_FRAMERATE_3_75:
            return 3.75;
        case DC1394_FRAMERATE_7_5:
            return 7.5;
        case DC1394_FRAMERATE_15:
            return 15;
        case DC1394_FRAMERATE_30:
            return 30;
        case DC1394_FRAMERATE_60:
            return 60;
        case DC1394_FRAMERATE_120:
            return 120;
        case DC1394_FRAMERATE_240:
            return 240;
        default:{
            AVG_ASSERT(false);
            return -1;
        }
    }
}


}
