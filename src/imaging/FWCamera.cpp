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

#include "FWCamera.h"
#ifdef AVG_ENABLE_1394_2
#include "FWCameraUtils.h"
#endif

#include "../base/Logger.h"
#include "../base/ScopeTimer.h"
#include "../base/TimeSource.h"
#include "../base/StringHelper.h"

#include <sstream>
#include <iomanip>

namespace avg {

using namespace std;

FWCamera::FWCamera(uint64_t guid, int unit, bool bFW800, IntPoint Size, 
        PixelFormat camPF, PixelFormat destPF, double FrameRate)
    : Camera(camPF, destPF),
      m_Size(Size),
      m_FrameRate(FrameRate),
      m_WhitebalanceU(-1),
      m_WhitebalanceV(-1)
{
#ifdef AVG_ENABLE_1394_2
    m_FrameRateConstant = getFrameRateConst(m_FrameRate);
    m_Mode = getCamMode(Size, camPF);
    dc1394camera_list_t * pCameraList;

    m_pDC1394 = dc1394_new();
    if (m_pDC1394 == 0) {
        throw Exception(AVG_ERR_CAMERA_NONFATAL, 
                "Failed to initialize firewire subsystem");
    }
    int err=dc1394_camera_enumerate(m_pDC1394, &pCameraList);

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
        for (unsigned i=0; i<pCameraList->num;++i) {
            if (pCameraList->ids[i].guid == guid) {
                id_to_use = i;
            }       
        }
        if (id_to_use == -1) {
            AVG_TRACE(Logger::WARNING, "Firewire GUID=" << guid 
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

    dumpCameraInfo();
    if (bFW800) {
        dc1394_video_set_operation_mode(m_pCamera, DC1394_OPERATION_MODE_1394B);
        err = dc1394_video_set_iso_speed(m_pCamera, DC1394_ISO_SPEED_800);
    } else {
        err = dc1394_video_set_iso_speed(m_pCamera, DC1394_ISO_SPEED_400);
    }
    assert(err == DC1394_SUCCESS);
    err = dc1394_video_set_mode(m_pCamera, m_Mode);
    assert(err == DC1394_SUCCESS);

    dc1394framerates_t FrameRates;
    err = dc1394_video_get_supported_framerates(m_pCamera, m_Mode, &FrameRates);
    assert(err == DC1394_SUCCESS);
    bool bFrameRateSupported = false;
    for (unsigned int i=0; i<FrameRates.num; i++) {
        if (FrameRates.framerates[i] == m_FrameRateConstant) {
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
    assert(err == DC1394_SUCCESS);

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
    err = dc1394_video_set_transmission(m_pCamera, DC1394_ON);
    assert(err == DC1394_SUCCESS);

    dc1394switch_t status = DC1394_OFF;

    int i = 0;
    while( status == DC1394_OFF && i++ < 5 ) {
        usleep(50000);
        err = dc1394_video_get_transmission(m_pCamera, &status);
        assert(err == DC1394_SUCCESS);
    }

    if( i == 5 ) {
        assert(false);
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
    if (camPF == BAYER8) {
        enablePtGreyBayer();
    }
#else
    assert(false);
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

IntPoint FWCamera::getImgSize()
{
#ifdef AVG_ENABLE_1394_2
    return m_Size;
#else
    return IntPoint(0, 0);
#endif
}


static ProfilingZone CameraConvertProfilingZone("FW Camera format conversion");

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
//        cerr << "CamBmp: " << pCamBmp->getPixelFormatString() << ", DestBmp: " 
//                << pDestBmp->getPixelFormatString() << endl;
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
    ss<<m_pCamera->vendor<<" "<<m_pCamera->model<<" (guid="<<m_pCamera->guid<<", unit="<<m_pCamera->unit<<")";
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

int FWCamera::getFeature(CameraFeature Feature) const
{
#ifdef AVG_ENABLE_1394_2
    FeatureMap::const_iterator it = m_Features.find(Feature);
    if (it == m_Features.end()) {
        return 0;
    } else {
        return it->second;
    }
#else
    return 0;
#endif
}

void FWCamera::setFeature(CameraFeature Feature, int Value, bool bIgnoreOldValue)
{
#ifdef AVG_ENABLE_1394_2
    if (bIgnoreOldValue || m_Features[Feature] != Value) {
        m_Features[Feature] = Value;
        if (Feature == CAM_FEATURE_STROBE_DURATION) {
            try {
                setStrobeDuration(Value);
            } catch (Exception& e) {
                AVG_TRACE(Logger::WARNING, 
                        string("Camera: Setting strobe duration failed. ")+e.GetStr());
            }
        } else {
            dc1394feature_t FeatureID = getFeatureID(Feature);
            setFeature(FeatureID, Value);
            //        dumpCameraInfo();
        }
    }
#endif
}

void FWCamera::setFeatureOneShot(CameraFeature Feature)
{
#ifdef AVG_ENABLE_1394_2
    dc1394feature_t FeatureID = getFeatureID(Feature);
    dc1394error_t err = dc1394_feature_set_mode(m_pCamera, FeatureID, 
            DC1394_FEATURE_MODE_ONE_PUSH_AUTO);
    if (err != DC1394_SUCCESS) {
        AVG_TRACE(Logger::WARNING, "Camera: Unable to set one-shot for " 
                << cameraFeatureToString(Feature) << ". Error was " << err);
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
#endif
}

void FWCamera::setFeature(dc1394feature_t Feature, int Value)
{
#ifdef AVG_ENABLE_1394_2
    dc1394error_t err;
    if (Value == -1) {
        err = dc1394_feature_set_mode(m_pCamera, Feature, DC1394_FEATURE_MODE_AUTO);
        err = dc1394_feature_set_power(m_pCamera, Feature, DC1394_OFF);
    } else {
        dc1394_feature_set_mode(m_pCamera, Feature, DC1394_FEATURE_MODE_MANUAL);
        err = dc1394_feature_set_power(m_pCamera, Feature, DC1394_ON);
        err = dc1394_feature_set_value(m_pCamera, Feature, Value);
    }
    if (err != DC1394_SUCCESS) {
        AVG_TRACE(Logger::WARNING, "Camera: Unable to set " << Feature << 
                ". Error was " << err);
    }
/*
    dc1394feature_info_t featureInfo;
    featureInfo.id = Feature;
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
            assert(len == sizeof(realTimes)/sizeof(*realTimes));
            int i;
            for (i=1; realTimes[i] < targetMillisecs; ++i); 
            double ratio = (targetMillisecs-realTimes[i])/(realTimes[i-1]-realTimes[i]);
            durationRegValue = ratio*regValues[i-1]+(1-ratio)*regValues[i];
        } 

        err = dc1394_set_PIO_register(m_pCamera, 0x08, 0xC0000000);
        assert(err == DC1394_SUCCESS);
        
        uint32_t strobeRegValue = 0x83001000+durationRegValue;
        err = dc1394_set_strobe_register(m_pCamera, 0x200, strobeRegValue);
        assert(err == DC1394_SUCCESS);
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
    assert(err == DC1394_SUCCESS);
    if (imageDataFormat & 0x80000000) {
        err = dc1394_set_adv_control_register(m_pCamera, 0x48, 0x80000081);
        assert(err == DC1394_SUCCESS);
        uint32_t bayerFormat;
        err = dc1394_get_adv_control_register(m_pCamera, 0x40, &bayerFormat);
        assert(err == DC1394_SUCCESS);
        PixelFormat exactPF = fwBayerStringToPF(bayerFormat);
        if (exactPF == I8) {
            throw(Exception(AVG_ERR_CAMERA_NONFATAL, 
                    "Greyscale camera doesn't support bayer pattern."));
        }
        setCamPF(exactPF);
    }
#endif
}

void FWCamera::dumpCameraInfo()
{
#ifdef AVG_ENABLE_1394_2
    dc1394error_t err;
    dc1394featureset_t FeatureSet;
    err = dc1394_feature_get_all(m_pCamera, &FeatureSet);
    assert(err == DC1394_SUCCESS);
    // TODO: do this using AVG_TRACE
    dc1394_feature_print_all(&FeatureSet, stderr);

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

    if (err != DC1394_SUCCESS) {
        dc1394_free(pDC1394);
        return;
    }
    if (pCameraList->num == 0) {
        dc1394_camera_free_list(pCameraList);
        dc1394_free(pDC1394);
        return;
    }
    cerr << "Firewire cameras: " << endl;
    for (unsigned i=0; i<pCameraList->num;++i) {
        dc1394camera_id_t id = pCameraList->ids[i];
        dc1394camera_t * pCamera = dc1394_camera_new_unit(pDC1394, id.guid, id.unit);
        if (pCamera) {
            dc1394_camera_print_info(pCamera, stderr);
            dc1394_camera_free(pCamera);
        }

//        AVG_TRACE(Logger::CONFIG, "Found firewire camera guid="<<pCameraList->ids[i].guid<<" unit="<<pCameraList->ids[i].unit);
    }
    dc1394_camera_free_list(pCameraList);
    dc1394_free(pDC1394);
#endif
}

}
