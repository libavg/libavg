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

#include "VDPAUHelper.h"

#include "../base/Exception.h"
#include "../base/ConfigMgr.h"

#include "../graphics/Bitmap.h"

using namespace std;

namespace avg {

VdpGetProcAddress* vdp_get_proc_address;

VdpVideoSurfaceGetParameters* vdp_video_surface_get_parameters;
VdpVideoSurfaceGetBitsYCbCr* vdp_video_surface_get_bits_y_cb_cr;
VdpVideoSurfaceCreate* vdp_video_surface_create;
VdpVideoSurfaceDestroy* vdp_video_surface_destroy;

VdpDeviceDestroy* vdp_device_destroy;

VdpDecoderCreate* vdp_decoder_create;
VdpDecoderDestroy* vdp_decoder_destroy;
VdpDecoderRender* vdp_decoder_render;

VdpOutputSurfaceCreate* vdp_output_surface_create;
VdpOutputSurfaceDestroy* vdp_output_surface_destroy;
VdpOutputSurfaceGetBitsNative* vdp_output_surface_get_bits_native;
VdpOutputSurfaceGetParameters* vdp_output_surface_get_parameters;

VdpVideoMixerCreate* vdp_video_mixer_create;
VdpVideoMixerDestroy* vdp_video_mixer_destroy;
VdpVideoMixerRender* vdp_video_mixer_render;

VdpPresentationQueueCreate* vdp_presentation_queue_create;
VdpPresentationQueueDestroy* vdp_presentation_queue_destroy;
VdpPresentationQueueGetTime* vdp_presentation_queue_get_time;
VdpPresentationQueueTargetCreateX11* vdp_presentation_queue_target_create_x11;
VdpPresentationQueueQuerySurfaceStatus* vdp_presentation_queue_query_surface_status;
VdpPresentationQueueDisplay* vdp_presentation_queue_display;
VdpPresentationQueueBlockUntilSurfaceIdle*
        vdp_presentation_queue_block_until_surface_idle;


void safeGetProcAddress(VdpFuncId functionId, void** functionPointer)
{
    VdpStatus status;
    status = vdp_get_proc_address(getVDPAUDevice(), functionId, functionPointer);
    AVG_ASSERT(status == VDP_STATUS_OK);
}

VdpDevice getVDPAUDevice()
{
    static VdpDevice vdpDevice = 0;
    static bool bInitFailed = false;

    if (vdpDevice) {
        return vdpDevice;
    }

    if (bInitFailed) {
        return 0;
    }

    Display* pXDisplay = XOpenDisplay(0);
    AVG_ASSERT(pXDisplay);

    if (!(ConfigMgr::get()->getBoolOption("scr", "videoaccel", true))) {
        bInitFailed = true;
        return 0;
    }
    VdpStatus status;
    status = vdp_device_create_x11(pXDisplay, DefaultScreen(pXDisplay), &vdpDevice, 
            &vdp_get_proc_address);
    if (status != VDP_STATUS_OK)
    {
        bInitFailed = true;
        return 0;
    }

    safeGetProcAddress(VDP_FUNC_ID_DEVICE_DESTROY, (void**)&vdp_device_destroy);
    safeGetProcAddress(VDP_FUNC_ID_OUTPUT_SURFACE_CREATE,
            (void**)&vdp_output_surface_create);
    safeGetProcAddress(VDP_FUNC_ID_OUTPUT_SURFACE_DESTROY,
            (void**)&vdp_output_surface_destroy);
    safeGetProcAddress(VDP_FUNC_ID_OUTPUT_SURFACE_GET_BITS_NATIVE,
            (void**)&vdp_output_surface_get_bits_native);
    safeGetProcAddress(VDP_FUNC_ID_VIDEO_SURFACE_CREATE, 
            (void**)&vdp_video_surface_create);
    safeGetProcAddress(VDP_FUNC_ID_VIDEO_SURFACE_DESTROY,
            (void**)&vdp_video_surface_destroy);
    safeGetProcAddress(VDP_FUNC_ID_DECODER_CREATE, (void**)&vdp_decoder_create);
    safeGetProcAddress(VDP_FUNC_ID_DECODER_DESTROY, (void**)&vdp_decoder_destroy);
    safeGetProcAddress(VDP_FUNC_ID_DECODER_RENDER, (void**)&vdp_decoder_render);
    safeGetProcAddress(VDP_FUNC_ID_VIDEO_SURFACE_GET_BITS_Y_CB_CR,
            (void**)&vdp_video_surface_get_bits_y_cb_cr);
    safeGetProcAddress(VDP_FUNC_ID_VIDEO_MIXER_CREATE,
            (void**)&vdp_video_mixer_create);
    safeGetProcAddress(VDP_FUNC_ID_VIDEO_MIXER_DESTROY,
            (void**)&vdp_video_mixer_destroy);
    safeGetProcAddress(VDP_FUNC_ID_VIDEO_MIXER_RENDER,
            (void**)&vdp_video_mixer_render);
    safeGetProcAddress(VDP_FUNC_ID_PRESENTATION_QUEUE_CREATE, 
            (void**)&vdp_presentation_queue_create);
    safeGetProcAddress(VDP_FUNC_ID_PRESENTATION_QUEUE_DESTROY, 
            (void**)&vdp_presentation_queue_destroy);
    safeGetProcAddress(VDP_FUNC_ID_PRESENTATION_QUEUE_TARGET_CREATE_X11,
            (void**)&vdp_presentation_queue_target_create_x11);
    safeGetProcAddress(VDP_FUNC_ID_PRESENTATION_QUEUE_QUERY_SURFACE_STATUS, 
            (void**)&vdp_presentation_queue_query_surface_status);
    safeGetProcAddress(VDP_FUNC_ID_PRESENTATION_QUEUE_DISPLAY,
            (void**)&vdp_presentation_queue_display);
    safeGetProcAddress(VDP_FUNC_ID_PRESENTATION_QUEUE_GET_TIME,
            (void**)&vdp_presentation_queue_get_time);
    safeGetProcAddress(VDP_FUNC_ID_PRESENTATION_QUEUE_BLOCK_UNTIL_SURFACE_IDLE, 
            (void**)&vdp_presentation_queue_block_until_surface_idle);
    safeGetProcAddress(VDP_FUNC_ID_VIDEO_SURFACE_GET_PARAMETERS,
            (void**)&vdp_video_surface_get_parameters);
    safeGetProcAddress(VDP_FUNC_ID_OUTPUT_SURFACE_GET_PARAMETERS,
            (void**)&vdp_output_surface_get_parameters);
    return vdpDevice;
}

void getPlanesFromVDPAU(vdpau_render_state* pRenderState, BitmapPtr pBmpY,
        BitmapPtr pBmpU, BitmapPtr pBmpV)
{
    VdpStatus status;
    void *dest[3] = {
        pBmpY->getPixels(),
        pBmpV->getPixels(),
        pBmpU->getPixels()
    };
    uint32_t pitches[3] = {
        pBmpY->getStride(),
        pBmpV->getStride(),
        pBmpU->getStride()
    };
    status = vdp_video_surface_get_bits_y_cb_cr(pRenderState->surface,
            VDP_YCBCR_FORMAT_YV12, dest, pitches);
    AVG_ASSERT(status == VDP_STATUS_OK);
    unlockVDPAUSurface(pRenderState);
}

void getBitmapFromVDPAU(vdpau_render_state* pRenderState, BitmapPtr pBmpDest)
{
    IntPoint YSize = pBmpDest->getSize();
    IntPoint UVSize(YSize.x>>1, YSize.y);
    BitmapPtr pBmpY(new Bitmap(YSize, I8));
    BitmapPtr pBmpU(new Bitmap(UVSize, I8));
    BitmapPtr pBmpV(new Bitmap(UVSize, I8));
    getPlanesFromVDPAU(pRenderState, pBmpY, pBmpU, pBmpV);
    pBmpDest->copyYUVPixels(*pBmpY, *pBmpU, *pBmpV, false);
}   

void unlockVDPAUSurface(vdpau_render_state* pRenderState)
{
    pRenderState->state &= ~FF_VDPAU_STATE_USED_FOR_REFERENCE;
}

}
