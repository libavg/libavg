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
#ifndef _VDPAUHelper_H_
#define _VDPAUHelper_H_


#include "../avgconfigwrapper.h"

#include <vdpau/vdpau.h>
#include <libavcodec/vdpau.h>
#include <boost/shared_ptr.hpp>

namespace avg {

class Bitmap;
typedef boost::shared_ptr<Bitmap> BitmapPtr;

extern VdpGetProcAddress* vdp_get_proc_address;

extern VdpVideoSurfaceGetParameters* vdp_video_surface_get_parameters;
extern VdpVideoSurfaceGetBitsYCbCr* vdp_video_surface_get_bits_y_cb_cr;
extern VdpVideoSurfaceCreate* vdp_video_surface_create;
extern VdpVideoSurfaceDestroy* vdp_video_surface_destroy;

extern VdpDeviceDestroy* vdp_device_destroy;

extern VdpDecoderCreate* vdp_decoder_create;
extern VdpDecoderDestroy* vdp_decoder_destroy;
extern VdpDecoderRender* vdp_decoder_render;

extern VdpOutputSurfaceCreate* vdp_output_surface_create;
extern VdpOutputSurfaceDestroy* vdp_output_surface_destroy;
extern VdpOutputSurfaceGetBitsNative* vdp_output_surface_get_bits_native;
extern VdpOutputSurfaceGetParameters* vdp_output_surface_get_parameters;

extern VdpVideoMixerCreate* vdp_video_mixer_create;
extern VdpVideoMixerDestroy* vdp_video_mixer_destroy;
extern VdpVideoMixerRender* vdp_video_mixer_render;

extern VdpPresentationQueueCreate* vdp_presentation_queue_create;
extern VdpPresentationQueueDestroy* vdp_presentation_queue_destroy;
extern VdpPresentationQueueGetTime* vdp_presentation_queue_get_time;
extern VdpPresentationQueueTargetCreateX11* vdp_presentation_queue_target_create_x11;
extern VdpPresentationQueueQuerySurfaceStatus*
        vdp_presentation_queue_query_surface_status;
extern VdpPresentationQueueDisplay* vdp_presentation_queue_display;
extern VdpPresentationQueueBlockUntilSurfaceIdle*
        vdp_presentation_queue_block_until_surface_idle;


VdpDevice getVDPAUDevice();

void getPlanesFromVDPAU(vdpau_render_state* pRenderState, BitmapPtr pBmpY,
        BitmapPtr pBmpU, BitmapPtr pBmpV);
void getBitmapFromVDPAU(vdpau_render_state* pRenderState, BitmapPtr pBmpDest);
void unlockVDPAUSurface(vdpau_render_state* pRenderState);

}
#endif

