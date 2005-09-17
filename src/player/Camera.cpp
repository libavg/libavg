//
// $Id$
//
// The 1394-specific code here was adapted from
// libdc1394/examples/grab_color_image.c and
// dc1394_multiview.c

#include "../avgconfig.h"

#include "Camera.h"
#include "IDisplayEngine.h"
#include "Player.h"
#include "Container.h"
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

// Precomputed conversion matrix entries
static int y2colTable[256]; // y to any color component
static int u2bTable[256]; // u to blue
static int u2gTable[256]; // u to green
static int v2gTable[256]; // v to green
static int v2rTable[256]; // v to red

#ifdef AVG_ENABLE_1394
bool Camera::m_bInitialized = false;
#endif

Camera::Camera ()
#ifdef AVG_ENABLE_1394
    : m_sDevice("Default"),
      m_FrameRate(15),
      m_sMode("640x480_RGB"),
      m_FWHandle(0)
#else
    : m_sDevice("Camera disabled"),
      m_FrameRate(15),
      m_sMode("---")
#endif
{
}

Camera::Camera (const xmlNodePtr xmlNode, Container * pParent)
    : VideoBase(xmlNode, pParent),
#ifdef AVG_ENABLE_1394
      m_sDevice("Default"),
      m_FrameRate(15),
      m_sMode("640x480_RGB"),
      m_FWHandle(0)
#else
      m_sDevice("Camera disabled"),
      m_FrameRate(15),
      m_sMode("---")
#endif
{
    m_sDevice = getDefaultedStringAttr (xmlNode, "device", "Default");
    m_FrameRate = getDefaultedDoubleAttr (xmlNode, "framerate", 15);
    m_sMode = getDefaultedStringAttr (xmlNode, "mode", "640x480_RGB");
    setFeature ("brightness", getDefaultedIntAttr(xmlNode, "brightness", -1));
    setFeature ("exposure", getDefaultedIntAttr(xmlNode, "exposure", -1));
    setFeature ("sharpness", getDefaultedIntAttr(xmlNode, "sharpness", -1));
    setFeature ("saturation", getDefaultedIntAttr(xmlNode, "saturation", -1));
    setFeature ("gamma", getDefaultedIntAttr(xmlNode, "gamma", -1));
    setFeature ("shutter", getDefaultedIntAttr(xmlNode, "shutter", -1));
    setFeature ("gain", getDefaultedIntAttr(xmlNode, "gain", -1));
}

Camera::~Camera ()
{
}

void Camera::init (IDisplayEngine * pEngine, Container * pParent,
        Player * pPlayer)
{
#ifdef AVG_ENABLE_1394
    initCameraSupport();
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
/*    if (sMode == "160x120_YUV444") {
        m_Mode = MODE_160x120_YUV444;
    } else if (sMode == "320x240_YUV422") {
        m_Mode = MODE_320x240_YUV422;
    } else
*/
    if (m_sMode == "640x480_YUV411") {
        m_Mode = MODE_640x480_YUV411;
    } /*else if (sMode == "640x480_YUV422") {
        m_Mode = MODE_640x480_YUV422;
    } */ else if (m_sMode == "640x480_RGB") {
        m_Mode = MODE_640x480_RGB;
    } /*else if (sMode == "640x480_MONO") {
        m_Mode = MODE_640x480_MONO;
    } else if (sMode == "640x480_MONO16") {
        m_Mode = MODE_640x480_MONO16;
    }*/ else {
        fatalError ("Unsupported or illegal value for camera mode.");
    }
#else
    AVG_TRACE(Logger::ERROR,
            "Unable to setup camera. Camera support not compiled.");
#endif
    VideoBase::init(pEngine, pParent, pPlayer);
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
    dc1394_get_feature_value(m_FWHandle, m_Camera.node, FeatureID, &Value);
    return Value;
#else
    return 0;
#endif
}

void Camera::setFeature (const std::string& sFeature, int Value)
{
#ifdef AVG_ENABLE_1394
    if (m_bCameraAvailable) {
        int FeatureID = getFeatureID(sFeature);
        if (m_FWHandle != 0) {
            if (Value == -1) {
                dc1394_auto_on_off(m_FWHandle, m_Camera.node, FeatureID, 1);
            } else {
                dc1394_auto_on_off(m_FWHandle, m_Camera.node, FeatureID, 0);
                dc1394_set_feature_value(m_FWHandle, m_Camera.node, FeatureID,
                        (unsigned int)Value);
            }
        }
    }
#endif
}

