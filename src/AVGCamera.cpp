//
// $Id$
// 
// The 1394-specific code here was adapted from 
// libdc1394/examples/grab_color_image.c and
// dc1394_multiview.c

#include "AVGCamera.h"
#include "IAVGDisplayEngine.h"
#include "AVGException.h"
#include "AVGPlayer.h"
#include "AVGLogger.h"
#include "AVGContainer.h"
#include "IAVGSurface.h"
#include "AVGOGLSurface.h"
#include "AVGTimeSource.h"

#include <paintlib/plbitmap.h>
#include <paintlib/plpngenc.h>
#include <paintlib/planybmp.h>
#include <paintlib/Filter/plfilterfill.h>
#include <paintlib/Filter/plfilterfliprgb.h>

#include <nsMemory.h>
#include <xpcom/nsComponentManagerUtils.h>

#include <iostream>
#include <sstream>
#include <unistd.h>

using namespace std;

#define MAX_PORTS 4
#define MAX_RESETS 10
#define DROP_FRAMES 0
#define NUM_BUFFERS 3

// Precomputed conversion matrix entries
static int y2colTable[256]; // y to any color component
static int u2bTable[256]; // u to blue
static int u2gTable[256]; // u to green
static int v2gTable[256]; // v to green
static int v2rTable[256]; // v to red

NS_IMPL_ISUPPORTS3_CI(AVGCamera, IAVGNode, IAVGVideoBase, IAVGCamera);

bool AVGCamera::m_bInitialized = false;

AVGCamera * AVGCamera::create()
{
    return createNode<AVGCamera>("@c-base.org/avgcamera;1");
}       

AVGCamera::AVGCamera ()
{
    NS_INIT_ISUPPORTS();
}

AVGCamera::~AVGCamera ()
{
}

NS_IMETHODIMP 
AVGCamera::GetType(PRInt32 *_retval)
{
    *_retval = NT_CAMERA;
    return NS_OK;
}

void AVGCamera::init (const std::string& id, const std::string& sDevice, 
        double frameRate, const std::string& sMode, bool bOverlay, 
        IAVGDisplayEngine * pEngine, AVGContainer * pParent, AVGPlayer * pPlayer)
{
    initCameraSupport();
    m_sDevice = sDevice;
    m_FrameRate = frameRate;
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
    m_sMode = sMode;
/*    if (sMode == "160x120_YUV444") {
        m_Mode = MODE_160x120_YUV444;
    } else if (sMode == "320x240_YUV422") {
        m_Mode = MODE_320x240_YUV422;
    } else 
*/
    if (sMode == "640x480_YUV411") {
        m_Mode = MODE_640x480_YUV411;
    } /*else if (sMode == "640x480_YUV422") {
        m_Mode = MODE_640x480_YUV422;
    } */ else if (sMode == "640x480_RGB") {
        m_Mode = MODE_640x480_RGB;
    } /*else if (sMode == "640x480_MONO") {
        m_Mode = MODE_640x480_MONO;
    } else if (sMode == "640x480_MONO16") {
        m_Mode = MODE_640x480_MONO16;
    }*/ else {
        fatalError ("Unsupported or illegal value for camera mode.");
    }
    
    AVGVideoBase::init(id, bOverlay, pEngine, pParent, pPlayer);
}

string AVGCamera::getTypeStr ()
{
    return "AVGCamera";
}

unsigned int AVGCamera::getFeature (const std::string& sFeature)
{
    int FeatureID = getFeatureID(sFeature);
    unsigned int Value;
    dc1394_get_feature_value(m_FWHandle, m_Camera.node, FeatureID, &Value);
    return Value;
}

void AVGCamera::setFeature (const std::string& sFeature, int Value)
{
    int FeatureID = getFeatureID(sFeature);
    if (Value == -1) {
        dc1394_auto_on_off(m_FWHandle, m_Camera.node, FeatureID, 1);
    } else {
        dc1394_auto_on_off(m_FWHandle, m_Camera.node, FeatureID, 0);
        dc1394_set_feature_value(m_FWHandle, m_Camera.node, FeatureID, 
            (unsigned int)Value);
    }
}

