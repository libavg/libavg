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
#if defined(AVG_ENABLE_1394) || defined(AVG_ENABLE_1394_2)
#include "FWCameraUtils.h"
#endif

#include "../base/Logger.h"
#include "../base/ScopeTimer.h"
#include "../base/TimeSource.h"
#include "../base/StringHelper.h"

#include <sstream>

namespace avg {

#ifdef AVG_ENABLE_1394
#define MAX_PORTS 4
#define MAX_RESETS 10
#define DROP_FRAMES 1
#define NUM_BUFFERS 3
#endif

using namespace std;

FWCamera::FWCamera(std::string sDevice, uint64_t Guid, int Unit, IntPoint Size, 
        std::string sPF, double FrameRate, bool bColor)
    : m_sDevice(sDevice),
      m_Guid(Guid),
      m_Unit(Unit),
      m_sPF(sPF),
      m_Size(Size),
      m_FrameRate(FrameRate),
      m_bColor(bColor),
      m_WhitebalanceU(-1),
      m_WhitebalanceV(-1)
{
#if defined(AVG_ENABLE_1394) || defined(AVG_ENABLE_1394_2)
    m_FrameRateConstant = getFrameRateConst(m_FrameRate);
    m_Mode = getCamMode(Size, sPF);
#endif
#ifdef AVG_ENABLE_1394
    int CaptureFormat = 0;
    switch(m_Mode) {
        case MODE_320x240_YUV422:
        case MODE_640x480_MONO:
        case MODE_640x480_YUV422:
        case MODE_640x480_YUV411:
        case MODE_640x480_RGB:
            CaptureFormat=FORMAT_VGA_NONCOMPRESSED;
            break;
        case MODE_1024x768_MONO:
        case MODE_1024x768_YUV422:
        case MODE_1024x768_RGB:
            CaptureFormat=FORMAT_SVGA_NONCOMPRESSED_1;
            break;
        default:
            fatalError ("FWCamera::open: Unsupported or illegal value for camera resolution:");
    }
            
    m_FWHandle = raw1394_new_handle();
    if (m_FWHandle==NULL) {
        AVG_TRACE(Logger::ERROR,
                "Unable to aquire a raw1394 handle.");
#ifdef linux
        AVG_TRACE(Logger::ERROR, "Please check");
        AVG_TRACE(Logger::ERROR,
                "  - if the kernel modules `ieee1394',`raw1394' and `ohci1394' are loaded");
        AVG_TRACE(Logger::ERROR,
                "  - if you have read/write access to /dev/raw1394.");
#endif
        throw Exception(AVG_ERR_CAMERA,"Unable to get a firewire handle");
    }

    /* get the number of ports (cards) */
    struct raw1394_portinfo ports[MAX_PORTS];
    int numPorts = 0;
    numPorts = raw1394_get_port_info(m_FWHandle, ports, numPorts);

    bool bFound = false;
    int j;
    for (j = 0; j < MAX_RESETS && !bFound; j++) {
        /* look across all ports for cameras */
        for (int i = 0; i < numPorts && !bFound; i++) {
            if (m_FWHandle != 0) {
                dc1394_destroy_handle(m_FWHandle);
            }
            bFound = findCameraOnPort(i);
        } /* next port */
    } /* next reset retry */

    if (!bFound) {
        static bool bFirstWarning = true;
        if (bFirstWarning) {
            AVG_TRACE(Logger::WARNING, "No firewire cameras found.");
            bFirstWarning = false;
        }
        if (m_FWHandle != 0) {
            dc1394_destroy_handle(m_FWHandle);
        }
        throw Exception(AVG_ERR_CAMERA,"No firewire cameras found.");
    }
    if (j == MAX_RESETS) {
        throw Exception(AVG_ERR_CAMERA,"Failed to not make camera the root node");
    }
    int err;
//    dumpCameraInfo();

    const char * pDeviceFileName = 0;
    if (m_sDevice != "") {
        pDeviceFileName = m_sDevice.c_str();
    }
    err = dc1394_dma_setup_capture(m_FWHandle, m_Camera.node,
                1, CaptureFormat, m_Mode,
                SPEED_400, m_FrameRateConstant, NUM_BUFFERS, DROP_FRAMES, 
                pDeviceFileName, &m_Camera);
    if (err != DC1394_SUCCESS) {
        AVG_TRACE(Logger::ERROR,
                "Unable to setup camera. Make sure that");
        AVG_TRACE(Logger::ERROR,
                "video mode and framerate (" <<
                m_FrameRate << ") are");
        AVG_TRACE(Logger::ERROR, "supported by your camera.");
//        dc1394_dma_release_camera(m_FWHandle,&m_Camera);
        dc1394_destroy_handle(m_FWHandle);
        throw Exception(AVG_ERR_CAMERA,"Failed to setup firewire camera");
    }

    err = dc1394_start_iso_transmission(m_FWHandle, m_Camera.node);
    checkDC1394Error(err, "Unable to start camera iso transmission");
#elif AVG_ENABLE_1394_2
    dc1394camera_list_t * pCameraList;

    m_pDC1394 = dc1394_new();
    int err=dc1394_camera_enumerate(m_pDC1394, &pCameraList);

    if (err!=DC1394_SUCCESS) {
        AVG_TRACE(Logger::ERROR, "Unable to look for cameras");
#ifdef linux
        AVG_TRACE(Logger::ERROR, "Please check");
        AVG_TRACE(Logger::ERROR,
                "  - if the kernel modules `ieee1394',`raw1394' and `ohci1394' are loaded");
        AVG_TRACE(Logger::ERROR,
                "  - if you have read/write access to /dev/raw1394.");
#endif
        dc1394_camera_free_list(pCameraList);
        throw Exception(AVG_ERR_CAMERA,"Firewire failure");
    }
    
    if (pCameraList->num == 0) {
        dc1394_camera_free_list(pCameraList);
        throw Exception(AVG_ERR_CAMERA,"No firewire cameras found.");
    }
    for(unsigned i=0; i<pCameraList->num;++i)
    {
        AVG_TRACE(Logger::CONFIG,"Found firewire camera guid="<<pCameraList->ids[i].guid<<" unit="<<pCameraList->ids[i].unit);
    }
    int id_to_use = -1;
    if (m_Guid != 0) {
        for(unsigned i=0; i<pCameraList->num;++i)
        {
            if (pCameraList->ids[i].guid == m_Guid) {
                id_to_use = i;
            }       
        }
        if ( id_to_use == -1 ){
            AVG_TRACE(Logger::WARNING, "Firewire GUID="<<m_Guid<<" requested but not found on bus. Using first camera");
            id_to_use = 0;
        }
    } else {
        id_to_use = 0;
    }
    if (m_Unit != -1) {
        m_pCamera = dc1394_camera_new_unit(m_pDC1394, pCameraList->ids[id_to_use].guid, m_Unit);
    } else {
        m_pCamera = dc1394_camera_new(m_pDC1394, pCameraList->ids[id_to_use].guid);
    }
    if (!m_pCamera) {
        dc1394_camera_free_list(pCameraList);
        throw Exception(AVG_ERR_CAMERA,"Failed to initialize camera");
    }

    dc1394_camera_free_list(pCameraList);

    dumpCameraInfo();

    err = dc1394_video_set_iso_speed(m_pCamera, DC1394_ISO_SPEED_400);
    checkDC1394Error(err, "Unable to set camera iso speed.");
    err = dc1394_video_set_mode(m_pCamera, m_Mode);
    checkDC1394Error(err, "Unable to set camera mode.");

    dc1394framerates_t FrameRates;
    err = dc1394_video_get_supported_framerates(m_pCamera, m_Mode, &FrameRates);
    checkDC1394Error(err, "Unable to get supported framerates.");
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
        throw Exception(AVG_ERR_CAMERA,"Failed to initialize camera");
    }

