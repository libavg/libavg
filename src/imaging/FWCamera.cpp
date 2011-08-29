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

#include "FWCamera.h"
#ifdef AVG_ENABLE_1394_2
#include "FWCameraUtils.h"
#endif

#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/ScopeTimer.h"
#include "../base/StringHelper.h"

#include <sstream>
#include <iomanip>

namespace avg {

using namespace std;

FWCamera::FWCamera(uint64_t guid, int unit, bool bFW800, IntPoint size, 
        PixelFormat camPF, PixelFormat destPF, double frameRate)
    : Camera(camPF, destPF),
      m_Size(size),
      m_FrameRate(frameRate),
      m_WhitebalanceU(-1),
      m_WhitebalanceV(-1)
{
#ifdef AVG_ENABLE_1394_2
    m_FrameRateConstant = getFrameRateConst(m_FrameRate);
    if (camPF == I16) {
        throw Exception(AVG_ERR_CAMERA_NONFATAL, 
                "I16 pixel format is not supported for firewire cameras.");
    }
    m_Mode = getCamMode(size, camPF);
    dc1394camera_list_t * pCameraList;

    m_pDC1394 = dc1394_new();
    if (m_pDC1394 == 0) {
        throw Exception(AVG_ERR_CAMERA_NONFATAL, 
                "Failed to initialize firewire subsystem");
    }
    int err = dc1394_camera_enumerate(m_pDC1394, &pCameraList);

    if (err != DC1394_SUCCESS) {
        AVG_TRACE(Logger::ERROR, "Unable to look for cameras");
#ifdef linux
        AVG_TRACE(Logger::ERROR, "Please check");
        AVG_TRACE(Logger::ERROR,
              "  - if the kernel modules `ieee1394',`raw1394' and `ohci1394' are loaded");
        AVG_TRACE(Logger::ERROR,
              "  - if you have read/write access to /dev/raw1394.");
#endif
        dc1394_free(m_pDC1394);
        throw Exception(AVG_ERR_CAMERA_NONFATAL,"Firewire failure");
    }
    
    if (pCameraList->num == 0) {
        dc1394_camera_free_list(pCameraList);
        dc1394_free(m_pDC1394);
        throw Exception(AVG_ERR_CAMERA_NONFATAL,"No firewire cameras found.");
    }
    int id_to_use = -1;
    if (guid != 0) {
        for (unsigned i = 0; i < pCameraList->num; ++i) {
            if (pCameraList->ids[i].guid == guid) {
                id_to_use = i;
            }       
        }
        if (id_to_use == -1) {
            AVG_TRACE(Logger::WARNING, "Firewire GUID=" << hex << guid << dec
                    << " requested but not found on bus. Using first camera");
            id_to_use = 0;
        }
    } else {
        id_to_use = 0;
    }
    if (unit != -1) {
        m_pCamera = dc1394_camera_new_unit(m_pDC1394, pCameraList->ids[id_to_use].guid, 
                unit);
    } else {
        m_pCamera = dc1394_camera_new(m_pDC1394, pCameraList->ids[id_to_use].guid);
    }
    if (!m_pCamera) {
        dc1394_camera_free_list(pCameraList);
        dc1394_free(m_pDC1394);
        throw Exception(AVG_ERR_CAMERA_FATAL,"Failed to initialize camera");
    }

    dc1394_camera_free_list(pCameraList);

    if (bFW800) {
        dc1394_video_set_operation_mode(m_pCamera, DC1394_OPERATION_MODE_1394B);
        err = dc1394_video_set_iso_speed(m_pCamera, DC1394_ISO_SPEED_800);
    } else {
        err = dc1394_video_set_iso_speed(m_pCamera, DC1394_ISO_SPEED_400);
    }
    AVG_ASSERT(err == DC1394_SUCCESS);
    err = dc1394_video_set_mode(m_pCamera, m_Mode);
    AVG_ASSERT(err == DC1394_SUCCESS);

    dc1394framerates_t frameRates;
    err = dc1394_video_get_supported_framerates(m_pCamera, m_Mode, &frameRates);
    AVG_ASSERT(err == DC1394_SUCCESS);
    bool bFrameRateSupported = false;
    for (unsigned int i = 0; i < frameRates.num; i++) {
        if (frameRates.framerates[i] == m_FrameRateConstant) {
            bFrameRateSupported = true;
            break;
        }
    }
    if (!bFrameRateSupported) {
        AVG_TRACE(Logger::ERROR, "Camera does not support framerate " << m_FrameRate 
                << " in the current video mode.");
        dc1394_capture_stop(m_pCamera);
        dc1394_video_set_transmission(m_pCamera, DC1394_OFF);
        dc1394_camera_free(m_pCamera);
        dc1394_free(m_pDC1394);
        throw Exception(AVG_ERR_CAMERA_NONFATAL, 
                string("Camera does not support framerate ")+toString(m_FrameRate)+
                " in the current video mode.");
    }

    err = dc1394_video_set_framerate(m_pCamera, m_FrameRateConstant);
    AVG_ASSERT(err == DC1394_SUCCESS);

    err = dc1394_capture_setup(m_pCamera,8, DC1394_CAPTURE_FLAGS_DEFAULT);
    if (err != DC1394_SUCCESS) {
        AVG_TRACE(Logger::ERROR, "Unable to setup camera. Make sure that");
        AVG_TRACE(Logger::ERROR, "video mode and framerate (" <<
                m_FrameRate << ") are");
        AVG_TRACE(Logger::ERROR, "supported by your camera.");
        dc1394_capture_stop(m_pCamera);
        dc1394_video_set_transmission(m_pCamera, DC1394_OFF);
        dc1394_camera_free(m_pCamera);
        dc1394_free(m_pDC1394);
        throw Exception(AVG_ERR_CAMERA_NONFATAL, "Failed to initialize camera");
    }
#else
    AVG_ASSERT(false);
#endif
}

FWCamera::~FWCamera()
{
#ifdef AVG_ENABLE_1394_2
    dc1394_video_set_transmission(m_pCamera, DC1394_OFF);
    dc1394_capture_stop(m_pCamera);
    dc1394_camera_free(m_pCamera);
    dc1394_free(m_pDC1394);
#endif
    AVG_TRACE(Logger::CONFIG, "Firewire camera closed.");
}

void FWCamera::startCapture()
{
#ifdef AVG_ENABLE_1394_2
    int err = dc1394_video_set_transmission(m_pCamera, DC1394_ON);
    AVG_ASSERT(err == DC1394_SUCCESS);

    dc1394switch_t status = DC1394_OFF;

    int i = 0;
    while (status == DC1394_OFF && i++ < 5) {
        usleep(50000);
        err = dc1394_video_get_transmission(m_pCamera, &status);
        AVG_ASSERT(err == DC1394_SUCCESS);
    }

    if (i == 5) {
        AVG_ASSERT(false);
    }
    // Default to turning off any camera sharpness manipulation.
    setFeature(CAM_FEATURE_SHARPNESS, 0);

    // Turn off possible auto exposure.
    dc1394_feature_set_mode(m_pCamera, DC1394_FEATURE_EXPOSURE, 
            DC1394_FEATURE_MODE_MANUAL);
    dc1394_feature_set_power(m_pCamera, DC1394_FEATURE_EXPOSURE, DC1394_OFF);

    AVG_TRACE(Logger::CONFIG, "Firewire camera opened.");
    for (FeatureMap::iterator it=m_Features.begin(); it != m_Features.end(); it++) {
        setFeature(it->first, it->second, true);
    }
    setWhitebalance(m_WhitebalanceU, m_WhitebalanceV, true);
    
    if (getCamPF() == BAYER8) {
        if (strcmp(m_pCamera->model, "DFx 31BF03") == 0) {
            AVG_TRACE(Logger::CONFIG,
                    "Applying bayer pattern fixup for IS DFx31BF03 camera");
            setCamPF(BAYER8_GRBG);
        } else if (strcmp(m_pCamera->vendor, "Point Grey Research") == 0) {
            AVG_TRACE(Logger::CONFIG,
                    "Applying bayer pattern fixup for PointGrey cameras");
            enablePtGreyBayer();
        }
    }
#endif
}

IntPoint FWCamera::getImgSize()
{
#ifdef AVG_ENABLE_1394_2
    return m_Size;
#else
    return IntPoint(0, 0);
#endif
}


static ProfilingZoneID CameraConvertProfilingZone("FW Camera format conversion");

BitmapPtr FWCamera::getImage(bool bWait)
{
#ifdef AVG_ENABLE_1394_2
    bool bGotFrame = false;
    unsigned char * pCaptureBuffer = 0;
    dc1394video_frame_t * pFrame;
    dc1394error_t err;
    if (bWait) {
        err = dc1394_capture_dequeue(m_pCamera, DC1394_CAPTURE_POLICY_WAIT, &pFrame);
    } else {
        err = dc1394_capture_dequeue(m_pCamera, DC1394_CAPTURE_POLICY_POLL, &pFrame);
    }
    if (err == DC1394_SUCCESS && pFrame) {
        bGotFrame = true;
        pCaptureBuffer = pFrame->image;
    }
    if (bGotFrame) {
        int lineLen;
        if (getCamPF() == YCbCr411) {
            lineLen = m_Size.x*1.5;
        } else {
            lineLen = m_Size.x*Bitmap::getBytesPerPixel(getCamPF());
        }
        BitmapPtr pCamBmp(new Bitmap(m_Size, getCamPF(), pCaptureBuffer, lineLen, false,
                "TempCameraBmp"));
        BitmapPtr pDestBmp = convertCamFrameToDestPF(pCamBmp);
//        cerr << "CamBmp: " << pCamBmp->getPixelFormat() << ", DestBmp: " 
//                << pDestBmp->getPixelFormat() << endl;
        dc1394_capture_enqueue(m_pCamera, pFrame);
        return pDestBmp;
    } else {
        return BitmapPtr();
    }
#else
    return BitmapPtr();
#endif
}
    

const string& FWCamera::getDevice() const
{
    static string deviceInfo;
    stringstream ss;
#ifdef AVG_ENABLE_1394_2
    ss << m_pCamera->vendor << " " << m_pCamera->model << " (guid=" << m_pCamera->guid 
            << ", unit=" << m_pCamera->unit << ")";
#endif
    deviceInfo = ss.str();
    return deviceInfo;
}

const std::string& FWCamera::getDriverName() const
{
#ifdef  AVG_ENABLE_1394_2
    static string sDriverName = "libdc1394 v2";
#else
    static string sDriverName = "";
#endif
    return sDriverName;
}

double FWCamera::getFrameRate() const
{
    return m_FrameRate;
}

int FWCamera::getFeature(CameraFeature feature) const
{
#ifdef AVG_ENABLE_1394_2
    FeatureMap::const_iterator it = m_Features.find(feature);
    if (it == m_Features.end()) {
        return 0;
    } else {
        return it->second;
    }
#else
    return 0;
#endif
}

bool FWCamera::hasFeature(CameraFeature feature)
{
#ifdef AVG_ENABLE_1394_2
    if (feature == CAM_FEATURE_STROBE_DURATION) {
        // FIXME
        return true; 
    } else {
        dc1394feature_t featureID = getFeatureID(feature);
        dc1394bool_t bAvailable;
        dc1394_feature_is_present(m_pCamera, featureID, &bAvailable);
        return bAvailable;
    }
#else
    return false;
#endif
}

void FWCamera::setFeature(CameraFeature feature, int value, bool bIgnoreOldValue)
{
#ifdef AVG_ENABLE_1394_2
    if (hasFeature(feature)) { 
        if (bIgnoreOldValue || m_Features[feature] != value) {
            m_Features[feature] = value;
            if (feature == CAM_FEATURE_STROBE_DURATION) {
                try {
                    setStrobeDuration(value);
                } catch (Exception& e) {
                    AVG_TRACE(Logger::WARNING, 
                            string("Camera: Setting strobe duration failed. ") + 
                            e.getStr());
                }
            } else {
                dc1394feature_t featureID = getFeatureID(feature);
                setFeature(featureID, value);
                //        dumpCameraInfo();
            }
        }
    }
#endif
}

void FWCamera::setFeatureOneShot(CameraFeature feature)
{
#ifdef AVG_ENABLE_1394_2
    dc1394feature_t featureID = getFeatureID(feature);
    dc1394error_t err = dc1394_feature_set_mode(m_pCamera, featureID, 
            DC1394_FEATURE_MODE_ONE_PUSH_AUTO);
    if (err != DC1394_SUCCESS) {
        AVG_TRACE(Logger::WARNING, "Camera: Unable to set one-shot for " 
                << cameraFeatureToString(feature) << ". Error was " << err);
    }
#endif
}

int FWCamera::getWhitebalanceU() const
{
    int u;
    int v;
    getWhitebalance(&u, &v);
    return u;
}

int FWCamera::getWhitebalanceV() const
{
    int u;
    int v;
    getWhitebalance(&u, &v);
    return v;
}

void FWCamera::setWhitebalance(int u, int v, bool bIgnoreOldValue)
{
#ifdef AVG_ENABLE_1394_2
    if (hasFeature(CAM_FEATURE_WHITE_BALANCE)) {
        if (bIgnoreOldValue || u != m_WhitebalanceU || v != m_WhitebalanceV) {
            m_WhitebalanceU = u;
            m_WhitebalanceV = v;
            dc1394error_t err;
            if (u == -1) {
                err = dc1394_feature_set_mode(m_pCamera, DC1394_FEATURE_WHITE_BALANCE,
                        DC1394_FEATURE_MODE_AUTO);
            } else {
                err = dc1394_feature_set_mode(m_pCamera, DC1394_FEATURE_WHITE_BALANCE, 
                        DC1394_FEATURE_MODE_MANUAL);
                err = dc1394_feature_whitebalance_set_value(m_pCamera, u, v);
            }
            if (err != DC1394_SUCCESS) {
                AVG_TRACE(Logger::WARNING,
                        "Camera: Unable to set whitebalance. Error was " << err);
            }
        }
    }
#endif
}

void FWCamera::setFeature(dc1394feature_t feature, int value)
{
#ifdef AVG_ENABLE_1394_2
    dc1394error_t err;
    if (value == -1) {
        err = dc1394_feature_set_mode(m_pCamera, feature, DC1394_FEATURE_MODE_AUTO);
        err = dc1394_feature_set_power(m_pCamera, feature, DC1394_OFF);
    } else {
        dc1394_feature_set_mode(m_pCamera, feature, DC1394_FEATURE_MODE_MANUAL);
        err = dc1394_feature_set_power(m_pCamera, feature, DC1394_ON);
        err = dc1394_feature_set_value(m_pCamera, feature, value);
    }
    if (err != DC1394_SUCCESS) {
        AVG_TRACE(Logger::WARNING, "Camera: Unable to set " << feature << 
                ". Error was " << err);
    }
/*
    dc1394feature_info_t featureInfo;
    featureInfo.id = feature;
    err = dc1394_feature_get(m_pCamera, &featureInfo);
    dc1394_feature_print(&featureInfo, stdout);
*/
#endif
}

void FWCamera::setStrobeDuration(int microsecs)
{
#ifdef AVG_ENABLE_1394_2
    dc1394error_t err;
    uint32_t durationRegValue;
    if (microsecs >= 63930 || microsecs < -1) {
        throw Exception(AVG_ERR_CAMERA_FATAL, string("Illegal value ")
                +toString(microsecs)+" for strobe duration.");
    }
    if (microsecs == -1) {
        // Turn off strobe. No error checking done here (if the camera doesn't support
        // strobe, setting the register will fail. But there is really no error, because
        // we're turning the feature off anyway.)
        uint32_t strobeRegValue = 0x81000000;
        err = dc1394_set_strobe_register(m_pCamera, 0x200, strobeRegValue);
    } else {
        if (microsecs < 0x400) {
            durationRegValue = microsecs;
        } else {
            // Wierd calculations: IIDC register values for time are non-linear. Translate
            // the method parameter in microseconds to appropriate register values.
            double targetMillisecs = microsecs/1000.;
            const double realTimes[] = {1,2,4,6,8,12,16,24,32,48,63.93};
            const uint32_t regValues[] = 
                {0x400, 0x600, 0x800, 0x900, 0xA00, 0xB00, 0xC00, 0xD00, 
                 0xE00, 0xF00, 0xFFF};
            int len = sizeof(regValues)/sizeof(*regValues);
            AVG_ASSERT(len == sizeof(realTimes)/sizeof(*realTimes));
            int i;
            for (i = 1; realTimes[i] < targetMillisecs; ++i); 
            double ratio = (targetMillisecs-realTimes[i])/(realTimes[i-1]-realTimes[i]);
            durationRegValue = ratio*regValues[i-1]+(1-ratio)*regValues[i];
        } 

        err = dc1394_set_PIO_register(m_pCamera, 0x08, 0xC0000000);
        AVG_ASSERT(err == DC1394_SUCCESS);
        
        uint32_t strobeRegValue = 0x83001000+durationRegValue;
        err = dc1394_set_strobe_register(m_pCamera, 0x200, strobeRegValue);
        AVG_ASSERT(err == DC1394_SUCCESS);
    }
#endif
}

void FWCamera::getWhitebalance(int* pU, int* pV) const
{
#ifdef AVG_ENABLE_1394_2
    dc1394error_t err = dc1394_feature_whitebalance_get_value(m_pCamera, 
            (uint32_t*)pU, (uint32_t*)pV);
    if (err != DC1394_SUCCESS) {
        AVG_TRACE(Logger::WARNING,
                "Camera: Unable to get whitebalance setting. Error was " << err);
    }
#endif
}

void FWCamera::enablePtGreyBayer()
{
#ifdef AVG_ENABLE_1394_2
    dc1394error_t err; 
    uint32_t imageDataFormat;
    err = dc1394_get_adv_control_register(m_pCamera, 0x48, &imageDataFormat);
    AVG_ASSERT(err == DC1394_SUCCESS);
    if (imageDataFormat & 0x80000000) {
        err = dc1394_set_adv_control_register(m_pCamera, 0x48, 0x80000081);
        AVG_ASSERT(err == DC1394_SUCCESS);
        uint32_t bayerFormat;
        err = dc1394_get_adv_control_register(m_pCamera, 0x40, &bayerFormat);
        AVG_ASSERT(err == DC1394_SUCCESS);
        PixelFormat exactPF = fwBayerStringToPF(bayerFormat);
        if (exactPF == I8) {
            throw(Exception(AVG_ERR_CAMERA_NONFATAL, 
                    "Greyscale camera doesn't support bayer pattern."));
        }
        setCamPF(exactPF);
    }
#endif
}

void FWCamera::dumpCameras()
{
#ifdef AVG_ENABLE_1394_2
    dc1394_t* pDC1394 = dc1394_new();
    if (pDC1394 == 0) {
        return;
    }
    dc1394camera_list_t * pCameraList;
    int err=dc1394_camera_enumerate(pDC1394, &pCameraList);
    if (err == DC1394_SUCCESS) {
        if (pCameraList->num != 0) {
            cerr << "Firewire cameras: " << endl;
            for (unsigned i=0; i<pCameraList->num;++i) {
                dc1394camera_id_t id = pCameraList->ids[i];
                dc1394camera_t * pCamera = dc1394_camera_new_unit(pDC1394, id.guid, 
                        id.unit);
                if (pCamera) {
                    dc1394_camera_print_info(pCamera, stderr);
                    dumpCameraInfo(pCamera);
                    dc1394_camera_free(pCamera);
                    cerr << endl;
                }
            }
        }
        dc1394_camera_free_list(pCameraList);
    }
    dc1394_free(pDC1394);
#endif
}

#ifdef AVG_ENABLE_1394_2
void FWCamera::dumpCameraInfo(dc1394camera_t * pCamera)
{
    dc1394error_t err;
    dc1394featureset_t FeatureSet;
    err = dc1394_feature_get_all(pCamera, &FeatureSet);
    AVG_ASSERT(err == DC1394_SUCCESS);
    // TODO: do this using AVG_TRACE
    dc1394_feature_print_all(&FeatureSet, stderr);
}
#endif

void FWCamera::resetBus()
{
#ifdef AVG_ENABLE_1394_2
    dc1394_t* pDC1394 = dc1394_new();
    if (pDC1394 == 0) {
        return;
    }
    dc1394camera_list_t * pCameraList;
    int err=dc1394_camera_enumerate(pDC1394, &pCameraList);
    if (err == DC1394_SUCCESS) {
        if (pCameraList->num != 0) {
            dc1394camera_t * pCam = dc1394_camera_new(pDC1394, pCameraList->ids[0].guid);
            if (pCam) {
                dc1394_reset_bus(pCam);
                dc1394_camera_free(pCam);
            }
        }
        dc1394_camera_free_list(pCameraList);
    }
    dc1394_free(pDC1394);
#endif
}

}
