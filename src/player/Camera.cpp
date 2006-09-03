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

#include "../avgconfig.h"

#include "Camera.h"
#include "DisplayEngine.h"
#include "Player.h"
#include "ISurface.h"
#ifdef AVG_ENABLE_GL
#include "OGLSurface.h"
#endif

#include "../base/TimeSource.h"
#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/ScopeTimer.h"
#include "../base/XMLHelper.h"

#include <iostream>
#include <sstream>
#include <unistd.h>

using namespace std;

namespace avg {

#define MAX_PORTS 4
#define MAX_RESETS 10
#define DROP_FRAMES 1
#define NUM_BUFFERS 3

Camera::Camera ()
#ifdef AVG_ENABLE_1394
    : m_sDevice("Default"),
      m_FrameRate(15),
      m_sMode("640x480_RGB"),
      m_FWHandle(0)
#else
    : m_sDevice("Camera disabled"),
      m_FrameRate(30),
      m_sMode("---")
#endif
{
}

Camera::Camera (const xmlNodePtr xmlNode, Player * pPlayer)
    : VideoBase(xmlNode, pPlayer),
#ifdef AVG_ENABLE_1394
      m_sDevice(""),
      m_FrameRate(15),
      m_sMode("640x480_RGB"),
      m_FWHandle(0)
#else
      m_sDevice("Camera disabled"),
      m_FrameRate(15),
      m_sMode("---")
#endif
{
    m_sDevice = getDefaultedStringAttr (xmlNode, "device", "");
    m_FrameRate = getDefaultedDoubleAttr (xmlNode, "framerate", 15);
    m_sMode = getDefaultedStringAttr (xmlNode, "mode", "640x480_RGB");
    setFeature ("brightness", getDefaultedIntAttr(xmlNode, "brightness", -1));
    setFeature ("exposure", getDefaultedIntAttr(xmlNode, "exposure", -1));
    setFeature ("sharpness", getDefaultedIntAttr(xmlNode, "sharpness", -1));
    setFeature ("saturation", getDefaultedIntAttr(xmlNode, "saturation", -1));
    setFeature ("gamma", getDefaultedIntAttr(xmlNode, "gamma", -1));
    setFeature ("shutter", getDefaultedIntAttr(xmlNode, "shutter", -1));
    setFeature ("gain", getDefaultedIntAttr(xmlNode, "gain", -1));
    setFeature ("whitebalance", getDefaultedIntAttr(xmlNode, "whitebalance", -1));
}

Camera::~Camera ()
{
}

void Camera::setDisplayEngine(DisplayEngine * pEngine)
{
#ifdef AVG_ENABLE_1394
    if (m_FrameRate == 1.875) {
        m_FrameRateConstant = FRAMERATE_1_875;
    } else if (m_FrameRate == 3.75) {
        m_FrameRateConstant = FRAMERATE_3_75;
    } else if (m_FrameRate == 7.5) {
        m_FrameRateConstant = FRAMERATE_7_5;
    } else if (m_FrameRate == 15) {
        m_FrameRateConstant = FRAMERATE_15;
    } else if (m_FrameRate == 30) {
        m_FrameRateConstant = FRAMERATE_30;
    } else if (m_FrameRate == 60) {
        m_FrameRateConstant = FRAMERATE_60;
    } else {
        fatalError ("Unsupported or illegal value for camera framerate.");
    }
/*    if (m_sMode == "160x120_YUV444") {
        m_Mode = MODE_160x120_YUV444;
    } else if (m_sMode == "320x240_YUV422") {
        m_Mode = MODE_320x240_YUV422;
    } else
*/
    if (m_sMode == "640x480_YUV411") {
        m_Mode = MODE_640x480_YUV411;
    } else if (m_sMode == "640x480_YUV422") {
        m_Mode = MODE_640x480_YUV422;
    } else if (m_sMode == "640x480_RGB") {
        m_Mode = MODE_640x480_RGB;
/*    } else if (m_sMode == "640x480_MONO") {
        m_Mode = MODE_640x480_MONO;
    } else if (m_sMode == "640x480_MONO16") {
        m_Mode = MODE_640x480_MONO16;
*/        
    } else if (m_sMode == "1024x768_RGB") {
        m_Mode = MODE_1024x768_RGB;
    } else if (m_sMode == "1024x768_YUV422") {
        m_Mode = MODE_1024x768_YUV422;
    } else {
        fatalError (std::string("Unsupported or illegal value for camera mode \"") 
                + m_sMode + std::string("\"."));
    }
#else
    AVG_TRACE(Logger::ERROR,
            "Unable to set up camera. Camera support not compiled.");
#endif
    VideoBase::setDisplayEngine(pEngine);
}

string Camera::getTypeStr ()
{
    return "Camera";
}

unsigned int Camera::getFeature (const std::string& sFeature) const
{
#ifdef AVG_ENABLE_1394
    int FeatureID = getFeatureID(sFeature);
    unsigned int Value;
    int err;
    if (FeatureID == FEATURE_WHITE_BALANCE) {
        unsigned int u_b_value = 0;
        unsigned int v_r_value = 0;
        err = dc1394_get_white_balance(m_FWHandle, m_Camera.node, &u_b_value, &v_r_value);
        Value = ((u_b_value & 0xff) << 8) | (v_r_value & 0xff);
    } else {
        err = dc1394_get_feature_value(m_FWHandle, m_Camera.node, FeatureID, &Value);
    }
    if (err != DC1394_SUCCESS) {
        AVG_TRACE(Logger::WARNING, "Camera: Unable to get " << sFeature << 
                ". Error was " << err);
    }
    return Value;
#else
    return 0;
#endif
}

void Camera::setFeature (const std::string& sFeature, int Value)
{
#ifdef AVG_ENABLE_1394
    int FeatureID = getFeatureID(sFeature);
    m_Features[FeatureID] = Value;
    if (m_bCameraAvailable) {
        setFeature(FeatureID);
    }
#endif
}

void Camera::setFeature(int FeatureID)
{
#ifdef AVG_ENABLE_1394
    if (m_bCameraAvailable && m_FWHandle != 0) {
        int Value = m_Features[FeatureID];
        if (Value == -1) {
            dc1394_auto_on_off(m_FWHandle, m_Camera.node, FeatureID, 1);
        } else {
            dc1394_auto_on_off(m_FWHandle, m_Camera.node, FeatureID, 0);
            int err;
            if (FeatureID == FEATURE_WHITE_BALANCE) {
                unsigned int u_b_value = (Value >> 8) & 0xff;
                unsigned int v_r_value = Value & 0xff;
                err = dc1394_set_white_balance(m_FWHandle, m_Camera.node, 
                        u_b_value, v_r_value);
            } else {
                err = dc1394_set_feature_value(m_FWHandle, m_Camera.node, FeatureID, 
                        (unsigned int)Value);
            } 
            if (err != DC1394_SUCCESS) {
                AVG_TRACE(Logger::WARNING, "Camera: Unable to set " << FeatureID << 
                        ". Error was " << err);
            }
        }
    }
#endif
}

IntPoint Camera::getNativeSize() 
{
#ifdef AVG_ENABLE_1394
    switch(m_Mode) {
        case MODE_640x480_YUV411:
        case MODE_640x480_YUV422:
        case MODE_640x480_RGB:
            return IntPoint(640, 480);
        case MODE_1024x768_RGB:
        case MODE_1024x768_YUV422:
            return IntPoint(1024, 768);
        default:
            fatalError ("Camera::getNativeSize: Unsupported or illegal value for camera resolution:");
            return IntPoint(0,0);
    }
#else
    return IntPoint(640, 480);
#endif
}

double Camera::getFPS()
{
    return m_FrameRate;
}

void Camera::open(int* pWidth, int* pHeight)
{
    *pWidth = getNativeSize().x;
    *pHeight = getNativeSize().y;
    
#ifdef AVG_ENABLE_1394
    int CaptureFormat = 0;
    // TODO: Support other resolutions.
    switch(m_Mode) {
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
            fatalError ("Camera::open: Unsupported or illegal value for camera resolution:");
    }
            
    m_FWHandle = raw1394_new_handle();
    if (m_FWHandle==NULL) {
        AVG_TRACE(Logger::ERROR,
                "Unable to aquire a raw1394 handle (Node: "
                << getID() << ").");
        AVG_TRACE(Logger::ERROR, "Please check");
        AVG_TRACE(Logger::ERROR,
                "  - if the kernel modules `ieee1394',`raw1394' and `ohci1394' are loaded");
        AVG_TRACE(Logger::ERROR,
                "  - if you have read/write access to /dev/raw1394.");
        // TODO: Disable node instead of exit(-1).
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
        AVG_TRACE(Logger::WARNING,
                "No firewire cameras found (Node: " + getID() + ").");
        m_bCameraAvailable = false;
        if (m_FWHandle != 0) {
            dc1394_destroy_handle(m_FWHandle);
        }
        return;
    }
    m_bCameraAvailable = true;
    m_LastFrameTime = 0;
    if (j == MAX_RESETS) {
        fatalError(string("Failed to not make camera root node (Node: ")+
                getID() + ").");
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
                "video mode (" << m_sMode << ") and framerate (" <<
                m_FrameRate << ") are");
        AVG_TRACE(Logger::ERROR, "supported by your camera.");
        dc1394_dma_release_camera(m_FWHandle,&m_Camera);
        dc1394_destroy_handle(m_FWHandle);
        exit(-1);
    }

