//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
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

namespace avg {

#ifdef AVG_ENABLE_1394
#define MAX_PORTS 4
#define MAX_RESETS 10
#define DROP_FRAMES 1
#define NUM_BUFFERS 3
#endif

using namespace std;

FWCamera::FWCamera(string sDevice, double FrameRate, std::string sMode, bool bColor)
    : m_sDevice(sDevice),
      m_FrameRate(FrameRate),
      m_sMode(sMode),
      m_bColor(bColor),
      m_bCameraAvailable(false)
{
#if defined(AVG_ENABLE_1394) || defined(AVG_ENABLE_1394_2)
    m_FrameRateConstant = getFrameRateConst(m_FrameRate);
    m_Mode = getCamMode(m_sMode);
#endif
}

FWCamera::~FWCamera()
{
    close();
}

void FWCamera::open()
{
#ifdef AVG_ENABLE_1394
    int CaptureFormat = 0;
    // TODO: Support other resolutions.
    switch(m_Mode) {
        case MODE_640x480_MONO:
        case MODE_640x480_YUV422:
        case MODE_640x480_YUV411:
        case MODE_640x480_RGB:
            CaptureFormat=FORMAT_VGA_NONCOMPRESSED;
            break;
        case MODE_1024x768_RGB:
        case MODE_1024x768_YUV422:
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
        // TODO: Disable camera instead of exit(-1).
        exit(-1);
    }

    /* get the number of ports (cards) */
    struct raw1394_portinfo ports[MAX_PORTS];
    int numPorts = 0;
    numPorts = raw1394_get_port_info(m_FWHandle, ports, numPorts);
    raw1394_destroy_handle(m_FWHandle);
    m_FWHandle = 0;

    bool bFound = false;
    int j;
    for (j = 0; j < MAX_RESETS && !bFound; j++) {
        /* look across all ports for cameras */
        for (int i = 0; i < numPorts && !bFound; i++) {
            if (m_FWHandle != 0) {
                dc1394_destroy_handle(m_FWHandle);
            }
            bFound = findCameraOnPort(i, m_FWHandle);
        } /* next port */
    } /* next reset retry */

    if (!bFound) {
        static bool bFirstWarning = true;
        if (bFirstWarning) {
            AVG_TRACE(Logger::WARNING, "No firewire cameras found.");
            bFirstWarning = false;
        }
        m_bCameraAvailable = false;
        if (m_FWHandle != 0) {
            dc1394_destroy_handle(m_FWHandle);
        }
        return;
    }
    m_bCameraAvailable = true;
    if (j == MAX_RESETS) {
        AVG_TRACE(Logger::ERROR, "Failed to not make camera root node.");
        exit(1);
    }
    int err;
    err = dc1394_get_camera_feature_set(m_FWHandle,
                m_Camera.node, &m_FeatureSet);
    checkDC1394Error(err,
            "Unable to get firewire camera feature set.");
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
        exit(1);
    }

    err = dc1394_start_iso_transmission(m_FWHandle, m_Camera.node);
    checkDC1394Error(err, "Unable to start camera iso transmission");
#elif AVG_ENABLE_1394_2
    dc1394camera_t **ppCameras=NULL;
    uint32_t numCameras;
    m_bCameraAvailable = true;

    int err=dc1394_find_cameras(&ppCameras, &numCameras);

    if (err!=DC1394_SUCCESS && err != DC1394_NO_CAMERA) {
        AVG_TRACE(Logger::ERROR, "Unable to look for cameras");
#ifdef linux
        AVG_TRACE(Logger::ERROR, "Please check");
        AVG_TRACE(Logger::ERROR,
                "  - if the kernel modules `ieee1394',`raw1394' and `ohci1394' are loaded");
        AVG_TRACE(Logger::ERROR,
                "  - if you have read/write access to /dev/raw1394.");
#endif
        exit(1);
    }
    
    if (numCameras<1) {
        static bool bFirstWarning = true;
        if (bFirstWarning) {
            AVG_TRACE(Logger::WARNING, "No firewire cameras found.");
            bFirstWarning = false;
        }
        m_bCameraAvailable = false;
        return;
    }
    // This always uses the first camera on the bus.
    m_pCamera=ppCameras[0];

    // Free the other cameras
    for (unsigned int i=1;i<numCameras;i++)
        dc1394_free_camera(ppCameras[i]);
    free(ppCameras);

