//
// $Id$
// 
// Most of this was adapted from libdc1394/examples/grab_color_image.c

#include "AVGCamera.h"
#include "AVGDFBDisplayEngine.h"
#include "AVGException.h"
#include "AVGPlayer.h"
#include "AVGLogger.h"
#include "AVGContainer.h"

#include <paintlib/plbitmap.h>
#include <paintlib/pldirectfbbmp.h>
#include <paintlib/plpngenc.h>
#include <paintlib/planybmp.h>
#include <paintlib/Filter/plfilterfill.h>

#include <libraw1394/raw1394.h>
#include <libdc1394/dc1394_control.h>

#include <nsMemory.h>
#include <xpcom/nsComponentManagerUtils.h>

#include <iostream>
#include <sstream>
#include <unistd.h>

using namespace std;

#define MAX_PORTS 4
#define MAX_RESETS 10


NS_IMPL_ISUPPORTS2_CI(AVGCamera, IAVGNode, IAVGVideoBase);

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
    // TODO: Support more than one camera.
    u_int64_t g_guid = 0;

    dc1394_cameracapture camera;

    raw1394handle_t handle = raw1394_new_handle();
    if (handle==NULL)
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
    numPorts = raw1394_get_port_info(handle, ports, numPorts);
    raw1394_destroy_handle(handle);
    handle = NULL;

    int j;
    int numCameras = 0;
    nodeid_t * camera_nodes = NULL;
    bool found = false;
    for (j = 0; j < MAX_RESETS && !found; j++)
    {
        /* look across all ports for cameras */
        for (int i = 0; i < numPorts && found == 0; i++)
        {
            if (handle != NULL)
                dc1394_destroy_handle(handle);
            handle = dc1394_create_handle(i);
            if (handle == NULL)
            {
                AVG_TRACE(AVGPlayer::DEBUG_ERROR, "Unable to aquire a raw1394 handle for port " 
                       << i << " (Node: " << getID() << ").");
                // TODO: Disable node instead of exit(-1).
                exit(-1);
            }
            numCameras = 0;
            camera_nodes = dc1394_get_camera_nodes(handle, &numCameras, 0);
            if (numCameras > 0)
            {
                if (g_guid == 0)
                {
                    dc1394_camerainfo info;
                    /* use the first camera found */
                    camera.node = camera_nodes[0];
                    if (dc1394_get_camera_info(handle, camera_nodes[0], &info) 
                        == DC1394_SUCCESS) 
                    {
                        // TODO: Replace with correct logging.
                        dc1394_print_camera_info(&info);
                    }
                    found = true;
                }
                else
                {
                    /* attempt to locate camera by guid */
                    // This isn't active right now since guid is always 0.
                    int k;
                    for (k = 0; k < numCameras && found == 0; k++)
                    {
                        dc1394_camerainfo info;
                        if (dc1394_get_camera_info(handle, camera_nodes[k], &info) == DC1394_SUCCESS)
                        {
                            if (info.euid_64 == g_guid)
                            {
                                dc1394_print_camera_info(&info);
                                camera.node = camera_nodes[k];
                                found = true;
                            }
                        }
                    }
                }
                if (found)
                {
                    /* camera can not be root--highest order node */
                    if (camera.node == raw1394_get_nodecount(handle)-1)
                    {
                        /* reset and retry if root */
                        AVG_TRACE(AVGPlayer::DEBUG_WARNING, 
                                "Resetting firewire bus for camera support... (Node: " 
                                << getID() << ").");
                        raw1394_reset_bus(handle);
                        sleep(2);
                        found = false;
                    }
                }
                dc1394_free_camera_nodes(camera_nodes);
            } /* cameras >0 */
        } /* next port */
    } /* next reset retry */

    if (!found && g_guid != 0)
    {
        AVG_TRACE(AVGPlayer::DEBUG_ERROR, "Unable to locate camera node by guid (Node: " 
                << getID() << ").");
        // TODO: Disable node instead of exit(-1).
        exit(-1);
    }
    else if (numCameras == 0)
    {
        AVG_TRACE(AVGPlayer::DEBUG_ERROR, "No firewire cameras found (Node: " 
                << getID() << ").");
        dc1394_destroy_handle(handle);
        // TODO: Disable node instead of exit(-1).
        exit(-1);
    }
    if (j == MAX_RESETS)
    {
        AVG_TRACE(AVGPlayer::DEBUG_ERROR, "failed to not make camera root node (Node: " 
                << getID() << ").");
        dc1394_destroy_handle(handle);
        // TODO: Disable node instead of exit(-1).
        exit(-1);
    }


}

void AVGCamera::close()
{
}

double AVGCamera::getFPS()
{
    return m_FrameRate;
}

bool AVGCamera::renderToBmp(PLBmp * pBmp)
{
/*    
    m_bEOF = m_pDecoder->renderToBmp(pBmp);
    if (!m_bEOF) {
        getEngine()->surfaceChanged(pBmp);
    }
    advancePlayback();
    return !m_bEOF;
*/
    return false;
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

void AVGCamera::initCameraSupport()
{
}

