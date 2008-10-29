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

}