    err = dc1394_start_iso_transmission(m_FWHandle, m_Camera.node);
    checkDC1394Error(err, "Unable to start camera iso transmission");
#endif

    std::map<int, int>::iterator it;
    for (it=m_Features.begin(); it != m_Features.end(); ++it) {
        setFeature((*it).first);
    }
}

#ifdef AVG_ENABLE_1394
bool Camera::findCameraOnPort(int port, raw1394handle_t& FWHandle)
{
    bool bFound = false;
    FWHandle = dc1394_create_handle(port);
    if (FWHandle == NULL) {
        AVG_TRACE(Logger::ERROR,
                "Unable to aquire a raw1394 handle for port "
                << port << " (Node: " << getID() << ").");
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
                    "Resetting firewire bus for camera support... (Node: "
                    << getID() << ").");
            raw1394_reset_bus(FWHandle);
            sleep(2);
            bFound = false;
        }
        dc1394_free_camera_nodes(camera_nodes);
    }
    return bFound;
}
#endif

void Camera::close()
{
#ifdef AVG_ENABLE_1394
    if (m_bCameraAvailable) {
//        dc1394_stop_iso_transmission(m_FWHandle, m_Camera.node);
        dc1394_dma_unlisten(m_FWHandle, &m_Camera);
        dc1394_dma_release_camera(m_FWHandle, &m_Camera);
        dc1394_destroy_handle(m_FWHandle);
    }
#endif
}

