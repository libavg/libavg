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

#ifndef _CMUCamera_H_
#define _CMUCamera_H_

#include "../api.h"
#include "Camera.h"

#include <windows.h>
#include <1394Camera.h>

#include <string>

namespace avg {

class CMUCamera : public Camera {
public:
    CMUCamera(std::string sDevice, IntPoint Size, std::string sPF,
            double FrameRate, bool bColor);
    virtual ~CMUCamera();
    virtual void open();
    virtual void close();

    virtual IntPoint getImgSize();
    virtual BitmapPtr getImage(bool bWait);
    virtual bool isCameraAvailable();

    virtual const std::string& getDevice() const; 
    virtual const std::string& getDriverName() const; 
    virtual double getFrameRate() const;

    virtual unsigned int getFeature(CameraFeature Feature) const;
    virtual void setFeature(CameraFeature Feature, int Value, bool bIgnoreOldValue=false);

private:
    void fatalError(const std::string& sMsg);

    bool m_bCameraAvailable;
    std::string m_sDevice;
    std::string m_sPF;
    IntPoint m_Size;
    double m_FrameRate;
    bool m_bFlipRGB;

    PixelFormat m_FramePixelFormat;
    PixelFormat m_OutputPixelFormat;

    C1394Camera m_Camera;
    FeatureMap m_Features;
};

}

#endif