    err = dc1394_video_set_framerate(m_pCamera, m_FrameRateConstant);
    checkDC1394Error(err, "Unable to set camera framerate.");

    if (dc1394_capture_setup(m_pCamera,8, DC1394_CAPTURE_FLAGS_DEFAULT)!=DC1394_SUCCESS) {
        AVG_TRACE(Logger::ERROR, "Unable to setup camera. Make sure that");
        AVG_TRACE(Logger::ERROR, "video mode and framerate (" <<
                m_FrameRate << ") are");
        AVG_TRACE(Logger::ERROR, "supported by your camera.");
        dc1394_capture_stop(m_pCamera);
        dc1394_video_set_transmission(m_pCamera, DC1394_OFF);
        dc1394_camera_free(m_pCamera);
        throw Exception(AVG_ERR_CAMERA,"Failed to initialize camera");
    }
    if (dc1394_video_set_transmission(m_pCamera, DC1394_ON) !=DC1394_SUCCESS) {
        dc1394_capture_stop(m_pCamera);
        dc1394_video_set_transmission(m_pCamera, DC1394_OFF);
        dc1394_camera_free(m_pCamera);
        throw Exception(AVG_ERR_CAMERA,"Unable to start camera iso transmission");
    }

    dc1394switch_t status = DC1394_OFF;

