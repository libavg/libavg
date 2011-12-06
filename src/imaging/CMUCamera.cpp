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
//  Original author of this file is Nick Hebner (hebnern@gmail.com).
//

#include "CMUCamera.h"
#include "CMUCameraUtils.h"

#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/OSHelper.h"
#include "../base/StringHelper.h"

#include <windows.h>
#include <1394Camera.h>

using namespace std;

namespace avg {

CMUCamera::CMUCamera(long long guid, bool bFW800, IntPoint size, 
        PixelFormat camPF, PixelFormat destPF, float frameRate)
    : Camera(camPF, destPF, size, frameRate),
      m_WhitebalanceU(-1),
      m_WhitebalanceV(-1),
      m_pCamera(0)
{
    m_pCamera = new C1394Camera();
    int err;
    unsigned long videoFormat, videoMode;
    getVideoFormatAndMode(getImgSize(), getCamPF(), &videoFormat, &videoMode);

    // Find and open camera
    if (m_pCamera->RefreshCameraList() <= 0) {
        throw Exception(AVG_ERR_CAMERA_NONFATAL, "No Firewire cameras found");
    }
    int i = getCamIndex(guid);
    err = m_pCamera->SelectCamera(i);
    err = m_pCamera->InitCamera(TRUE);
    AVG_ASSERT(err == CAM_SUCCESS);

    if (bFW800) {
        m_pCamera->Set1394b(true);
    }

    // Setup video format and rate
    err = m_pCamera->SetVideoFormat(videoFormat);
    checkCMUError(err, AVG_ERR_CAMERA_NONFATAL,
            string("CMUCamera: Error setting video format ") + toString(videoFormat) + 
            ", mode: " + toString(videoMode));
    err = m_pCamera->SetVideoMode(videoMode);
    checkCMUError(err, AVG_ERR_CAMERA_NONFATAL,
            string("CMUCamera: Error setting video mode ") + toString(videoMode) + 
            ", format: " + toString(videoFormat));
    err = m_pCamera->SetVideoFrameRate(getFrameRateConst(getFrameRate()));
    checkCMUError(err, AVG_ERR_CAMERA_NONFATAL, "Error setting frame rate");

    // Start capturing images
    err = m_pCamera->StartImageAcquisition();
    if (err != CAM_SUCCESS) {
        throw Exception(AVG_ERR_CAMERA_NONFATAL,
                "CMUCamera: Could not start image acquisition. " +
                CMUErrorToString(err));
    }

    // Set camera features
    for (FeatureMap::iterator it=m_Features.begin(); it != m_Features.end(); it++) {
        setFeature(it->first, it->second, true);
    }
    setWhitebalance(m_WhitebalanceU, m_WhitebalanceV, true);

    if (camPF == BAYER8) {
        char sModel[256], sVendor[256];
        m_pCamera->GetCameraName(sModel, 256);
        m_pCamera->GetCameraVendor(sVendor, 256);

        if (strcmp(sModel, "DFx 31BF03") == 0) {
            AVG_TRACE(Logger::CONFIG, "Applying bayer pattern fixup for IS DFx31BF03 camera");
            setCamPF(BAYER8_GRBG);
        } else if (strcmp(sVendor, "Point Grey Research") == 0) {
            AVG_TRACE(Logger::CONFIG, "Applying bayer pattern fixup for PointGrey cameras");
            enablePtGreyBayer();
        }
        
    }
}

CMUCamera::~CMUCamera()
{
    m_pCamera->StopImageAcquisition();
    delete m_pCamera;
}

BitmapPtr CMUCamera::getImage(bool bWait)
{
    if (bWait) {
        unsigned rc = WaitForSingleObject(m_pCamera->GetFrameEvent(), INFINITE);
        AVG_ASSERT(rc == WAIT_OBJECT_0);
    } else {
        unsigned rc = WaitForSingleObject(m_pCamera->GetFrameEvent(), 0);
        if (rc == WAIT_TIMEOUT) {
            // No frame yet
            return BitmapPtr();
        }
        AVG_ASSERT(rc == WAIT_OBJECT_0);
    }
    int rc2 = m_pCamera->AcquireImageEx(FALSE, NULL);
    if (rc2 != CAM_SUCCESS) {
        throw Exception(AVG_ERR_CAMERA_NONFATAL,
                "CMUCamera: Could not acquire image from camera. " +
                CMUErrorToString(rc2));
    }
    unsigned long captureBufferLength;
    unsigned char* pCaptureBuffer = m_pCamera->GetRawData(&captureBufferLength);

    BitmapPtr pCamBmp(new Bitmap(getImgSize(), getCamPF(), pCaptureBuffer, 
            captureBufferLength / getImgSize().y, false, "TempCameraBmp"));
    return convertCamFrameToDestPF(pCamBmp);
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
                int err = pControl->SetValue(Value);
                checkCMUWarning(err == CAM_SUCCESS, "Error setting camera strobe.");
            } else {
                AVG_TRACE(Logger::WARNING, "Camera does not support strobe.");
            }
        } else {
            CAMERA_FEATURE cmuFeature = getFeatureID(Feature);
            if (m_pCamera->HasFeature(cmuFeature)) {
                bool bAuto = (Value == -1);
                
                C1394CameraControl* pControl = m_pCamera->GetCameraControl(cmuFeature);
                int err1 = pControl->SetAutoMode(bAuto);
                int err2 = CAM_SUCCESS;
                if (!bAuto) {
                    err2 = pControl->SetValue(Value);
                }
                checkCMUWarning(err1 == CAM_SUCCESS && err2 == CAM_SUCCESS, 
                        string("Error setting camera feature: ") + 
                        cameraFeatureToString(Feature));
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
        int err1 = pControl->SetOnOff(false);
        int err2 = pControl->SetAutoMode(false);
        int err3 = pControl->SetOnePush(true);
        checkCMUWarning(err1 == CAM_SUCCESS && err2 == CAM_SUCCESS 
                && err3 == CAM_SUCCESS,
                string("Error setting feature: ") + cameraFeatureToString(Feature));
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
            int err1 = pControl->SetAutoMode(bAuto);
            int err2 = CAM_SUCCESS;
            if (!bAuto) {
                err2 = pControl->SetValue(u, v);
            }
            checkCMUWarning(err1 == CAM_SUCCESS && err2 == CAM_SUCCESS,
                    string("Error setting camera feature: ") + 
                    cameraFeatureToString(CAM_FEATURE_WHITE_BALANCE));
        } else {
            AVG_TRACE(Logger::WARNING, string("Camera does not support feature: ") + 
                    cameraFeatureToString(CAM_FEATURE_WHITE_BALANCE));
        }
    }
}

