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

#include "FWCameraUtils.h"

#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/StringHelper.h"

namespace avg {

using namespace std;

dc1394video_mode_t getCamMode(IntPoint Size, PixelFormat pf) 
{
    if (Size.x == 320 && Size.y == 240 && pf == YCbCr422) {
        return DC1394_VIDEO_MODE_320x240_YUV422;
    } else if (Size.x == 640 && Size.y == 480) {
        switch(pf) {
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
    } else if (Size.x == 800 && Size.y == 600) {
        switch(pf) {
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
    } else if (Size.x == 1024 && Size.y == 768) {
        switch(pf) {
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
    } else if (Size.x == 1280 && Size.y == 960) {
        switch(pf) {
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
    } else if (Size.x == 1600 && Size.y == 1200) {
        switch(pf) {
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
            "Unsupported or illegal value ("+toString(Size.x)+", "+toString(Size.y)+
            "), "+Bitmap::getPixelFormatString(pf)+"\" for camera mode.");
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
        throw Exception(AVG_ERR_CAMERA_FATAL, string("Illegal value ")
                +toString(FrameRate)+" for camera framerate.");
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
            AVG_ASSERT(false);
            return dc1394feature_t(0);
    }
}

}