    int i = 0;
    while( status == DC1394_OFF && i++ < 5 ) {
        usleep(50000);
        if (dc1394_video_get_transmission(m_pCamera, &status)!=DC1394_SUCCESS) {
        dc1394_capture_stop(m_pCamera);
        dc1394_video_set_transmission(m_pCamera, DC1394_OFF);
        dc1394_camera_free(m_pCamera);
        throw Exception(AVG_ERR_CAMERA,"unable to get transmision status");
        }
    }

    if( i == 5 ) {
        dc1394_capture_stop(m_pCamera);
        dc1394_video_set_transmission(m_pCamera, DC1394_OFF);
        dc1394_camera_free(m_pCamera);
        throw Exception(AVG_ERR_CAMERA,"Camera doesn't seem to want to turn on");
    }
#endif
    // Default to turning off any camera sharpness manipulation.
    setFeature(CAM_FEATURE_SHARPNESS, -1);
    AVG_TRACE(Logger::CONFIG, "Firewire camera opened.");
    for (FeatureMap::iterator it=m_Features.begin(); it != m_Features.end(); it++) {
        setFeature(it->first, it->second, true);
    }
    setWhitebalance(m_WhitebalanceU, m_WhitebalanceV, true);
}

FWCamera::~FWCamera()
{
#ifdef AVG_ENABLE_1394
    dc1394_dma_unlisten(m_FWHandle, &m_Camera);
//        dc1394_dma_release_camera(m_FWHandle, &m_Camera);
    dc1394_destroy_handle(m_FWHandle);
#elif AVG_ENABLE_1394_2
    dc1394_video_set_transmission(m_pCamera, DC1394_OFF);
    dc1394_capture_stop(m_pCamera);
    dc1394_camera_free(m_pCamera);
    dc1394_free(m_pDC1394);
#endif
    AVG_TRACE(Logger::CONFIG, "Firewire camera closed.");
}

IntPoint FWCamera::getImgSize()
{
#if defined(AVG_ENABLE_1394) || defined(AVG_ENABLE_1394_2)
    return m_Size;
#else
    return IntPoint(640, 480);
#endif
}


static ProfilingZone CameraConvertProfilingZone("FW Camera format conversion");

