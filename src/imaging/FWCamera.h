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

#ifdef AVG_ENABLE_1394
#include <libraw1394/raw1394.h>
#include <libdc1394/dc1394_control.h>
#endif
#ifdef AVG_ENABLE_1394_2
#include <dc1394/control.h>
#include <dc1394/register.h>
#endif
#ifndef AVG_ENABLE_1394_2
typedef unsigned int dc1394feature_t;
#endif

#include <string>
#include <map>

namespace avg {

typedef Queue<BitmapPtr> BitmapQueue;

class FWCamera: public Camera {
public:
    FWCamera(std::string sDevice, IntPoint Size, std::string sPF,
            double FrameRate, bool bColor);
    virtual ~FWCamera();
    virtual void open();
    virtual void close();

    virtual IntPoint getImgSize();
    virtual BitmapPtr getImage(bool bWait);
    virtual bool isCameraAvailable();

    virtual const std::string& getDevice() const; 
    virtual const std::string& getDriverName() const; 
    virtual double getFrameRate() const;

    virtual unsigned int getFeature(CameraFeature Feature) const;
    virtual void setFeature(CameraFeature Feature, int Value);
    void setFeature(dc1394feature_t Feature, int Value);

private:
    void setStrobeDuration(int microsecs);

    std::string m_sDevice;
    std::string m_sPF;
    IntPoint m_Size;
    double m_FrameRate;
    bool m_bColor;

#ifdef AVG_ENABLE_1394
    bool findCameraOnPort(int port, raw1394handle_t& FWHandle);

    dc1394_cameracapture m_Camera;
    raw1394handle_t m_FWHandle;
    int m_FrameRateConstant;  // libdc1394 constant for framerate.
    int m_Mode;               // libdc1394 constant for mode.
#elif AVG_ENABLE_1394_2
    dc1394_t * m_pDC1394;
    dc1394camera_t * m_pCamera;
    dc1394framerate_t m_FrameRateConstant; 
    dc1394video_mode_t m_Mode;            
#endif
    void checkDC1394Error(int Code, const std::string & sMsg);
    void fatalError(const std::string & sMsg);
    void dumpCameraInfo();

    bool m_bCameraAvailable;
    typedef std::map<dc1394feature_t, int> FeatureMap;
    FeatureMap m_Features;
    int m_StrobeDuration;
};

}

#endif