#ifdef AVG_ENABLE_1394
void Camera::checkDC1394Error(int Code, const string & sMsg)
{
    if (Code != DC1394_SUCCESS) {
        fatalError(sMsg);
    }
}

void Camera::fatalError(const string & sMsg)
{
    AVG_TRACE(Logger::ERROR, sMsg);
    dc1394_destroy_handle(m_FWHandle);
    exit(-1);
}
#endif

static ProfilingZone CameraProfilingZone("    Camera::render");
static ProfilingZone CameraUploadProfilingZone("      Camera tex download");
static ProfilingZone CameraYUVConvertProfilingZone("      Camera YUV conversion");

bool Camera::renderToSurface(ISurface * pSurface)
{
#ifdef AVG_ENABLE_1394
    ScopeTimer Timer(CameraProfilingZone);
    if (m_bCameraAvailable) {
        int rc = dc1394_dma_single_capture(&m_Camera);
        if (rc == DC1394_SUCCESS) {
            m_LastFrameTime = TimeSource::get()->getCurrentMillisecs();
/*
#ifdef AVG_ENABLE_GL            
            OGLSurface * pOGLSurface = dynamic_cast<OGLSurface *>(pSurface);
#endif
*/
            // New frame available
            switch (m_Mode) {
                case MODE_640x480_YUV422:
                case MODE_1024x768_YUV422:
                    {
                        BitmapPtr pBmp = pSurface->lockBmp();
                        ScopeTimer Timer(CameraYUVConvertProfilingZone);
                        YUV422toBGR32((unsigned char *)(m_Camera.capture_buffer), pBmp);
                    }
                    break;
                case MODE_640x480_YUV411:
                    {
                        BitmapPtr pBmp = pSurface->lockBmp();
                        ScopeTimer Timer(CameraYUVConvertProfilingZone);
                        YUV411toBGR32((unsigned char *)(m_Camera.capture_buffer), pBmp);
                    }
                    break;
                case MODE_640x480_RGB:
                case MODE_1024x768_RGB:
                    {
/*
#ifdef AVG_ENABLE_GL
                        if (pOGLSurface) {
                            pOGLSurface->createFromBits(getNativeSize(), R8G8B8,
                                    (unsigned char *)(m_Camera.capture_buffer), 
                                    getNativeSize().x*3);
                        } else {
#endif
*/
                            BitmapPtr pBmp = pSurface->lockBmp();
                            unsigned char * pPixels = pBmp->getPixels();

                            if (getEngine()->hasRGBOrdering()) {
                                AVG_TRACE(Logger::ERROR,
                                        "Wrong engine rgb order for camera. Aborting.");
                            } else {
                                for (int y = 0; y < pBmp->getSize().y; y++) {
                                    unsigned char * pDestLine = 
                                        pPixels+y*pBmp->getStride();
                                    unsigned char * pSrcLine = (unsigned char*)
                                        m_Camera.capture_buffer+3*pBmp->getSize().x*y;
                                    for (int x = 0; x<pBmp->getSize().x; x++) {
                                        pDestLine[x*4] = pSrcLine[x*3+2];
                                        pDestLine[x*4+1] = pSrcLine[x*3+1];
                                        pDestLine[x*4+2] = pSrcLine[x*3];
                                        pDestLine[x*4+3] = 0xFF;
                                    }
                                }
                            }
/*
#ifdef AVG_ENABLE_GL
                        }
#endif
*/
                    }
                    break;
                default:
                    AVG_TRACE(Logger::WARNING,
                            "Illegal Mode in renderToSurface");
                    break;
            }
            pSurface->unlockBmps();
            {
                ScopeTimer Timer(CameraUploadProfilingZone);
                getEngine()->surfaceChanged(pSurface);
            }
            dc1394_dma_done_with_buffer(&m_Camera);
        } else {
            if (rc == DC1394_NO_FRAME) {
                AVG_TRACE(Logger::WARNING,
                        "Camera: Frame not available.");
            } else {
                AVG_TRACE(Logger::WARNING,
                        "Camera: Frame capture failed.");
            }
        }
    }
    if (m_LastFrameTime != 0 &&
        TimeSource::get()->getCurrentMillisecs() > m_LastFrameTime+3000)
    {
        AVG_TRACE(Logger::WARNING,
                "Camera: Reinitializing camera...");
        close();
        int Width, Height; // Dummys
        open(&Width, &Height);
        AVG_TRACE(Logger::WARNING,
                "Camera: Camera reinit done.");
    }
#endif
    return true;
}