double Camera::getFPS()
{
    return m_FrameRate;
}

void Camera::open(int* pWidth, int* pHeight)
{
    // TODO: Support other resolutions.
    *pWidth=640;
    *pHeight=480;

#ifdef AVG_ENABLE_1394
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

    unsigned int channel;
    unsigned int speed;
    int err;
    err = dc1394_get_iso_channel_and_speed(m_FWHandle,
                m_Camera.node, &channel, &speed);
    checkDC1394Error(err,
            "Unable to get the firewire camera iso channel number.");

    err = dc1394_get_camera_feature_set(m_FWHandle,
                m_Camera.node, &m_FeatureSet);
    checkDC1394Error(err,
            "Unable to get firewire camera feature set.");
//    dumpCameraInfo();

    err = dc1394_dma_setup_capture(m_FWHandle, m_Camera.node,
                channel+1, FORMAT_VGA_NONCOMPRESSED, m_Mode,
                SPEED_400, m_FrameRateConstant, NUM_BUFFERS, DROP_FRAMES, 0,
                &m_Camera);
    if (err != DC1394_SUCCESS) {
        AVG_TRACE(Logger::ERROR,
                "Unable to setup camera. Make sure that");
        AVG_TRACE(Logger::ERROR,
                "video mode (" << m_sMode << ") and framerate (" <<
                m_FrameRate << ") are");
        AVG_TRACE(Logger::ERROR, "supported by your camera");
        dc1394_dma_release_camera(m_FWHandle,&m_Camera);
        dc1394_destroy_handle(m_FWHandle);
        exit(-1);
    }

    err = dc1394_start_iso_transmission(m_FWHandle, m_Camera.node);
    checkDC1394Error(err, "Unable to start camera iso transmission");
#endif
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

static ProfilingZone CameraProfilingZone("  Camera::render");
static ProfilingZone CameraUploadProfilingZone("    Camera::render tex upload");

bool Camera::renderToSurface(ISurface * pSurface)
{
#ifdef AVG_ENABLE_1394
    ScopeTimer Timer(CameraProfilingZone);
    if (m_bCameraAvailable) {
        int rc = dc1394_dma_single_capture(&m_Camera);
        /*
        if (rc == DC1394_NO_FRAME) {
            AVG_TRACE(Logger::WARNING,
                        "Camera: Frame delay.");
            usleep(10);
            rc = dc1394_dma_single_capture_poll(&m_Camera);
        }
        */
        //    int rc = dc1394_dma_single_capture(&m_Camera);
        if (rc == DC1394_SUCCESS) {
            m_LastFrameTime = TimeSource::get()->getCurrentTicks();
#ifdef AVG_ENABLE_GL            
            OGLSurface * pOGLSurface = dynamic_cast<OGLSurface *>(pSurface);
#endif            
            // New frame available
            switch (m_Mode) {
                case MODE_640x480_YUV411:
                    {
                        BitmapPtr pBmp = pSurface->getBmp();
                        YUV411toBGR24((unsigned char *)(m_Camera.capture_buffer), pBmp);
                    }
                    break;
                case MODE_640x480_RGB:
                    {
#ifdef AVG_ENABLE_GL                        
                        if (pOGLSurface) {
                            pOGLSurface->createFromBits(IntPoint(640, 480), R8G8B8,
                                    (unsigned char *)(m_Camera.capture_buffer), 640*3);
                        } else {
#endif                            
                            BitmapPtr pBmp = pSurface->getBmp();
                            unsigned char * pPixels = pBmp->getPixels();
                            int WidthBytes = pBmp->getSize().x*3;

                            if (getEngine()->hasRGBOrdering()) {
                                for (int y = 0; y < pBmp->getSize().y; y++) {
                                    memcpy(pPixels+y*pBmp->getStride(),
                                            (unsigned char*)(m_Camera.capture_buffer)+
                                            y*WidthBytes,
                                            WidthBytes);
                                }
                            } else {
                                for (int y = 0; y < pBmp->getSize().x; y++) {
                                    unsigned char * pDestLine = 
                                        pPixels+y*pBmp->getStride();
                                    unsigned char * pSrcLine = (unsigned char*)
                                        m_Camera.capture_buffer+y*WidthBytes;
                                    for (int x = 0; x < WidthBytes; x+=3) {
                                        pDestLine[x] = pSrcLine[x+2];
                                        pDestLine[x+1] = pSrcLine[x+1];
                                        pDestLine[x+2] = pSrcLine[x];
                                    }
                                }
                            }
#ifdef AVG_ENABLE_GL                            
                        }
#endif                        
                    }
                    break;
                default:
                    AVG_TRACE(Logger::WARNING,
                            "Illegal Mode in renderToBmp");
                    break;
            }
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
        TimeSource::get()->getCurrentTicks() > m_LastFrameTime+3000)
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

bool Camera::canRenderToBackbuffer(int BitsPerPixel)
{
    return (BitsPerPixel == 24);
}

#ifdef AVG_ENABLE_1394
inline void YUVtoBGR24Pixel(Pixel24* pDest, 
        unsigned char y, unsigned char u, unsigned char v)
{
//    pDest->Set(y,y,y);
    // u = Cb, v = Cr
/*    y -= 16;
    u -= 128;
    v -= 128;
    int b = (298 * y + 516 * u          ) >> 8;
    int g = (298 * y - 100 * u - 208 * v) >> 8;
    int r = (298 * y           + 409 * v) >> 8;
*/
    int ycomp = y2colTable[y];
    int b = ((ycomp+u2bTable[u])>>8);
    int g = ((ycomp+u2gTable[u]+v2gTable[v])>>8);
    int r = ((ycomp+v2rTable[v])>>8);

    if (b<0) b = 0;
    if (b>255) b= 255;
    if (g<0) g = 0;
    if (g>255) g= 255;
    if (r<0) r = 0;
    if (r>255) r= 255;
    pDest->Set(b,g,r);
/*
    asm volatile ("sub %%eax, %%eax;         \n\t"
                  "mov %1, %%al;             \n\t"
                  "imul $0x010101, %%eax;  \n\t"
                  "mov %0, %%ebx;            \n\t"
                  "movl %%eax, (%%ebx)       \n\t"
                  :
                  :"m"(pDest), "m"(y) // , "m"(byte2rgba_factor)
                  : "eax", "ebx", "memory");
*/
}

void Camera::YUV411toBGR24Line(unsigned char* pSrc, int y, Pixel24 * pDestLine)
{
    Pixel24 * pDestPixel = pDestLine;
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

        YUVtoBGR24Pixel(pDestPixel, pSrcPixels[1], u, v0/2+v/2);
        YUVtoBGR24Pixel(pDestPixel+1, pSrcPixels[2], (u*3)/4+u1/4, v0/4+(v*3)/4);
        YUVtoBGR24Pixel(pDestPixel+2, pSrcPixels[4], u/2+u1/2, v);
        YUVtoBGR24Pixel(pDestPixel+3, pSrcPixels[5], u/4+(u1*3)/4, (v*3)/4+v1/4);

        pSrcPixels+=6;
        pDestPixel+=4;
    }

}

void Camera::YUV411toBGR24(unsigned char* pSrc, BitmapPtr pBmp)
{
    Pixel24 * pBits = (Pixel24 *)(pBmp->getPixels());
    for (int y = 0; y < getMediaHeight(); y++) {
        Pixel24 * pDest = pBits+y*pBmp->getStride();
        YUV411toBGR24Line(pSrc, y, pDest);
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
    } else if (sFeature == "white_balance") {
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
    return 0;
}

void Camera::initCameraSupport()
{
    if (!m_bInitialized) {
        m_bInitialized = true;
        initYUV2RGBConversionMatrix();
    }
}

void Camera::initYUV2RGBConversionMatrix()
{
    // This is for the full 0...255 range

    for (int comp=0; comp<256; comp++) {
        y2colTable[comp] = (comp-16)*298;
        u2bTable[comp] = (comp-128)*516;
        u2gTable[comp] = -(comp-128)*100;
        v2gTable[comp] = -(comp-128)*208;
        v2rTable[comp] = (comp-128)*409;
    }

/*
         r = (256 * y            + 351 * cr) >> 8 + 16;
         g = (256 * y -  86 * cb - 179 * cr) >> 8 + 16;
         b = (256 * y + 444 * cb           ) >> 8 + 16;
*/
    // This is RGB235 mode.
/*
    for (int comp=0; comp<256; comp++) {
        y2colTable[comp] = (comp-16)*256;
        u2bTable[comp] = (comp-128)*444;
        u2gTable[comp] = -(comp-128)*86;
        v2gTable[comp] = -(comp-128)*179;
        v2rTable[comp] = (comp-128)*351;
    }
*/
}
#endif

}