int CMUCamera::countCameras()
{
    C1394Camera* pCamera = new C1394Camera();
    if (pCamera->RefreshCameraList() <= 0) {
        return 0;
    }
    int numCameras = pCamera->GetNumberCameras();
    return numCameras;
}

CameraInfo* CMUCamera::getCameraInfos(int deviceNumber)
{
#ifdef AVG_ENABLE_CMU1394
    C1394Camera* pCamera = new C1394Camera();
    int err = pCamera->RefreshCameraList();
    if (err <= 0) {
        return 0;
    }

    err = pCamera->SelectCamera(deviceNumber);
    if (err != CAM_SUCCESS) {
        AVG_ASSERT(false);
    }
    pCamera->InitCamera(true);

    long long uniqueID;
    pCamera->GetCameraUniqueID((PLARGE_INTEGER)&uniqueID);
    stringstream deviceID;
    deviceID << uniqueID;

    CameraInfo* pCamInfo = new CameraInfo("Firewire", deviceID.str());
    getCameraImageFormats(pCamera, pCamInfo);
    getCameraControls(pCamera, pCamInfo);

    delete pCamera;
    return pCamInfo;
#endif
    return NULL;
}

void CMUCamera::getCameraImageFormats(C1394Camera* pCamera, CameraInfo* pCamInfo)
{
    //Iterate over formats (up to 3 formats are supported)
    for (int format = 0; format <= 2; format++) {
        BOOL hasFormat = false;
        hasFormat = pCamera->HasVideoFormat(format);
        if(!hasFormat){
            continue;
        }
        //Iterate over modes (up to 8 modes are supported)
        for (int mode = 0; mode <= 7; mode++) {
            BOOL hasMode = false;
            hasMode = pCamera->HasVideoMode(format, mode);
            if (!hasMode) {
                continue;
            }
            //Ignore not libavg supported formats
            if (mode == 0 && format == 0) {
                continue;
            }

            IntPoint size;
            PixelFormat pixelFormat;
            FrameratesVector framerates;

            getImageSizeAndPF(format, mode, size, pixelFormat);
            getCameraFramerates(pCamera, format, mode, framerates);
                    
            CameraImageFormat imageFormat = CameraImageFormat(size, pixelFormat, framerates);
            pCamInfo->addImageFormat(imageFormat);
        }
    }
}

void CMUCamera::getCameraFramerates(C1394Camera* pCamera, unsigned long videoFormat, unsigned long videoMode, FrameratesVector &framerates)
{
    for (int itFramerate = 0; itFramerate <= 7; itFramerate++) {
        BOOL hasFramerate = false;
        hasFramerate = pCamera->HasVideoFrameRate(videoFormat, videoMode, itFramerate);
        if (!hasFramerate) {
            continue;
        }

        float framerate = getFrameRateFloat(itFramerate);
        framerates.push_back(framerate);
    }
}