BitmapPtr FWCamera::getImage(bool bWait)
{
#if defined(AVG_ENABLE_1394) || defined(AVG_ENABLE_1394_2)
    BitmapPtr pCurBitmap;
    if (m_bColor || m_sPF == "BY8_GBRG") { 
        pCurBitmap = BitmapPtr(new Bitmap(m_Size, B8G8R8X8));
    } else {
        pCurBitmap = BitmapPtr(new Bitmap(m_Size, I8));
    }
    bool bGotFrame = false;
    unsigned char * pCaptureBuffer = 0;
#ifdef AVG_ENABLE_1394
    int rc;
    if (bWait) {
        rc = dc1394_dma_single_capture(&m_Camera);
    } else {
        rc = dc1394_dma_single_capture_poll(&m_Camera);
    }
    pCaptureBuffer = (unsigned char *)(m_Camera.capture_buffer);
    bGotFrame = (rc == DC1394_SUCCESS);
#else
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
#endif
    if (bGotFrame) {
        {
            ScopeTimer Timer(CameraConvertProfilingZone);
            switch (m_Mode) {
#ifdef AVG_ENABLE_1394
                case MODE_640x480_MONO:
                case MODE_1024x768_MONO:
#else
                case DC1394_VIDEO_MODE_640x480_MONO8:
                case DC1394_VIDEO_MODE_1024x768_MONO8:
#endif                    
                    {
                        // Bayer pattern conversion
                        if (m_sPF == "BY8_GBRG") {
                            Bitmap TempBmp(m_Size, BAYER8_GBRG, 
                                    pCaptureBuffer,
                                    m_Size.x, false, "TempCameraBmp");
                            pCurBitmap->copyPixels(TempBmp);
                        }
                        else {
                            Bitmap TempBmp(m_Size, I8, 
                                    pCaptureBuffer,
                                    m_Size.x, false, "TempCameraBmp");
                            pCurBitmap->copyPixels(TempBmp);
                        }
                    }
                    break;
#ifdef AVG_ENABLE_1394
                case MODE_320x240_YUV422:
                case MODE_640x480_YUV422:
                case MODE_1024x768_YUV422:
                case MODE_1280x960_YUV422:
#else
                case DC1394_VIDEO_MODE_320x240_YUV422:
                case DC1394_VIDEO_MODE_640x480_YUV422:
                case DC1394_VIDEO_MODE_1024x768_YUV422:
                case DC1394_VIDEO_MODE_1280x960_YUV422:
#endif                    
                    {
                        Bitmap TempBmp(m_Size, YCbCr422, 
                                pCaptureBuffer,
                                m_Size.x*2, false, "TempCameraBmp");
                        pCurBitmap->copyPixels(TempBmp);
                    }
                    break;
#ifdef AVG_ENABLE_1394
                case MODE_640x480_YUV411:
#else
                case DC1394_VIDEO_MODE_640x480_YUV411:
#endif                    
                    {
                        Bitmap TempBmp(m_Size, YCbCr411, 
                                pCaptureBuffer,
                                (int)(m_Size.x*1.5), false, "TempCameraBmp");
                        pCurBitmap->copyPixels(TempBmp);
                    }
                    break;
#ifdef AVG_ENABLE_1394
                case MODE_640x480_RGB:
                case MODE_1024x768_RGB:
#else
                case DC1394_VIDEO_MODE_640x480_RGB8:
                case DC1394_VIDEO_MODE_1024x768_RGB8:
#endif
                    {
                        unsigned char * pSrcLine = (unsigned char*)
                            pCaptureBuffer;
                        int SrcStride = 3*m_Size.x;
                        unsigned char * pDestLine = pCurBitmap->getPixels();
                        int DestStride = pCurBitmap->getStride();
                        for (int y = 0; y < m_Size.y; y++) {
                            unsigned char * pDest = pDestLine;
                            unsigned char * pSrc = pSrcLine;
                            for (int x = 0; x<m_Size.x; x++) {
                                *pDest++ = *(pSrc+2);
                                *pDest++ = *(pSrc+1);
                                *pDest++ = *pSrc;
                                *pDest++ = 0xFF;
                                pSrc += 3;
                            }
                            pSrcLine += SrcStride;
                            pDestLine += DestStride;
                        }
                    }
                    break;
                default:
                    AVG_TRACE(Logger::WARNING,
                            "Illegal Mode in renderToSurface");
                    break;
            }
        }
#ifdef AVG_ENABLE_1394
        dc1394_dma_done_with_buffer(&m_Camera);
#else
        dc1394_capture_enqueue(m_pCamera, pFrame);
#endif
        return pCurBitmap;
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
#elif AVG_ENABLE_1394
    dc1394_camerainfo info;
    dc1394_get_camera_info(m_FWHandle,m_Camera.node,&info);
    ss<<info.vendor<<" "<<info.model<<" (guid="<<info.euid_64<<")";
#endif
    deviceInfo = ss.str();
    return deviceInfo;
}

const std::string& FWCamera::getDriverName() const
{
#ifdef AVG_ENABLE_1394
    static string sDriverName = "libdc1394 v1";
    return sDriverName;
#elif AVG_ENABLE_1394_2
    static string sDriverName = "libdc1394 v2";
    return sDriverName;
#endif
}

double FWCamera::getFrameRate() const
{
    return m_FrameRate;
}

int FWCamera::getFeature(CameraFeature Feature) const
{
#if defined(AVG_ENABLE_1394) || defined(AVG_ENABLE_1394_2)
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
    if (bIgnoreOldValue || m_Features[Feature] != Value) {
        m_Features[Feature] = Value;
#if defined(AVG_ENABLE_1394) || defined(AVG_ENABLE_1394_2)
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
#endif
    }
}

void FWCamera::setFeatureOneShot(CameraFeature Feature)
{
#if defined(AVG_ENABLE_1394) 
    dc1394feature_t FeatureID = getFeatureID(Feature);
    dc1394_auto_on_off(m_FWHandle, m_Camera.node, Feature, 0);
    int err = dc1394_start_one_push_operation(m_FWHandle, m_Camera.node, FeatureID);
    if (err != DC1394_SUCCESS) {
        AVG_TRACE(Logger::WARNING, "Camera: Unable to set one-shot for " 
                << cameraFeatureToString(Feature) << ". Error was " << err);
    }
#elif defined(AVG_ENABLE_1394_2)
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
    if (bIgnoreOldValue || u != m_WhitebalanceU || v != m_WhitebalanceV) {
        m_WhitebalanceU = u;
        m_WhitebalanceV = v;
#ifdef AVG_ENABLE_1394
        int err;
        if (u == -1) {
            err = dc1394_auto_on_off(m_FWHandle, m_Camera.node, FEATURE_WHITE_BALANCE, 1);
        } else {
            err = dc1394_auto_on_off(m_FWHandle, m_Camera.node, FEATURE_WHITE_BALANCE, 0);
            err = dc1394_set_white_balance(m_FWHandle, m_Camera.node, u, v); 
        } 
        if (err != DC1394_SUCCESS) {
            AVG_TRACE(Logger::WARNING,
                    "Camera: Unable to set whitebalance. Error was " << err);
        }
#elif AVG_ENABLE_1394_2
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
#endif
    }
}

void FWCamera::setFeature(dc1394feature_t Feature, int Value)
{
#ifdef AVG_ENABLE_1394
    int err;
    if (Value == -1) {
        err = dc1394_auto_on_off(m_FWHandle, m_Camera.node, Feature, 1);
        err = dc1394_feature_on_off(m_FWHandle, m_Camera.node, Feature, 0);
    } else {
        dc1394_auto_on_off(m_FWHandle, m_Camera.node, Feature, 0);
        err = dc1394_feature_on_off(m_FWHandle, m_Camera.node, Feature, 1);
        err = dc1394_set_feature_value(m_FWHandle, m_Camera.node, Feature, 
                (unsigned int)Value);
    } 
    if (err != DC1394_SUCCESS) {
        AVG_TRACE(Logger::WARNING, "Camera: Unable to set " << Feature << 
                ". Error was " << err);
    }
#elif AVG_ENABLE_1394_2
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
        throw Exception(AVG_ERR_CAMERA, string("Illegal value ")+toString(microsecs)
                +" for strobe duration.");
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
        checkDC1394Error(err, "Unable to set camera PIO direction register.");
        
        uint32_t strobeRegValue = 0x83001000+durationRegValue;
        err = dc1394_set_strobe_register(m_pCamera, 0x200, strobeRegValue);
        checkDC1394Error(err, "Unable to set camera strobe register.");
    }
#else
    throw Exception(AVG_ERR_CAMERA, "setStrobeDuration not supported for libdc1394 v.1");
#endif
}

