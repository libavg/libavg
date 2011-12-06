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

#ifndef _V4LCamera_H_
#define _V4LCamera_H_

#include "../api.h"
#include "../avgconfigwrapper.h"

#include "Camera.h"
#include <string>
#include <vector>

namespace avg {

typedef unsigned int V4LCID_t;

class AVG_API V4LCamera: public Camera {

    struct Buffer {
        void * start;
        size_t length;
    };

public:
    V4LCamera(std::string sDevice, int channel, IntPoint size, PixelFormat camPF,
            PixelFormat destPF, float frameRate);
    virtual ~V4LCamera();

    virtual BitmapPtr getImage(bool bWait);
    virtual bool isCameraAvailable();

    virtual const std::string& getDevice() const;
    virtual const std::string& getDriverName() const;

    virtual int getFeature(CameraFeature feature) const;
    virtual void setFeature(CameraFeature feature, int value,
            bool bIgnoreOldValue=false);
    virtual void setFeatureOneShot(CameraFeature feature);
    virtual int getWhitebalanceU() const;
    virtual int getWhitebalanceV() const;
    virtual void setWhitebalance(int u, int v, bool bIgnoreOldValue=false);

    static CameraInfo* getCameraInfos(int deviceNumber);
    static int countCameras();

private:
    void initDevice();
    void startCapture();
    void initMMap();
    virtual void close();

    int getV4LPF(PixelFormat pf);
    static int checkCamera(int j);
    static PixelFormat intToPixelFormat(unsigned int pixelformat);

    static void getCameraImageFormats(int fd, CameraInfo* camInfo);
    static void getCameraControls(int deviceNumber, CameraInfo* camInfo);

    void setFeature(V4LCID_t v4lFeature, int value);
    V4LCID_t getFeatureID(CameraFeature feature) const;
    std::string getFeatureName(V4LCID_t v4lFeature);
    bool isFeatureSupported(V4LCID_t v4lFeature) const;
    typedef std::map<V4LCID_t, unsigned int> FeatureMap;
    typedef std::map<int, std::string> FeatureNamesMap;
    FeatureMap m_Features;
    // TODO: Feature strings should really be handled by
    //       Camera::cameraFeatureToString
    FeatureNamesMap m_FeaturesNames;

    int m_Fd;
    int m_Channel;
    std::string m_sDevice;
    std::string m_sDriverName;
    std::vector<Buffer> m_vBuffers;
    bool m_bCameraAvailable;
    int m_v4lPF;
};

}

#endif
