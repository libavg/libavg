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

#include "Camera.h"

#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/ScopeTimer.h"
#include "../graphics/Filterfliprgb.h"

#if defined(AVG_ENABLE_1394_2)
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

#include <cstdlib>
#include <string.h>

#ifdef WIN32
#define strtoll(p, e, b) _strtoi64(p, e, b)
#endif

namespace avg {

using namespace std;

Camera::Camera(PixelFormat camPF, PixelFormat destPF, IntPoint size, float frameRate)
    : m_CamPF(camPF),
      m_DestPF(destPF),
      m_Size(size),
      m_FrameRate(frameRate)
{
//    cerr << "Camera: " << getPixelFormatString(camPF) << "-->" 
//        << getPixelFormatString(destPF) << endl;
}

PixelFormat Camera::getCamPF() const
{
    return m_CamPF;
}

void Camera::setCamPF(PixelFormat pf)
{
    m_CamPF = pf;
}

PixelFormat Camera::getDestPF() const
{
    return m_DestPF;
}

static ProfilingZoneID CameraConvertProfilingZone("Camera format conversion", true);

BitmapPtr Camera::convertCamFrameToDestPF(BitmapPtr pCamBmp)
{
    ScopeTimer Timer(CameraConvertProfilingZone);
    BitmapPtr pDestBmp = BitmapPtr(new Bitmap(pCamBmp->getSize(), m_DestPF));
    pDestBmp->copyPixels(*pCamBmp);
    if (m_CamPF == R8G8B8 && m_DestPF == B8G8R8X8) {
        pDestBmp->setPixelFormat(R8G8B8X8);
        FilterFlipRGB().applyInPlace(pDestBmp);
    }
    if (m_CamPF != R8G8B8 && m_DestPF == R8G8B8X8) {
        pDestBmp->setPixelFormat(B8G8R8X8);
        FilterFlipRGB().applyInPlace(pDestBmp);
    }

    return pDestBmp;
}

IntPoint Camera::getImgSize()
{
    return m_Size;
}

float Camera::getFrameRate() const
{
    return m_FrameRate;
}

PixelFormat Camera::fwBayerStringToPF(unsigned long reg)
{
    string sBayerFormat((char*)&reg, 4);
    if (sBayerFormat == "RGGB") {
        return BAYER8_RGGB;
    } else if (sBayerFormat == "GBRG") {
        return BAYER8_GBRG;
    } else if (sBayerFormat == "GRBG") {
        return BAYER8_GRBG;
    } else if (sBayerFormat == "BGGR") {
        return BAYER8_BGGR;
    } else if (sBayerFormat == "YYYY") {
        return I8;
    } else {
        AVG_ASSERT(false);
        return I8;
    }
}

void Camera::setImgSize(const IntPoint& size)
{
    m_Size = size;
}

string cameraFeatureToString(CameraFeature feature)
{
    switch (feature) {
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
        case CAM_FEATURE_TRIGGER_DELAY:
            return "trigger delay";
        case CAM_FEATURE_WHITE_SHADING:
            return "white shading";
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

CameraPtr createCamera(const string& sDriver, const string& sDevice, int unit,
        bool bFW800, const IntPoint& captureSize, PixelFormat camPF, PixelFormat destPF, 
        float frameRate)
{
    CameraPtr pCamera;
    try {
        if (sDriver == "firewire") {
            char * pszErr;
            long long guid = strtoll(sDevice.c_str(), &pszErr, 16);
            if (strlen(pszErr)) {
                throw Exception(AVG_ERR_INVALID_ARGS, "'"+sDevice
                        +"' is not a valid GUID.");
            }
#if defined(AVG_ENABLE_1394_2)
            pCamera = CameraPtr(new FWCamera(guid, unit, bFW800, captureSize, camPF, 
                    destPF, frameRate));
#elif defined(AVG_ENABLE_CMU1394)
            if (unit != -1) {
                throw Exception(AVG_ERR_INVALID_ARGS, 
                        "camera 'unit' attribute is not supported when using the cmu firewire driver.");
            }
            pCamera = CameraPtr(new CMUCamera(guid, bFW800, captureSize, camPF, destPF, 
                    frameRate));
#else
            (void)guid; // Silence compiler warning
            AVG_LOG_WARNING("Firewire camera specified, but firewire "
                    "support not compiled in.");
#endif
        } else if (sDriver == "video4linux") {
#if defined(AVG_ENABLE_V4L2)
            pCamera = CameraPtr(new V4LCamera(sDevice, unit, captureSize, camPF, 
                    destPF, frameRate));
#else
            AVG_LOG_WARNING("Video4Linux camera specified, but "
                    "Video4Linux support not compiled in.");
#endif
        } else if (sDriver == "directshow") {
#if defined(AVG_ENABLE_DSHOW)
            if (unit != -1) {
                throw Exception(AVG_ERR_INVALID_ARGS, 
                        "camera 'unit' attribute is not supported when using the directshow driver.");
            }
            pCamera = CameraPtr(new DSCamera(sDevice, captureSize, camPF, destPF,
                frameRate));
#else
            AVG_LOG_WARNING("DirectShow camera specified, but "
                    "DirectShow is only available under windows.");
#endif
        } else {
            throw Exception(AVG_ERR_INVALID_ARGS,
                    "Unable to set up camera. Camera source '"+sDriver+"' unknown.");
        }
    } catch (const Exception& e) {
        if (e.getCode() == AVG_ERR_CAMERA_NONFATAL) {
            AVG_LOG_WARNING(e.getStr());
        } else {
            throw;
        }

    }
    if (!pCamera) {
        pCamera = CameraPtr(new FakeCamera(camPF, destPF));
    }
    return pCamera;

}

std::vector<CameraInfo> getCamerasInfos()
{
    std::vector<CameraInfo> camerasInfo;
    
#ifdef AVG_ENABLE_1394_2
    int amountFWCameras = FWCamera::countCameras();
    for (int i = 0; i < amountFWCameras; i++) {
        CameraInfo* camInfo = FWCamera::getCameraInfos(i);
        if (camInfo != NULL) {
            camInfo->checkAddBayer8();
            camerasInfo.push_back(*camInfo);
        }
    }
#endif
#ifdef AVG_ENABLE_CMU1394
    int amountCMUCameras = CMUCamera::countCameras();
    for (int i = 0; i < amountCMUCameras; i++) {
        CameraInfo* camInfo = CMUCamera::getCameraInfos(i);
        if (camInfo != NULL) {
            camInfo->checkAddBayer8();
            camerasInfo.push_back(*camInfo);
        }
    }
#endif
#ifdef AVG_ENABLE_DSHOW
    int amountDSCameras = DSCamera::countCameras();
    for (int i = 0; i < amountDSCameras; i++) {
        CameraInfo* camInfo = DSCamera::getCameraInfos(i);
        if (camInfo != NULL) {
            camInfo->checkAddBayer8();
            camerasInfo.push_back(*camInfo);
        }
    }
#endif
#ifdef AVG_ENABLE_V4L2
    int amountV4LCameras = V4LCamera::countCameras();
    for (int i = 0; i < amountV4LCameras; i++) {
        CameraInfo* camInfo = V4LCamera::getCameraInfos(i);
        if (camInfo != NULL) {
            camInfo->checkAddBayer8();
            camerasInfo.push_back(*camInfo);
        }
    }
#endif
    return camerasInfo;
}


}