void FWCamera::getWhitebalance(int* pU, int* pV) const
{
#ifdef AVG_ENABLE_1394
    int err = dc1394_get_white_balance(m_FWHandle, m_Camera.node, 
            (uint32_t*)pU, (uint32_t*)pV); 
    checkDC1394Error(err, "Unable to read out whitebalance.");
#elif AVG_ENABLE_1394_2
    dc1394error_t err = dc1394_feature_whitebalance_get_value(m_pCamera, 
            (uint32_t*)pU, (uint32_t*)pV);
    checkDC1394Error(err, "Unable to read out whitebalance.");
#endif
}

#ifdef AVG_ENABLE_1394
bool FWCamera::findCameraOnPort(int port)
{
    bool bFound = false;
    m_FWHandle = dc1394_create_handle(port);
    if (m_FWHandle == NULL) {
        AVG_TRACE(Logger::ERROR,
                "Unable to aquire a raw1394 handle for port "
                << port << ".");

        throw Exception(AVG_ERR_CAMERA, "Unable to acquire a raw1394 handle");
    }
    int numCameras = 0;
    nodeid_t * camera_nodes = dc1394_get_camera_nodes(m_FWHandle, &numCameras, 0);
    if (numCameras <= 0) {
        return bFound;
    }
    int id_to_use = -1;
    if (m_Guid != 0) {
        for(int i=0; i<numCameras;++i)
        {       

            dc1394_camerainfo info;
            dc1394_get_camera_info(m_FWHandle, camera_nodes[i], &info);
            if (info.euid_64 == m_Guid) {
                id_to_use = i;
            }       
        }
        if ( id_to_use == -1 ){
            AVG_TRACE(Logger::WARNING, "Firewire GUID="<<m_Guid<<" requested but not found on bus. Using first camera");
            id_to_use = 0;
        }
    } else {
        id_to_use = 0;
    }
    m_Camera.node = camera_nodes[id_to_use];
    bFound = true;

    /* camera can not be root--highest order node */
    if (m_Camera.node == raw1394_get_nodecount(m_FWHandle)-1) {
        /* reset and retry if root */
        AVG_TRACE(Logger::WARNING,
                "Resetting firewire bus for camera support...");
        raw1394_reset_bus(m_FWHandle);
        sleep(2);
        bFound = false;
    }
    dc1394_free_camera_nodes(camera_nodes);
    return bFound;
}
#endif

