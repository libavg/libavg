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

#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/StringHelper.h"
#include "../base/TimeSource.h"
#include "../graphics/Filterfliprgb.h"

#include <windows.h>
#include <1394Camera.h>

using namespace std;

namespace avg {

CMUCamera::CMUCamera(std::string sDevice, IntPoint Size, std::string sPF,
            double FrameRate, bool bColor)
    : m_sDevice(sDevice),
      m_sPF(sPF),
      m_Size(Size),
      m_FrameRate(FrameRate),
      m_bFlipRGB(false),
      m_WhitebalanceU(-1),
      m_WhitebalanceV(-1),
      m_pCamera(0)
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
    m_pCamera = new C1394Camera();
    unsigned long videoFormat, videoMode;
    getVideoFormatAndMode(m_Size, m_sPF, &videoFormat, &videoMode);

    // Find and open camera
    if (m_pCamera->RefreshCameraList() <= 0) {
        throw Exception(AVG_ERR_CAMERA, "No Firewire cameras found");
    }
    if (m_pCamera->SelectCamera(atoi(m_sDevice.c_str())) != CAM_SUCCESS) {
        fatalError(string("Error selecting camera") + m_sDevice);
    }
    if (m_pCamera->InitCamera(TRUE) != CAM_SUCCESS) {
        fatalError("Error initializing camera");
    }

    // Setup video format and rate
    if (m_pCamera->SetVideoFormat(videoFormat) != CAM_SUCCESS) {
        fatalError(string("CMUCamera: Error setting video format ") 
                + toString(videoFormat) + ", mode: " + toString(videoMode));
    }
    if (m_pCamera->SetVideoMode(videoMode) != CAM_SUCCESS) {
        fatalError(string("CMUCamera: Error setting video mode ") 
                + toString(videoMode) + ", format: " + toString(videoFormat));
    }
    if (m_pCamera->SetVideoFrameRate(getFrameRateConst(m_FrameRate)) != CAM_SUCCESS) {
        fatalError("Error setting frame rate");
    }

    // Start capturing images
    if (m_pCamera->StartImageAcquisition() != CAM_SUCCESS) {
        fatalError("Error starting image acquisition");
    }

    
    AVG_TRACE(Logger::CONFIG, "Firewire camera opened.");

    // Set camera features
    for (FeatureMap::iterator it=m_Features.begin(); it != m_Features.end(); it++) {
        setFeature(it->first, it->second, true);
    }
    setWhitebalance(m_WhitebalanceU, m_WhitebalanceV, true);
}

CMUCamera::~CMUCamera()
{
    m_pCamera->StopImageAcquisition();
    delete m_pCamera;
}


IntPoint CMUCamera::getImgSize()
{
    return m_Size;
}