    err = dc1394_get_camera_feature_set(m_pCamera, &m_FeatureSet);
    checkDC1394Error(err,
            "Unable to get firewire camera feature set.");
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
        dc1394_free_camera(m_pCamera);
        exit(1);
    }

    err = dc1394_video_set_framerate(m_pCamera, m_FrameRateConstant);
    checkDC1394Error(err, "Unable to set camera framerate.");

    if (dc1394_capture_setup_dma(m_pCamera,8)!=DC1394_SUCCESS) {
        AVG_TRACE(Logger::ERROR,
                "Unable to setup camera. Make sure that");
        AVG_TRACE(Logger::ERROR,
                "video mode and framerate (" <<
                m_FrameRate << ") are");
        AVG_TRACE(Logger::ERROR, "supported by your camera.");
        dc1394_capture_stop(m_pCamera);
        dc1394_video_set_transmission(m_pCamera, DC1394_OFF);
        dc1394_free_camera(m_pCamera);
        exit(1);
    }
    if (dc1394_video_set_transmission(m_pCamera, DC1394_ON) !=DC1394_SUCCESS) {
        fatalError("Unable to start camera iso transmission\n");
    }

    dc1394switch_t status = DC1394_OFF;

    int i = 0;
    while( status == DC1394_OFF && i++ < 5 ) {
        usleep(50000);
        if (dc1394_video_get_transmission(m_pCamera, &status)!=DC1394_SUCCESS) {
            fatalError("unable to get transmision status\n");
        }
    }

    if( i == 5 ) {
        fatalError("Camera doesn't seem to want to turn on!\n");
    }
#endif
    AVG_TRACE(Logger::CONFIG, "Firewire camera opened.");
    for (FeatureMap::iterator it=m_Features.begin(); it != m_Features.end(); it++) {
        setFeature(it->first, it->second);
    }
}

void FWCamera::close()
{
    if (m_bCameraAvailable) {

#ifdef AVG_ENABLE_1394
        dc1394_dma_unlisten(m_FWHandle, &m_Camera);
//        dc1394_dma_release_camera(m_FWHandle, &m_Camera);
        dc1394_destroy_handle(m_FWHandle);
#elif AVG_ENABLE_1394_2
        dc1394_capture_stop(m_pCamera);
        dc1394_video_set_transmission(m_pCamera, DC1394_OFF);
        dc1394_free_camera(m_pCamera);
#endif
        m_bCameraAvailable = false;
        AVG_TRACE(Logger::CONFIG, "Firewire camera closed.");
    }
}

IntPoint FWCamera::getImgSize()
{
#if defined(AVG_ENABLE_1394) || defined(AVG_ENABLE_1394_2)
    return getCamImgSize(m_Mode);
#endif
    return IntPoint(640, 480);
}

static ProfilingZone CameraConvertProfilingZone("Camera format conversion");