void AVGCamera::open(int* pWidth, int* pHeight)
{
    int rc;
    // TODO: Support other resolutions.
    *pWidth=640;
    *pHeight=480;
         
    m_FWHandle = raw1394_new_handle();
    if (m_FWHandle==NULL) {
        AVG_TRACE(AVGPlayer::DEBUG_ERROR,
                "Unable to aquire a raw1394 handle (Node: " 
                << getID() << ").");
        AVG_TRACE(AVGPlayer::DEBUG_ERROR, "Please check");
        AVG_TRACE(AVGPlayer::DEBUG_ERROR, 
                "  - if the kernel modules `ieee1394',`raw1394' and `ohci1394' are loaded");
        AVG_TRACE(AVGPlayer::DEBUG_ERROR, 
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
        AVG_TRACE(AVGPlayer::DEBUG_WARNING, 
                "No firewire cameras found (Node: " + getID() + ").");
        m_bCameraAvailable = false;
        dc1394_destroy_handle(m_FWHandle);
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
    dumpCameraInfo();

    err = dc1394_dma_setup_capture(m_FWHandle, m_Camera.node,
                channel+1, FORMAT_VGA_NONCOMPRESSED, m_Mode,
                SPEED_400, m_FrameRateConstant, NUM_BUFFERS, DROP_FRAMES, 0,
                &m_Camera);
    if (err != DC1394_SUCCESS) {
        AVG_TRACE(AVGPlayer::DEBUG_ERROR, 
                "Unable to setup camera. Make sure that");
        AVG_TRACE(AVGPlayer::DEBUG_ERROR, 
                "video mode (" << m_sMode << ") and framerate (" <<
                m_FrameRate << ") are");
        AVG_TRACE(AVGPlayer::DEBUG_ERROR, "supported by your camera");
        dc1394_dma_release_camera(m_FWHandle,&m_Camera);
        dc1394_destroy_handle(m_FWHandle);
        exit(-1);
    }
    
    err = dc1394_start_iso_transmission(m_FWHandle, m_Camera.node);
    checkDC1394Error(err, "Unable to start camera iso transmission");
    
}

bool AVGCamera::findCameraOnPort(int port, raw1394handle_t& FWHandle) 
{
    bool bFound = false;
    FWHandle = dc1394_create_handle(port);
    if (FWHandle == NULL) {
        AVG_TRACE(AVGPlayer::DEBUG_ERROR, 
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
            AVG_TRACE(AVGPlayer::DEBUG_WARNING, 
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

void AVGCamera::close()
{
    if (m_bCameraAvailable) {
        dc1394_dma_unlisten(m_FWHandle, &m_Camera);
        dc1394_dma_release_camera(m_FWHandle, &m_Camera);
        dc1394_destroy_handle(m_FWHandle);
    }
}

double AVGCamera::getFPS()
{
    return m_FrameRate;
}

void AVGCamera::checkDC1394Error(int Code, const string & sMsg)
{
    if (Code != DC1394_SUCCESS) {
        fatalError(sMsg);
    }
}

void AVGCamera::fatalError(const string & sMsg) 
{
    AVG_TRACE(AVGPlayer::DEBUG_ERROR, sMsg);
    dc1394_destroy_handle(m_FWHandle);
    exit(-1);
}

bool AVGCamera::renderToSurface(IAVGSurface * pSurface)
{
    if (m_bCameraAvailable) {
        int rc = dc1394_dma_single_capture_poll(&m_Camera);
        if (rc == DC1394_NO_FRAME) {
            AVG_TRACE(AVGPlayer::DEBUG_WARNING,
                        "Camera: Frame delay.");
            usleep(10);
            rc = dc1394_dma_single_capture_poll(&m_Camera);
        }
        //    int rc = dc1394_dma_single_capture(&m_Camera);
        if (rc == DC1394_SUCCESS) {
            m_LastFrameTime = AVGTimeSource::get()->getCurrentTicks();
            AVGOGLSurface * pOGLSurface = dynamic_cast<AVGOGLSurface *>(pSurface);
            // New frame available
            switch (m_Mode) {
                case MODE_640x480_YUV411:
                    {
                        PLBmpBase * pBmp = pSurface->getBmp();
                        YUV411toBGR24((PLBYTE*)(m_Camera.capture_buffer), pBmp);
                    }
                    break;
                case MODE_640x480_RGB:
                    {
                        if (pOGLSurface) {
                            pOGLSurface->createFromBits(640, 480, 24, false,
                                    (PLBYTE*)(m_Camera.capture_buffer), 640*3);
                        } else {
                            PLBmpBase * pBmp = pSurface->getBmp();
                            PLBYTE ** ppLines = pBmp->GetLineArray();
                            int WidthBytes = pBmp->GetWidth()*3;

                            if (getEngine()->hasRGBOrdering()) {
                                for (int y = 0; y < pBmp->GetHeight(); y++) {
                                    memcpy(ppLines[y],
                                            (PLBYTE*)(m_Camera.capture_buffer)+
                                            y*WidthBytes,
                                            WidthBytes);
                                }
                            } else {
                                for (int y = 0; y < pBmp->GetHeight(); y++) {
                                    PLBYTE * pDestLine = ppLines[y];
                                    PLBYTE * pSrcLine = (PLBYTE*)
                                        m_Camera.capture_buffer+y*WidthBytes;
                                    for (int x = 0; x < WidthBytes; x+=3) {
                                        pDestLine[x] = pSrcLine[x+2];
                                        pDestLine[x+1] = pSrcLine[x+1];
                                        pDestLine[x+2] = pSrcLine[x];
                                    }
                                }
                            }
                        }
                    }
                    break;
                default:
                    AVG_TRACE(AVGPlayer::DEBUG_WARNING, 
                            "Illegal Mode in renderToBmp");
                    break;
            }
            getEngine()->surfaceChanged(pSurface);
            dc1394_dma_done_with_buffer(&m_Camera);
        } else {
            if (rc == DC1394_NO_FRAME) {
                AVG_TRACE(AVGPlayer::DEBUG_WARNING,
                        "Camera: Frame not available.");
            } else {
                AVG_TRACE(AVGPlayer::DEBUG_WARNING,
                        "Camera: Frame capture failed.");

                //            fatalError("Frame capture failed.");
            }
        }
    } 
    if (m_LastFrameTime != 0 && 
        AVGTimeSource::get()->getCurrentTicks() > m_LastFrameTime+3000)
    {
        AVG_TRACE(AVGPlayer::DEBUG_WARNING,
                "Camera: Reinitializing camera...");
        close();
        int Width, Height; // Dummys
        open(&Width, &Height);
        AVG_TRACE(AVGPlayer::DEBUG_WARNING,
                "Camera: Camera reinit done.");
    }
    return true;
}

bool AVGCamera::canRenderToBackbuffer(int BitsPerPixel)
{
    return (BitsPerPixel == 24);
}

inline void YUVtoBGR24Pixel(PLPixel24* pDest, PLBYTE y, PLBYTE u, PLBYTE v)
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

void AVGCamera::YUV411toBGR24Line(PLBYTE* pSrc, int y, PLPixel24 * pDestLine)
{
    PLPixel24 * pDestPixel = pDestLine;
    int width = getMediaWidth();
    // We need the previous and next values to interpolate between the 
    // sampled u and v values.
    PLBYTE v = *(pSrc+y*(width*3)/2+3); 
    PLBYTE v0; // Previous v
    PLBYTE v1; // Next v;
    PLBYTE u;
    PLBYTE u1; // Next u;
    PLBYTE * pSrcPixels = pSrc+y*(width*3)/2;

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

void AVGCamera::YUV411toBGR24(PLBYTE* pSrc, PLBmpBase * pBmp)
{
    PLPixel24 ** ppBits = pBmp->GetLineArray24();
    for (int y = 0; y < getMediaHeight(); y++) {
        PLPixel24 * pDest = ppBits[y];
        YUV411toBGR24Line(pSrc, y, pDest);
    }
}

void AVGCamera::dumpCameraInfo()
{
    dc1394_camerainfo info;
    int rc = dc1394_get_camera_info(m_FWHandle, m_Camera.node, &info);
    if (rc == DC1394_SUCCESS)
    {
        AVG_TRACE(AVGPlayer::DEBUG_CONFIG, "Firewire camera:");
        AVG_TRACE(AVGPlayer::DEBUG_CONFIG, "  FW Node: " << info.id);
        /*
        // TODO: This prints the wrong UUID. Why?
        unsigned long val0 = info.euid_64 & 0xffffffff;
        unsigned long val1 = (info.euid_64 >>32) & 0xffffffff;
        AVG_TRACE(AVGPlayer::DEBUG_CONFIG, "  UUID: 0x" 
            << ios::hex << val1 << val0 << ios::dec);
         */                        
        AVG_TRACE(AVGPlayer::DEBUG_CONFIG, "  Vendor: " << info.vendor);
        AVG_TRACE(AVGPlayer::DEBUG_CONFIG, "  Model: " << info.model);
//        dc1394_print_camera_info(&info);
    } else {
        AVG_TRACE(AVGPlayer::DEBUG_ERROR, 
                "Unable to get firewire camera info.");
    }
    // TODO: do this using AVG_TRACE
    dc1394_print_feature_set(&m_FeatureSet);
}
 
int AVGCamera::getFeatureID(const std::string& sFeature)
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
}

void AVGCamera::initCameraSupport()
{
    if (!m_bInitialized) {
        m_bInitialized = true;
        initYUV2RGBConversionMatrix();
    }
}

void AVGCamera::initYUV2RGBConversionMatrix()
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