BitmapPtr CMUCamera::getImage(bool bWait)
{
    if (bWait) {
        // XXX: Untested!
        unsigned rc = WaitForSingleObject(m_pCamera->GetFrameEvent(), INFINITE);
        assert(rc == WAIT_OBJECT_0);
    } else {
        unsigned rc = WaitForSingleObject(m_pCamera->GetFrameEvent(), 0);
        if (rc == WAIT_TIMEOUT) {
            // No frame yet
            return BitmapPtr();
        }
        assert(rc == WAIT_OBJECT_0);
    }
    int rc2 = m_pCamera->AcquireImageEx(FALSE, NULL);
    assert(rc2 == CAM_SUCCESS);
    unsigned long captureBufferLength;
    unsigned char* pCaptureBuffer = m_pCamera->GetRawData(&captureBufferLength);

    Bitmap frame(m_Size, m_FramePixelFormat, pCaptureBuffer, 
            captureBufferLength / m_Size.y, false, "TempCameraBmp");
    
    BitmapPtr pFrameBuffer = BitmapPtr(new Bitmap(m_Size, m_OutputPixelFormat));
    pFrameBuffer->copyPixels(frame);
    if (m_bFlipRGB) {
        FilterFlipRGB().applyInPlace(pFrameBuffer);
    }

    return pFrameBuffer;
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

int CMUCamera::getFeature(CameraFeature Feature) const
{
    unsigned short val1;
    unsigned short val2;
    internalGetFeature(Feature, &val1, &val2);
    return val1;
}

void CMUCamera::setFeature(CameraFeature Feature, int Value, bool bIgnoreOldValue)
{
    if (bIgnoreOldValue || m_Features[Feature] != Value) {
        m_Features[Feature] = Value;
        if (Feature == CAM_FEATURE_STROBE_DURATION) {
            if (m_pCamera->HasStrobe()) {
                C1394CameraControlStrobe* pControl = m_pCamera->GetStrobeControl(0);
                if (pControl->SetValue(Value) != CAM_SUCCESS) {
                    AVG_TRACE(Logger::WARNING, "Error setting camera strobe.");
                }
            } else {
                AVG_TRACE(Logger::WARNING, "Camera does not support strobe.");
            }
        } else {
            CAMERA_FEATURE cmuFeature = getFeatureID(Feature);
            if (m_pCamera->HasFeature(cmuFeature)) {
                bool bAuto = (Value == -1);
                
                C1394CameraControl* pControl = m_pCamera->GetCameraControl(cmuFeature);

                if ((pControl->SetAutoMode(bAuto) != CAM_SUCCESS) ||
                        (!bAuto && pControl->SetValue(Value) != CAM_SUCCESS)) {
                    AVG_TRACE(Logger::WARNING, string("Error setting camera feature: ") + 
                            cameraFeatureToString(Feature));
                }
            } else {
                AVG_TRACE(Logger::WARNING, string("Camera does not support feature: ") + 
                        cameraFeatureToString(Feature));
            }
        }
    }
}

void CMUCamera::setFeatureOneShot(CameraFeature Feature)
{
    CAMERA_FEATURE cmuFeature = getFeatureID(Feature);
    if (cmuFeature != FEATURE_INVALID_FEATURE && m_pCamera->HasFeature(cmuFeature)) {
        C1394CameraControl* pControl = m_pCamera->GetCameraControl(cmuFeature);
        if (pControl->SetOnOff(false) != CAM_SUCCESS) {
            AVG_TRACE(Logger::WARNING, string("Error setting feature: ") + 
                    cameraFeatureToString(Feature));
        }
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

int CMUCamera::getWhitebalanceU() const
{
    unsigned short val1;
    unsigned short val2;
    internalGetFeature(CAM_FEATURE_WHITE_BALANCE, &val1, &val2);
    return val1;
}

int CMUCamera::getWhitebalanceV() const
{
    unsigned short val1;
    unsigned short val2;
    internalGetFeature(CAM_FEATURE_WHITE_BALANCE, &val1, &val2);
    return val2;
}

void CMUCamera::setWhitebalance(int u, int v, bool bIgnoreOldValue)
{
    if (bIgnoreOldValue || m_WhitebalanceU != u || m_WhitebalanceV != v) {
        m_WhitebalanceU = u;
        m_WhitebalanceV = v;
        CAMERA_FEATURE cmuFeature = getFeatureID(CAM_FEATURE_WHITE_BALANCE);
        if (m_pCamera->HasFeature(FEATURE_WHITE_BALANCE)) {
            bool bAuto = (u == -1);
            
            C1394CameraControl* pControl = m_pCamera->GetCameraControl(cmuFeature);

            if ((pControl->SetAutoMode(bAuto) != CAM_SUCCESS) ||
                    (!bAuto && pControl->SetValue(u, v) != CAM_SUCCESS)) {
                AVG_TRACE(Logger::WARNING, string("Error setting camera feature: ") + 
                        cameraFeatureToString(CAM_FEATURE_WHITE_BALANCE));
            }
        } else {
            AVG_TRACE(Logger::WARNING, string("Camera does not support feature: ") + 
                    cameraFeatureToString(CAM_FEATURE_WHITE_BALANCE));
        }
    }
}

void CMUCamera::internalGetFeature(CameraFeature Feature, unsigned short* val1, 
        unsigned short* val2) const
{
    *val1 = -1;
    *val2 = -1;
    CAMERA_FEATURE cmuFeature = getFeatureID(Feature);
    if (m_pCamera->HasFeature(cmuFeature)) {
        C1394CameraControl* pControl = m_pCamera->GetCameraControl(cmuFeature);
        pControl->GetValue(val1, val2);
    } else {
        AVG_TRACE(Logger::WARNING, string("Error reading camera feature: ") + 
                cameraFeatureToString(Feature));
    }
}

void CMUCamera::fatalError(const string & sMsg)
{
    throw Exception(AVG_ERR_CAMERA, sMsg);
}

}
