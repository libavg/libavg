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

const char * VideoModesToString(int mode)
{
    switch (mode) {
        case 64:
            return "    YUV444  (160, 120)   ";
        case 65:
            return "    YUV422  (320, 240)   ";
        case 66:
            return "    YUV411  (640, 480)   ";
        case 67:
            return "    YUV422  (640, 480)   ";
        case 68:
            return "    RGB     (640, 480)   ";
        case 69:
            return "    I8      (640, 480)   ";
        case 70:
            return "    I16     (640, 480)   ";
        case 71:
            return "    YUV422  (800, 600)   ";
        case 72:
            return "    RGB     (800, 600)   ";
        case 73:
            return "    I8      (800, 600)   ";
        case 74:
            return "    YUV422  (1024, 768)  ";
        case 75:
            return "    RGB     (1024, 768)  ";
        case 76:
            return "    I8      (1024, 768)  ";
        case 77:
            return "    I16     (800, 600)   ";
        case 78:
            return "    I16     (1024, 768)  ";
        case 79:
            return "    YUV422  (1280, 960)  ";
        case 80:
            return "    RGB     (1280, 960)  ";
        case 81:
            return "    I8      (1280, 960)  ";
        case 82:
            return "    YUV422  (1600, 1200) ";
        case 83:
            return "    RGB     (1600, 1200) ";
        case 84:
            return "    I8      (1600, 1200) ";
        case 85:
            return "    I16     (1280, 960)  ";
        case 86:
            return "    I16     (1600, 1200) ";
        case 87:
            return "DC1394_VIDEO_MODE_EXIF";
        case 88:
            return "DC1394_VIDEO_MODE_FORMAT7_0";
        case 89:
            return "DC1394_VIDEO_MODE_FORMAT7_1";
        case 90:
            return "DC1394_VIDEO_MODE_FORMAT7_2";
        case 91:
            return "DC1394_VIDEO_MODE_FORMAT7_3";
        case 92:
            return "DC1394_VIDEO_MODE_FORMAT7_4";
        case 93:
            return "DC1394_VIDEO_MODE_FORMAT7_5";
        case 94:
            return "DC1394_VIDEO_MODE_FORMAT7_6";
        case 95:
            return "DC1394_VIDEO_MODE_FORMAT7_7";
    }
}

const char * FrameRatesToString(int frameRate)
{
    switch (frameRate) {
        case 32:
            return "1.875";
        case 33:
            return "3.75";
        case 34:
            return "7.5";
        case 35:
            return "15";
        case 36:
            return "30";
        case 37:
            return "60";
        case 38:
            return "120";
        case 39:
            return "240";
    }
}


}
