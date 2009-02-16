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

#include "Camera.h"

#include "../base/Logger.h"
#include "../base/Exception.h"

#include <cstdlib>

//alright. I know this looks strange. Don't argue
#if defined(AVG_ENABLE_1394_2) || defined(AVG_ENABLE_1394)
#include "../imaging/FWCamera.h"
#endif
#ifdef AVG_ENABLE_V4L2
#include "../imaging/V4LCamera.h"
#endif
#ifdef AVG_ENABLE_CMU1394
#include "../imaging/CMUCamera.h"
#endif
#ifdef AVG_ENABLE_DSHOW
#include "../imaging/DSCamera.h"
#endif
#include "../imaging/FakeCamera.h"


namespace avg {

std::string cameraFeatureToString(CameraFeature Feature)
{
    switch(Feature) {
        case CAM_FEATURE_BRIGHTNESS:
            return "brightness";
        case CAM_FEATURE_EXPOSURE:
            return "exposure";
        case CAM_FEATURE_SHARPNESS:
            return "sharpness";
        case CAM_FEATURE_WHITE_BALANCE:
            return "white balance";
        case CAM_FEATURE_HUE:
            return "hue";
        case CAM_FEATURE_SATURATION:
            return "saturation";
        case CAM_FEATURE_GAMMA:
            return "gamma";
        case CAM_FEATURE_SHUTTER:
            return "shutter";
        case CAM_FEATURE_GAIN:
            return "gain";
        case CAM_FEATURE_IRIS:
            return "iris";
        case CAM_FEATURE_FOCUS:
            return "focus";
        case CAM_FEATURE_TEMPERATURE:
            return "temperature";
        case CAM_FEATURE_TRIGGER:
            return "trigger";
        case CAM_FEATURE_ZOOM:
            return "zoom";
        case CAM_FEATURE_PAN:
            return "pan";
        case CAM_FEATURE_TILT:
            return "tilt";
        case CAM_FEATURE_OPTICAL_FILTER:
            return "optical filter";
        case CAM_FEATURE_CAPTURE_SIZE:
            return "capture size";
        case CAM_FEATURE_CAPTURE_QUALITY:
            return "capture quality";
        case CAM_FEATURE_CONTRAST:
            return "contrast";
        case CAM_FEATURE_STROBE_DURATION:
            return "strobe duration";
        default:
            return "unknown";
    }
}

CameraPtr getCamera(const std::string& sSource, const std::string& sDevice, const std::string& sChannel, const IntPoint& CaptureSize, const std::string& sCaptureFormat, double FrameRate) {

    CameraPtr pCamera;
    try {
        if (sSource == "firewire" || sSource == "fw") {
#if defined(AVG_ENABLE_1394)\
        || defined(AVG_ENABLE_1394_2)
            //IFIXME parse sChannel and extract guid/unit
        char *dummy;
        pCamera = CameraPtr(new FWCamera(sDevice, strtoll(sChannel.c_str(),&dummy,10), -1, CaptureSize, sCaptureFormat, 
                FrameRate, true));
#elif defined(AVG_ENABLE_CMU1394)
        pCamera = CameraPtr(new CMUCamera(sDevice, CaptureSize, sCaptureFormat, 
                FrameRate, true));
#else
            AVG_TRACE(Logger::WARNING, "Firewire camera specified, but firewire "
                    "support not compiled in.");
#endif
        } else if (sSource == "v4l") {
#if defined(AVG_ENABLE_V4L2)
            char *dummy;
            int Channel = strtol(sChannel.c_str(), &dummy, 10);
            pCamera = CameraPtr(new V4LCamera(sDevice, Channel,
                CaptureSize, sCaptureFormat, true));
#else
            AVG_TRACE(Logger::WARNING, "Video4Linux camera specified, but "
                    "Video4Linux support not compiled in.");
#endif
        } else if (sSource == "directshow") {
#if defined(AVG_ENABLE_DSHOW)
            pCamera = CameraPtr(new DSCamera(sDevice, CaptureSize, sCaptureFormat, 
                FrameRate, true));
#else
            AVG_TRACE(Logger::WARNING, "DirectShow camera specified, but "
                    "DirectShow is only available under windows.");
#endif
        } else {
            throw Exception(AVG_ERR_INVALID_ARGS,
                    "Unable to set up camera. Camera source '"+sSource+"' unknown.");
        }
    } catch (const Exception &e) {
        AVG_TRACE(Logger::WARNING, e.GetStr());

    }
    if (!pCamera){
        pCamera = CameraPtr(new FakeCamera());
    }
    return pCamera;

}

}