PixelFormat Camera::getDesiredPixelFormat() 
{
    return R8G8B8X8;
}

bool Camera::canRenderToBackbuffer(int BitsPerPixel)
{
    return (BitsPerPixel == 24);
}

#ifdef AVG_ENABLE_1394
inline void YUVtoBGR32Pixel(Pixel32* pDest, 
        unsigned char y, unsigned char u, unsigned char v)
{
//    pDest->Set(y,y,y);
    // u = Cb, v = Cr
    int y1 = y - 16;
    int u1 = u - 128;
    int v1 = v - 128;
    int b = (298 * y1 + 516 * u1           ) >> 8;
    int g = (298 * y1 - 100 * u1 - 208 * v1) >> 8;
    int r = (298 * y1            + 409 * v1) >> 8;

    if (b<0) b = 0;
    if (b>255) b= 255;
    if (g<0) g = 0;
    if (g>255) g= 255;
    if (r<0) r = 0;
    if (r>255) r= 255;
    pDest->set(b,g,r);
}

void Camera::YUV411toBGR32Line(unsigned char* pSrc, int y, Pixel32 * pDestLine)
{
    Pixel32 * pDestPixel = pDestLine;
    int width = getMediaWidth();
    // We need the previous and next values to interpolate between the
    // sampled u and v values.
    unsigned char v = *(pSrc+y*(width*3)/2+3);
    unsigned char v0; // Previous v
    unsigned char v1; // Next v;
    unsigned char u;
    unsigned char u1; // Next u;
    unsigned char * pSrcPixels = pSrc+y*(width*3)/2;

    for (int x = 0; x < width/4; x++) {
        // Four pixels at a time.
        // Source format is UYYVYY.
        u = pSrcPixels[0];
        v0 = v;
        v = pSrcPixels[3];

        if (x < width/4-1) {
            u1 = pSrcPixels[6];
            v1 = pSrcPixels[9];
        } else {
            u1 = u;
            v1 = v;
        }

        YUVtoBGR32Pixel(pDestPixel, pSrcPixels[1], u, v0/2+v/2);
        YUVtoBGR32Pixel(pDestPixel+1, pSrcPixels[2], (u*3)/4+u1/4, v0/4+(v*3)/4);
        YUVtoBGR32Pixel(pDestPixel+2, pSrcPixels[4], u/2+u1/2, v);
        YUVtoBGR32Pixel(pDestPixel+3, pSrcPixels[5], u/4+(u1*3)/4, (v*3)/4+v1/4);

        pSrcPixels+=6;
        pDestPixel+=4;
    }

}