void CMUCamera::getCameraControls(C1394Camera* pCamera, CameraInfo* pCamInfo)
{
    //Iterate over amount of possible Features (up to 24 in CMU1394 DCD 6.4.5.240)
    for (int indexFeature = 0; indexFeature <= 23; indexFeature++) {
        C1394CameraControl* feature = pCamera->GetCameraControl((CAMERA_FEATURE)indexFeature);
        if (feature == NULL) {
            continue;
        }
        bool hasFeature = pCamera->HasFeature((CAMERA_FEATURE)indexFeature);
        if (!hasFeature) {
            continue;
        }
        //FrameRate (also known as TransferRate) is not supported
        if (feature->GetFeatureID() == FEATURE_FRAME_RATE) {
            continue;
        }

        std::string featureName = feature->GetName();
        unsigned short min = -1;
        unsigned short max = -1;
        feature->GetRange(&min, &max);
        unsigned short value_low = -1;
        unsigned short value_high = -1; //TODO: For Whitebalance or Temperature etc.
        feature->GetValue(&value_low, &value_high);
        CameraControl camControl = CameraControl(featureName, (int)min, (int)max, (int)value_low);
        pCamInfo->addControl(camControl);
    }
}

int CMUCamera::getCamIndex(long long guid)
{
    if (guid == 0) {
        return 0;
    } else {
        for (int i=0; i<m_pCamera->GetNumberCameras(); ++i) {
            m_pCamera->SelectCamera(i);
            long long camGuid;
            m_pCamera->GetCameraUniqueID((PLARGE_INTEGER)&camGuid);
            if (camGuid == guid) {
                return i;
            }
        }
        AVG_TRACE(Logger::WARNING, string("Camera with guid ") + toString(guid)
                + " not present. Using first camera.");
        return 0;
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
        pControl->Status();
        pControl->GetValue(val1, val2);
    } else {
        AVG_TRACE(Logger::WARNING, string("Error reading camera feature: ") + 
                cameraFeatureToString(Feature));
    }
}

void CMUCamera::enablePtGreyBayer()
{
    int err;
    unsigned long advOffset = m_pCamera->GetAdvancedFeatureOffset();
    
    unsigned long imageDataFormat;
    err = m_pCamera->ReadQuadlet(advOffset+0x48, &imageDataFormat);
    AVG_ASSERT(err == CAM_SUCCESS);
    if (imageDataFormat & 0x80000000) {
        err = m_pCamera->WriteQuadlet(advOffset+0x48, 0x80000081);
        AVG_ASSERT(err == CAM_SUCCESS);
        unsigned long bayerFormat;
        err = m_pCamera->ReadQuadlet(advOffset+0x40, &bayerFormat);
        AVG_ASSERT(err == CAM_SUCCESS);
        PixelFormat exactPF = fwBayerStringToPF(bayerFormat);
        setCamPF(exactPF);
    } else {
        AVG_TRACE(Logger::ERROR, "imageDataFormat not supported.");
    }
}

void CMUCamera::checkCMUError(int code, int type, const string & sMsg) const
{
    if (code != CAM_SUCCESS) {
        throw Exception(type, sMsg);
    }
}

void CMUCamera::checkCMUWarning(bool bOk, const string& sMsg) const
{
    if (!bOk) {
        AVG_TRACE(Logger::WARNING, sMsg);
    }
}

string CMUCamera::CMUErrorToString(int code)
{
    if (code == CAM_ERROR) {
        return "WinI/O returned: " + getWinErrMsg(GetLastError());
    }
    string msg = "1394Camera returned: ";
    switch (code) {
        case CAM_ERROR_UNSUPPORTED:
            return msg + "CAM_ERROR_UNSUPPORTED.";
        case CAM_ERROR_NOT_INITIALIZED:
            return msg + "CAM_ERROR_NOT_INITIALIZED.";
        case CAM_ERROR_INVALID_VIDEO_SETTINGS:
            return msg + "CAM_ERROR_INVALID_VIDEO_SETTINGS.";
        case CAM_ERROR_BUSY:
            return msg + "CAM_ERROR_BUSY.";
        case CAM_ERROR_INSUFFICIENT_RESOURCES:
            return msg + "CAM_ERROR_INSUFFICIENT_RESOURCES.";
        case CAM_ERROR_PARAM_OUT_OF_RANGE:
            return msg + "CAM_ERROR_PARAM_OUT_OF_RANGE.";
        case CAM_ERROR_FRAME_TIMEOUT:
            return msg + "CAM_ERROR_FRAME_TIMEOUT.";
        default:
            return msg + "unknown error.";
    }
}

}
