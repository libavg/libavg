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

#ifndef _ICamera_H_
#define _ICamera_H_

#include "../graphics/Bitmap.h"

#include <boost/shared_ptr.hpp>

#include <string>

namespace avg {

enum CameraFeature {
    CAM_FEATURE_BRIGHTNESS,
    CAM_FEATURE_EXPOSURE,
    CAM_FEATURE_SHARPNESS,
    CAM_FEATURE_WHITE_BALANCE,
    CAM_FEATURE_HUE,
    CAM_FEATURE_SATURATION,
    CAM_FEATURE_GAMMA,
    CAM_FEATURE_SHUTTER,
    CAM_FEATURE_GAIN,
    CAM_FEATURE_IRIS,
    CAM_FEATURE_FOCUS,
    CAM_FEATURE_TEMPERATURE,
    CAM_FEATURE_TRIGGER,
    CAM_FEATURE_ZOOM,
    CAM_FEATURE_PAN,
    CAM_FEATURE_TILT,
    CAM_FEATURE_OPTICAL_FILTER,
    CAM_FEATURE_CAPTURE_SIZE,
    CAM_FEATURE_CAPTURE_QUALITY
};

std::string cameraFeatureToString(CameraFeature Feature);

class ICamera
{
    public:
        virtual ~ICamera() {};
        virtual void open() = 0;
        virtual void close() = 0;

        virtual IntPoint getImgSize() = 0;
        virtual BitmapPtr getImage(bool bWait) = 0;
        virtual bool isCameraAvailable() = 0;

        virtual const std::string& getDevice() const = 0; 
        virtual const std::string& getDriverName() const = 0; 
        virtual double getFrameRate() const = 0;
        
        virtual unsigned int getFeature(CameraFeature Feature) const = 0;
        virtual void setFeature(CameraFeature Feature, int Value) = 0;
};

typedef boost::shared_ptr<ICamera> CameraPtr;

}

#endif

