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

#include "CMUCamera.h"
#include "CMUCameraUtils.h"

#include "../base/Logger.h"
#include "../base/StringHelper.h"
#include "../graphics/Filterfliprgb.h"


using namespace std;

namespace avg {

CMUCamera::CMUCamera(std::string sDevice, IntPoint Size, std::string sPF,
            double FrameRate, bool bColor)
    : m_sDevice(sDevice),
      m_sPF(sPF),
      m_Size(Size),
      m_FrameRate(FrameRate),
      m_bCameraAvailable(false),
      m_bFlipRGB(false)
{
    if (m_sPF == "MONO8") {
        m_FramePixelFormat = I8;
    } else if (m_sPF == "MONO16") {
        m_FramePixelFormat = I16;
    } else if (m_sPF == "YUV411") {
        m_FramePixelFormat = YCbCr411;
    } else if (m_sPF == "YUV422") {
        m_FramePixelFormat = YCbCr422;
    } else if (m_sPF == "RGB") {
        m_FramePixelFormat = R8G8B8;
        m_bFlipRGB = true;
    } else if (m_sPF == "BGR") {
        m_FramePixelFormat = B8G8R8;
        m_bFlipRGB = false;
    } else if (m_sPF == "BY8_GBRG") {
        m_FramePixelFormat = BAYER8_GBRG;
    } else {
        fatalError(string("Invalid pixel format '") + m_sPF + "'");
    }

    if (bColor || m_sPF == "BY8_GBRG") {
        if (m_bFlipRGB)
            m_OutputPixelFormat = R8G8B8X8;
        else
            m_OutputPixelFormat = B8G8R8X8;
    } else {
        m_OutputPixelFormat = I8;
    }
}

CMUCamera::~CMUCamera()
{
    close();
}

void CMUCamera::open()
{
    unsigned long videoFormat, videoMode;
    getVideoFormatAndMode(m_Size, m_sPF, &videoFormat, &videoMode);

    // Find and open camera
    if (m_Camera.RefreshCameraList() <= 0) {
        static bool bFirstWarning = true;
        if (bFirstWarning) {
            AVG_TRACE(Logger::WARNING, "No firewire cameras found.");
            bFirstWarning = false;
        }
        m_bCameraAvailable = false;
        return;
    }
    if (m_Camera.SelectCamera(atoi(m_sDevice.c_str())) != CAM_SUCCESS) {
        fatalError(string("Error selecting camera") + m_sDevice);
    }
    if (m_Camera.InitCamera(TRUE) != CAM_SUCCESS) {
        fatalError("Error initializing camera");
    }

    // Setup video format and rate
    if (m_Camera.SetVideoFormat(videoFormat) != CAM_SUCCESS) {
        fatalError(string("CMUCamera: Error setting video format ") 
                + toString(videoFormat) + ", mode: " + toString(videoMode));
    }
    if (m_Camera.SetVideoMode(videoMode) != CAM_SUCCESS) {
        fatalError(string("CMUCamera: Error setting video mode ") 
                + toString(videoMode) + ", format: " + toString(videoFormat));
    }
    if (m_Camera.SetVideoFrameRate(getFrameRateConst(m_FrameRate)) != CAM_SUCCESS) {
        fatalError("Error setting frame rate");
    }

    // Start capturing images
    if (m_Camera.StartImageAcquisition() != CAM_SUCCESS) {
        fatalError("Error starting image acquisition");
    }

    m_bCameraAvailable = true;
    
    AVG_TRACE(Logger::CONFIG, "Firewire camera opened.");

    // Set camera features
    for (FeatureMap::iterator it=m_Features.begin(); it != m_Features.end(); it++) {
        setFeature(it->first, it->second, true);
    }
}

void CMUCamera::close()
{
    if (m_bCameraAvailable) {
        m_Camera.StopImageAcquisition();
        m_bCameraAvailable = false;
    }
}

IntPoint CMUCamera::getImgSize()
{
    return m_Size;
}

BitmapPtr CMUCamera::getImage(bool bWait)
{
    if ((bWait || WaitForSingleObject(m_Camera.GetFrameEvent(), 0) == WAIT_OBJECT_0) &&
            (m_Camera.AcquireImageEx(TRUE, NULL) == CAM_SUCCESS)) {
        unsigned long captureBufferLength;
        unsigned char* pCaptureBuffer = m_Camera.GetRawData(&captureBufferLength);

        Bitmap frame(m_Size, m_FramePixelFormat, pCaptureBuffer, 
                captureBufferLength / m_Size.y, false, "TempCameraBmp");
        
        BitmapPtr pFrameBuffer = BitmapPtr(new Bitmap(m_Size, m_OutputPixelFormat));
        pFrameBuffer->copyPixels(frame);
        if (m_bFlipRGB) {
            FilterFlipRGB().applyInPlace(pFrameBuffer);
        }

        return pFrameBuffer;
    } else {
        return BitmapPtr();
    }
}
    
bool CMUCamera::isCameraAvailable()
{
    return m_bCameraAvailable;
}

const string& CMUCamera::getDevice() const
{
    return m_sDevice;
}

const std::string& CMUCamera::getDriverName() const
{
    static string sDriverName = "CMU 1394 Digital Camera Driver";
    return sDriverName;
}

double CMUCamera::getFrameRate() const
{
    return m_FrameRate;
}

unsigned int CMUCamera::getFeature(CameraFeature Feature) const
{
    FeatureMap::const_iterator it = m_Features.find(Feature);
    if (it == m_Features.end()) {
        return 0;
    } else {
        return it->second;
    }
}

void CMUCamera::setFeature(CameraFeature Feature, int Value, bool bIgnoreOldValue)
{
    if (bIgnoreOldValue || m_Features[Feature] != Value) {
        m_Features[Feature] = Value;
        if (m_bCameraAvailable) {
            if (Feature == CAM_FEATURE_STROBE_DURATION) {
                if (m_Camera.HasStrobe()) {
                    C1394CameraControlStrobe* pControl = m_Camera.GetStrobeControl(0);
                    if (pControl->SetValue(Value) != CAM_SUCCESS) {
                        AVG_TRACE(Logger::WARNING, "Error setting strobe");
                    }
                } else {
                    AVG_TRACE(Logger::WARNING, "Camera does not support strobe");
                }
            } else {
                CAMERA_FEATURE cmuFeature = getFeatureID(Feature);
                if (cmuFeature != FEATURE_INVALID_FEATURE && m_Camera.HasFeature(cmuFeature)) {
                    bool bAuto = (Value == -1);
                    
                    C1394CameraControl* pControl = m_Camera.GetCameraControl(cmuFeature);

                    if ((pControl->SetAutoMode(bAuto) != CAM_SUCCESS) ||
                            (!bAuto && pControl->SetValue(Value) != CAM_SUCCESS)) {
                        AVG_TRACE(Logger::WARNING, string("Error setting feature: ") + 
                                cameraFeatureToString(Feature));
                    }
                } else {
                    AVG_TRACE(Logger::WARNING, string("Camera does not support feature: ") + 
                            cameraFeatureToString(Feature));
                }
            }
        }
    }
}

void CMUCamera::setFeatureOneShot(CameraFeature Feature)
{
    if (m_bCameraAvailable) {
        CAMERA_FEATURE cmuFeature = getFeatureID(Feature);
        if (cmuFeature != FEATURE_INVALID_FEATURE && m_Camera.HasFeature(cmuFeature)) {
            C1394CameraControl* pControl = m_Camera.GetCameraControl(cmuFeature);
            if (pControl->SetAutoMode(false) != CAM_SUCCESS) {
                AVG_TRACE(Logger::WARNING, string("Error setting feature: ") + 
                        cameraFeatureToString(Feature));
            }
            if (pControl->SetOnePush(true) != CAM_SUCCESS) {
                AVG_TRACE(Logger::WARNING, string("Error setting feature: ") + 
                        cameraFeatureToString(Feature));
            }
        } else {
            AVG_TRACE(Logger::WARNING, string("Camera does not support feature: ") + 
                    cameraFeatureToString(Feature));
        }
    }
}

void CMUCamera::fatalError(const string & sMsg)
{
    AVG_TRACE(Logger::ERROR, sMsg);
    close();
    exit(1);
}

}
