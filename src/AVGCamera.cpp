//
// $Id$
// 
// Most of this was adapted from libdc1394/examples/grab_color_image.c
// and dc1394_multiview.c

#include "AVGCamera.h"
#include "IAVGDisplayEngine.h"
#include "AVGException.h"
#include "AVGPlayer.h"
#include "AVGLogger.h"
#include "AVGContainer.h"

#include <paintlib/plbitmap.h>
#include <paintlib/pldirectfbbmp.h>
#include <paintlib/plpngenc.h>
#include <paintlib/planybmp.h>
#include <paintlib/Filter/plfilterfill.h>


#include <nsMemory.h>
#include <xpcom/nsComponentManagerUtils.h>

#include <iostream>
#include <sstream>
#include <unistd.h>

using namespace std;

#define MAX_PORTS 4
#define MAX_RESETS 10
#define DROP_FRAMES 0
#define NUM_BUFFERS 2


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
    m_sMode = sMode;
 
    AVGVideoBase::init(id, bOverlay, pEngine, pParent, pPlayer);
}

string AVGCamera::getTypeStr ()
{
    return "AVGCamera";
}

bool AVGCamera::open(int* pWidth, int* pHeight)
{
    int rc;

    m_FWHandle = raw1394_new_handle();
    if (m_FWHandle==NULL)
    {
        AVG_TRACE(AVGPlayer::DEBUG_ERROR, "Unable to aquire a raw1394 handle (Node: " 
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
    m_FWHandle = NULL;

    int j;
    int numCameras = 0;
    nodeid_t * camera_nodes = NULL;
    bool found = false;
    for (j = 0; j < MAX_RESETS && !found; j++)
    {
        /* look across all ports for cameras */
        for (int i = 0; i < numPorts && found == 0; i++)
        {
            if (m_FWHandle != NULL)
                dc1394_destroy_handle(m_FWHandle);
            m_FWHandle = dc1394_create_handle(i);
            if (m_FWHandle == NULL)
            {
                AVG_TRACE(AVGPlayer::DEBUG_ERROR, 
                        "Unable to aquire a raw1394 handle for port " 
                        << i << " (Node: " << getID() << ").");
                // TODO: Disable node instead of exit(-1).
                exit(-1);
            }
            numCameras = 0;
            camera_nodes = dc1394_get_camera_nodes(m_FWHandle, &numCameras, 0);
            if (numCameras > 0)
            {
                /* use the first camera found */
                m_Camera.node = camera_nodes[0];
                dumpCameraInfo(m_FWHandle, m_Camera.node);
                found = true;

                if (found)
                {
                    /* camera can not be root--highest order node */
                    if (m_Camera.node == raw1394_get_nodecount(m_FWHandle)-1)
                    {
                        /* reset and retry if root */
                        AVG_TRACE(AVGPlayer::DEBUG_WARNING, 
                                "Resetting firewire bus for camera support... (Node: " 
                                << getID() << ").");
                        raw1394_reset_bus(m_FWHandle);
                        sleep(2);
                        found = false;
                    }
                }
                dc1394_free_camera_nodes(camera_nodes);
            } /* cameras >0 */
        } /* next port */
    } /* next reset retry */

    if (numCameras == 0)
    {
        fatalError(string("No firewire cameras found (Node: ") + getID() + ").");
    }
    
    if (j == MAX_RESETS)
    {
        fatalError(string("Failed to not make camera root node (Node: ")+
                getID() + ").");
    }

    unsigned int channel;
    unsigned int speed;
    int err;
    err = dc1394_get_iso_channel_and_speed(m_FWHandle,
                m_Camera.node,
                &channel, &speed);
    checkDC1394Error(err, "Unable to get the firewire camera iso channel number");

    err = dc1394_dma_setup_capture(m_FWHandle, m_Camera.node,
                channel+1, FORMAT_VGA_NONCOMPRESSED, MODE_640x480_RGB,
                SPEED_400, FRAMERATE_15, NUM_BUFFERS, DROP_FRAMES, 0,
                &m_Camera);
    if (err != DC1394_SUCCESS) 
    {
        // TODO: Include current settings in message :-).
        AVG_TRACE(AVGPlayer::DEBUG_ERROR, 
                "Unable to setup camera. Make sure that");
        AVG_TRACE(AVGPlayer::DEBUG_ERROR, 
                "video mode, framerate and format are");
        AVG_TRACE(AVGPlayer::DEBUG_ERROR, "supported by your camera");
        dc1394_dma_release_camera(m_FWHandle,&m_Camera);
        dc1394_destroy_handle(m_FWHandle);
        exit(-1);
    }
    
    err = dc1394_start_iso_transmission(m_FWHandle, m_Camera.node);
    checkDC1394Error(err, "Unable to start camera iso transmission");
    
    // TODO: Support other resolutions.
    *pWidth=640;
    *pHeight=480;
}

void AVGCamera::close()
{
    dc1394_dma_unlisten(m_FWHandle, &m_Camera);
    dc1394_dma_release_camera(m_FWHandle, &m_Camera);
    dc1394_destroy_handle(m_FWHandle);
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
    // TODO: Disable node instead of exit(-1).
    exit(-1);
}

bool AVGCamera::renderToBmp(PLBmp * pBmp)
{
    int rc = dc1394_dma_single_capture_poll(&m_Camera);
    if (rc == DC1394_SUCCESS) {
        // New frame available
        PLBYTE ** ppLines = pBmp->GetLineArray();
        for (int y = 0; y < pBmp->GetHeight(); y++) {
            memcpy(ppLines[y],
                   (PLBYTE*)(m_Camera.capture_buffer)+y*pBmp->GetWidth()*3,
                    pBmp->GetWidth()*3);
        }
        dc1394_dma_done_with_buffer(&m_Camera);
        getEngine()->surfaceChanged(pBmp);
    } else {
        cerr << "Camera: Frame not available." << endl;
    }
    return true;
}

void AVGCamera::renderToBackbuffer(PLBYTE * pSurfBits, int Pitch, 
                int BytesPerPixel, const AVGDRect& vpt)
{
/*
    m_bEOF = m_pDecoder->renderToBuffer(pSurfBits, Pitch, 
            getEngine()->getBPP()/8, vpt);
    advancePlayback();
*/
}

void AVGCamera::dumpCameraInfo(raw1394handle_t PortHandle, nodeid_t CameraNode)
{
    dc1394_camerainfo info;
    int rc = dc1394_get_camera_info(PortHandle, CameraNode, &info);
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

    dc1394_feature_set features;
    if (dc1394_get_camera_feature_set(PortHandle, 
                CameraNode, &features) 
            != DC1394_SUCCESS) 
    {
        AVG_TRACE(AVGPlayer::DEBUG_WARNING, 
                "Unable to get firewire camera feature set.");
    } else {
        // TODO: do this using AVG_TRACE
        dc1394_print_feature_set(&features);
    }

}

void AVGCamera::initCameraSupport()
{
}