BitmapPtr FWCamera::getImage(bool bWait)
{
#if defined(AVG_ENABLE_1394) || defined(AVG_ENABLE_1394_2)
    if (!m_bCameraAvailable && bWait) {
        TimeSource::get()->msleep(1000);
        open();
    }
    if (!m_bCameraAvailable) {
        // Open failed
        return BitmapPtr();
    }
    IntPoint ImgSize = getCamImgSize(m_Mode);
    BitmapPtr pCurBitmap;
    if (m_bColor) { 
        pCurBitmap = BitmapPtr(new Bitmap(ImgSize, B8G8R8X8));
    } else {
        pCurBitmap = BitmapPtr(new Bitmap(ImgSize, I8));
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
    if (bWait) {
        pFrame = dc1394_capture_dequeue_dma(m_pCamera, DC1394_VIDEO1394_WAIT);
    } else {
        pFrame = dc1394_capture_dequeue_dma(m_pCamera, DC1394_VIDEO1394_POLL);
    }
    if (pFrame) {
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
#else
                case DC1394_VIDEO_MODE_640x480_MONO8:
#endif                    
                    {
                        Bitmap TempBmp(ImgSize, I8, 
                                pCaptureBuffer,
                                ImgSize.x, false, "TempCameraBmp");
                        pCurBitmap->copyPixels(TempBmp);
                    }
                    break;
#ifdef AVG_ENABLE_1394
                case MODE_640x480_YUV422:
                case MODE_1024x768_YUV422:
#else
                case DC1394_VIDEO_MODE_640x480_YUV422:
                case DC1394_VIDEO_MODE_1024x768_YUV422:
#endif                    
                    {
                        Bitmap TempBmp(ImgSize, YCbCr422, 
                                pCaptureBuffer,
                                ImgSize.x*2, false, "TempCameraBmp");
                        pCurBitmap->copyPixels(TempBmp);
                    }
                    break;
#ifdef AVG_ENABLE_1394
                case MODE_640x480_YUV411:
#else
                case DC1394_VIDEO_MODE_640x480_YUV411:
#endif                    
                    {
                        Bitmap TempBmp(ImgSize, YCbCr411, 
                                pCaptureBuffer,
                                (int)(ImgSize.x*1.5), false, "TempCameraBmp");
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
                        int SrcStride = 3*ImgSize.x;
                        unsigned char * pDestLine = pCurBitmap->getPixels();
                        int DestStride = pCurBitmap->getStride();
                        for (int y = 0; y < ImgSize.y; y++) {
                            unsigned char * pDest = pDestLine;
                            unsigned char * pSrc = pSrcLine;
                            for (int x = 0; x<ImgSize.x; x++) {
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
        dc1394_capture_enqueue_dma (m_pCamera, pFrame);
#endif
        return pCurBitmap;
    } else {
        return BitmapPtr();
    }
#else
    return BitmapPtr();
#endif
}
    
bool FWCamera::isCameraAvailable()
{
    return m_bCameraAvailable;
}

const std::string& FWCamera::getDevice() const
{
    return m_sDevice;
}

double FWCamera::getFrameRate() const
{
    return m_FrameRate;
}

const std::string& FWCamera::getMode() const
{
    return m_sMode;
}

unsigned int FWCamera::getFeature(const std::string& sFeature) const
{
#if defined(AVG_ENABLE_1394) || defined(AVG_ENABLE_1394_2)
    dc1394feature_t FeatureID = getFeatureID(sFeature);
    FeatureMap::const_iterator it = m_Features.find(FeatureID);
    if (it == m_Features.end()) {
        return 0;
    } else {
        return it->second;
    }
#else
    return 0;
#endif
}

void FWCamera::setFeature(const std::string& sFeature, int Value)
{
#if defined(AVG_ENABLE_1394) || defined(AVG_ENABLE_1394_2)
    dc1394feature_t FeatureID = getFeatureID(sFeature);
    m_Features[FeatureID] = Value;
    if (m_bCameraAvailable) {
        setFeature(FeatureID, Value);
    }
#endif
}

void FWCamera::setFeature(dc1394feature_t Feature, int Value) {
#ifdef AVG_ENABLE_1394
    int err;
    if (Value == -1) {
        err = dc1394_auto_on_off(m_FWHandle, m_Camera.node, Feature, 1);
    } else {
        dc1394_auto_on_off(m_FWHandle, m_Camera.node, Feature, 0);
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
    } else {
        dc1394_feature_set_mode(m_pCamera, Feature, DC1394_FEATURE_MODE_MANUAL);
        err = dc1394_feature_set_value(m_pCamera, Feature, Value);
    }
    if (err != DC1394_SUCCESS) {
        AVG_TRACE(Logger::WARNING, "FWCamera: Unable to set " << Feature << 
                ". Error was " << err);
    }
#endif
}

#ifdef AVG_ENABLE_1394
bool FWCamera::findCameraOnPort(int port, raw1394handle_t& FWHandle)
{
    bool bFound = false;
    FWHandle = dc1394_create_handle(port);
    if (FWHandle == NULL) {
        AVG_TRACE(Logger::ERROR,
                "Unable to aquire a raw1394 handle for port "
                << port << ".");
        // TODO: Disable node instead of exit(-1).
        exit(-1);
    }
    int numCameras = 0;
    nodeid_t * camera_nodes = dc1394_get_camera_nodes(FWHandle, &numCameras, 0);
    if (numCameras > 0) {
        /* use the first camera found */
        m_Camera.node = camera_nodes[0];
        bFound = true;

        /* camera can not be root--highest order node */
        if (m_Camera.node == raw1394_get_nodecount(FWHandle)-1) {
            /* reset and retry if root */
            AVG_TRACE(Logger::WARNING,
                    "Resetting firewire bus for camera support...");
            raw1394_reset_bus(FWHandle);
            sleep(2);
            bFound = false;
        }
        dc1394_free_camera_nodes(camera_nodes);
    }
    return bFound;
}
#endif

void FWCamera::checkDC1394Error(int Code, const string & sMsg)
{
#if defined(AVG_ENABLE_1394) || defined(AVG_ENABLE_1394_2)
    if (Code != DC1394_SUCCESS) {
        fatalError(sMsg);
    }
#endif
}

void FWCamera::fatalError(const string & sMsg)
{
    AVG_TRACE(Logger::ERROR, sMsg);
    close();
    exit(1);
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
    dc1394_print_feature_set(&m_FeatureSet);
}

#elif AVG_ENABLE_1394_2
void FWCamera::dumpCameraInfo()
{
    // TODO: do this using AVG_TRACE
    dc1394_print_camera_info(m_pCamera);
    dc1394_print_feature_set(&m_FeatureSet);
}
#else
void FWCamera::dumpCameraInfo()
{
}
#endif


}
