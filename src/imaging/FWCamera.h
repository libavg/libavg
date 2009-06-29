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

#ifndef _FWCamera_H_
#define _FWCamera_H_

#include "../avgconfigwrapper.h"

#include "Camera.h"

#include "../graphics/Bitmap.h"
#include "../graphics/Pixel24.h"

#include "../base/Point.h"
#include "../base/Queue.h"
#include "../base/WorkerThread.h"

#ifdef AVG_ENABLE_1394_2
#include <dc1394/control.h>
#include <dc1394/register.h>
#endif
#ifndef AVG_ENABLE_1394_2
typedef unsigned int dc1394feature_t;
#endif
#ifndef uint64_t
#define uint64_t unsigned long long
#endif

#include <string>
#include <map>

namespace avg {

typedef Queue<BitmapPtr> BitmapQueue;

class AVG_API FWCamera: public Camera {
public:
    FWCamera(uint64_t guid, int unit, bool bFW800, IntPoint Size, PixelFormat camPF, 
            PixelFormat destPF, double FrameRate);
    virtual ~FWCamera();

    virtual IntPoint getImgSize();
    virtual BitmapPtr getImage(bool bWait);

    virtual const std::string& getDevice() const; 
    virtual const std::string& getDriverName() const; 
    virtual double getFrameRate() const;

    virtual int getFeature(CameraFeature Feature) const;
    virtual void setFeature(CameraFeature Feature, int Value, bool bIgnoreOldValue=false);
    virtual void setFeatureOneShot(CameraFeature Feature);
    virtual int getWhitebalanceU() const;
    virtual int getWhitebalanceV() const;
    virtual void setWhitebalance(int u, int v, bool bIgnoreOldValue=false);

    static void dumpCameras();

private:
    void setFeature(dc1394feature_t Feature, int Value);
    void setStrobeDuration(int microsecs);
    void getWhitebalance(int* pU, int* pV) const;
    void enablePtGreyBayer();

    IntPoint m_Size;
    double m_FrameRate;

#ifdef AVG_ENABLE_1394_2
    dc1394_t * m_pDC1394;
    dc1394camera_t * m_pCamera;
    dc1394framerate_t m_FrameRateConstant; 
    dc1394video_mode_t m_Mode;            
#endif
    void dumpCameraInfo();

    FeatureMap m_Features;
    int m_WhitebalanceU;
    int m_WhitebalanceV;
};

}

#endif
