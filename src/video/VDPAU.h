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
#ifndef _VDPAU_H_
#define _VDPAU_H_


#include "../avgconfigwrapper.h"
#include "../graphics/GL/glx.h"

#include "WrapFFMpeg.h"
#include "../base/Point.h"

#include <vdpau/vdpau.h>
#include <libavcodec/vdpau.h>


namespace avg {

#define N_VIDEO_SURFACES 64

struct FrameAge;

struct VideoSurface
{
    struct vdpau_render_state m_RenderState;
    VdpVideoSurface m_Surface;
    IntPoint m_Size;
};

extern VdpVideoSurfaceGetParameters* vdp_video_surface_get_parameters;
extern VdpVideoSurfaceGetBitsYCbCr* vdp_video_surface_get_bits_y_cb_cr;
extern VdpOutputSurfaceDestroy* vdp_output_surface_destroy;
extern VdpGetProcAddress* vdp_get_proc_address;
extern VdpDeviceDestroy* vdp_device_destroy;
extern VdpVideoSurfaceCreate* vdp_video_surface_create;
extern VdpVideoSurfaceDestroy* vdp_video_surface_destroy;
extern VdpDecoderCreate* vdp_decoder_create;
extern VdpDecoderDestroy* vdp_decoder_destroy;
extern VdpDecoderRender* vdp_decoder_render;
extern VdpOutputSurfaceCreate* vdp_output_surface_create;
extern VdpOutputSurfaceDestroy* vdp_output_surface_destroy;
extern VdpOutputSurfaceGetBitsNative* vdp_output_surface_get_bits_native;
extern VdpOutputSurfaceGetParameters* vdp_output_surface_get_parameters;
extern VdpVideoSurfaceGetBitsYCbCr* vdp_video_surface_get_bits_y_cb_cr;
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
extern VdpVideoSurfaceGetParameters* vdp_video_surface_get_parameters;


class VDPAU
{
public:
    VDPAU();
    ~VDPAU();
    AVCodec* openCodec(AVCodecContext* pCodec);
    void init();
    Display* getDisplay();
    static void unlockSurface(vdpau_render_state* pRenderState);
    static bool isAvailable();

private:
    static bool staticInit();
    static int getBuffer(AVCodecContext* pContext, AVFrame* pFrame);
    int getFreeSurfaceIndex();
    int getBufferInternal(AVCodecContext* pContext, AVFrame* pFrame, FrameAge* pAge);
    static void releaseBuffer(struct AVCodecContext* pContext, AVFrame* pFrame);
    static void drawHorizBand(AVCodecContext* pContext, const AVFrame* pFrame, 
            int offset[4], int y, int type, int height);
    static ::PixelFormat getFormat(AVCodecContext* pContext, const ::PixelFormat* pFmt);
    void render(AVCodecContext* pContext, const AVFrame* pFrame);
    static void safeGetProcAddress(VdpFuncId function_id, void** function_pointer);

    static VdpDevice s_VDPDevice;
    static Display* s_pXDisplay;
    static bool s_bInitFailed;

    VdpDecoder m_VDPDecoder;
    VdpVideoMixer m_VDPMixer;
    ::PixelFormat  m_PixFmt;
    IntPoint m_Size;
    VideoSurface m_VideoSurfaces[N_VIDEO_SURFACES];

};

}
#endif