void FWCamera::checkDC1394Error(int Code, const string & sMsg) const
{
#if defined(AVG_ENABLE_1394) || defined(AVG_ENABLE_1394_2)
    if (Code != DC1394_SUCCESS) {
        throw Exception(AVG_ERR_CAMERA, sMsg);
    }
#endif
}

void FWCamera::fatalError(const string & sMsg)
{
    AVG_TRACE(Logger::ERROR, sMsg);
    throw Exception(AVG_ERR_CAMERA, sMsg);
}

#ifdef AVG_ENABLE_1394
void FWCamera::dumpCameraInfo()
{
    dc1394_camerainfo info;
    int rc = dc1394_get_camera_info(m_FWHandle, m_Camera.node, &info);
    if (rc == DC1394_SUCCESS)
    {
        AVG_TRACE(Logger::CONFIG, "Firewire camera:");
        AVG_TRACE(Logger::CONFIG, "  FW Node: " << info.id);
        /*
        // TODO: This prints the wrong UUID. Why?
        unsigned long val0 = info.euid_64 & 0xffffffff;
        unsigned long val1 = (info.euid_64 >>32) & 0xffffffff;
        AVG_TRACE(Logger::CONFIG, "  UUID: 0x"
            << ios::hex << val1 << val0 << ios::dec);
         */
        AVG_TRACE(Logger::CONFIG, "  Vendor: " << info.vendor);
        AVG_TRACE(Logger::CONFIG, "  Model: " << info.model);
//        dc1394_print_camera_info(&info);
    } else {
        AVG_TRACE(Logger::ERROR,
                "Unable to get firewire camera info.");
    }
    // TODO: do this using AVG_TRACE
    dc1394_feature_set FeatureSet;
    rc = dc1394_get_camera_feature_set(m_FWHandle, m_Camera.node, &FeatureSet);
    checkDC1394Error(rc, "Unable to get firewire camera feature set.");
    dc1394_print_feature_set(&FeatureSet);
}

#elif AVG_ENABLE_1394_2
void FWCamera::dumpCameraInfo()
{
    dc1394error_t err;
    dc1394featureset_t FeatureSet;
    err = dc1394_feature_get_all(m_pCamera, &FeatureSet);
    checkDC1394Error(err, "Unable to get firewire camera feature set.");
    // TODO: do this using AVG_TRACE
    dc1394_feature_print_all(&FeatureSet, stderr);
}
#else
void FWCamera::dumpCameraInfo()
{
}
#endif

}