void Camera::YUV411toBGR32(unsigned char* pSrc, BitmapPtr pBmp)
{
    Pixel32 * pBits = (Pixel32 *)(pBmp->getPixels());
    for (int y = 0; y < getMediaHeight(); y++) {
        Pixel32 * pDest = pBits+(y*pBmp->getStride())/4;
        YUV411toBGR32Line(pSrc, y, pDest);
    }
}

void Camera::YUV422toBGR32Line(unsigned char* pSrc, int y, Pixel32 * pDestLine)
{
    Pixel32 * pDestPixel = pDestLine;
    int width = getMediaWidth();
    // We need the previous and next values to interpolate between the
    // sampled u and v values.
    unsigned char v = *(pSrc+y*(width*2)+2);
    unsigned char v0; // Previous v
    unsigned char v1; // Next v;
    unsigned char u;
    unsigned char u1; // Next u;
    unsigned char * pSrcPixels = pSrc+y*(width*2);

    for (int x = 0; x < width/2; x++) {
        // Two pixels at a time.
        // Source format is UYVY.
        u = pSrcPixels[0];
        v0 = v;
        v = pSrcPixels[2];

        if (x < width/2-1) {
            u1 = pSrcPixels[4];
            v1 = pSrcPixels[6];
        } else {
            u1 = u;
            v1 = v;
        }

        YUVtoBGR32Pixel(pDestPixel, pSrcPixels[1], u, v0/2+v/2);
        YUVtoBGR32Pixel(pDestPixel+1, pSrcPixels[3], u/2+u1/2, v);

        pSrcPixels+=4;
        pDestPixel+=2;
    }

}

void Camera::YUV422toBGR32(unsigned char* pSrc, BitmapPtr pBmp)
{
    Pixel32 * pBits = (Pixel32 *)(pBmp->getPixels());
    for (int y = 0; y < getMediaHeight(); y++) {
        Pixel32 * pDest = pBits+(y*pBmp->getStride())/4;
        YUV422toBGR32Line(pSrc, y, pDest);
    }
}

void Camera::dumpCameraInfo()
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

int Camera::getFeatureID(const std::string& sFeature) const
{
    if (sFeature == "brightness") {
        return FEATURE_BRIGHTNESS;
    } else if (sFeature == "exposure") {
        return FEATURE_EXPOSURE;
    } else if (sFeature == "sharpness") {
        return FEATURE_SHARPNESS;
    } else if (sFeature == "whitebalance") {
        return FEATURE_WHITE_BALANCE;
    } else if (sFeature == "hue") {
        return FEATURE_HUE;
    } else if (sFeature == "saturation") {
        return FEATURE_SATURATION;
    } else if (sFeature == "gamma") {
        return FEATURE_GAMMA;
    } else if (sFeature == "shutter") {
        return FEATURE_SHUTTER;
    } else if (sFeature == "gain") {
        return FEATURE_GAIN;
    } else if (sFeature == "iris") {
        return FEATURE_IRIS;
    } else if (sFeature == "focus") {
        return FEATURE_FOCUS;
    } else if (sFeature == "temperature") {
        return FEATURE_TEMPERATURE;
    } else if (sFeature == "trigger") {
        return FEATURE_TRIGGER;
    } else if (sFeature == "zoom") {
        return FEATURE_ZOOM;
    } else if (sFeature == "pan") {
        return FEATURE_PAN;
    } else if (sFeature == "tilt") {
        return FEATURE_TILT;
    } else if (sFeature == "optical_filter") {
        return FEATURE_OPTICAL_FILTER;
    } else if (sFeature == "capture_size") {
        return FEATURE_CAPTURE_SIZE;
    } else if (sFeature == "capture_quality") {
        return FEATURE_CAPTURE_QUALITY;
    }
    AVG_TRACE(Logger::WARNING, "Camera::getFeatureID: "+sFeature+" unknown.");
    return 0;
}

#endif

}
