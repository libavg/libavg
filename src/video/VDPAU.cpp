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
#include "VDPAU.h"
#include "FrameAge.h"
#include "AVCCOpaque.h"
#include "../base/Exception.h"
#include <iostream>

using namespace std;

namespace avg {

VdpGetProcAddress* vdp_get_proc_address;
VdpDeviceDestroy* vdp_device_destroy;
VdpVideoSurfaceCreate* vdp_video_surface_create;
VdpVideoSurfaceDestroy* vdp_video_surface_destroy;
VdpDecoderCreate* vdp_decoder_create;
VdpDecoderDestroy* vdp_decoder_destroy;
VdpDecoderRender* vdp_decoder_render;
VdpOutputSurfaceCreate* vdp_output_surface_create;
VdpOutputSurfaceDestroy* vdp_output_surface_destroy;
VdpOutputSurfaceGetBitsNative* vdp_output_surface_get_bits_native;
VdpOutputSurfaceGetParameters* vdp_output_surface_get_parameters;
VdpVideoSurfaceGetBitsYCbCr* vdp_video_surface_get_bits_y_cb_cr;
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
VdpVideoSurfaceGetParameters* vdp_video_surface_get_parameters;


VdpDevice VDPAU::s_VDPDevice = 0;
Display* VDPAU::s_pXDisplay = 0;

VDPAU::VDPAU()
    : m_PixFmt(PIX_FMT_NONE),
      m_VDPDecoder(0),
      m_VDPMixer(0),
      m_Size(-1,-1)
{
    for (int i = 0; i < N_VIDEO_SURFACES; i++) {
        m_VideoSurfaces[i].m_Surface = VDP_INVALID_HANDLE;
    }
}

VDPAU::~VDPAU()
{
    VdpStatus status;
    if (m_VDPMixer) {
        status = vdp_video_mixer_destroy(m_VDPMixer);
        AVG_ASSERT(status == VDP_STATUS_OK);
        m_VDPMixer = VDP_INVALID_HANDLE;
    }
    if (m_VDPDecoder) {
        status = vdp_decoder_destroy(m_VDPDecoder);
        AVG_ASSERT(status == VDP_STATUS_OK);
        m_VDPDecoder = VDP_INVALID_HANDLE;
    }
    for (int i = 0; i < N_VIDEO_SURFACES; i++) {
        if (m_VideoSurfaces[i].m_Surface != VDP_INVALID_HANDLE) {
            status = vdp_video_surface_destroy(m_VideoSurfaces[i].m_Surface);
            AVG_ASSERT(status == VDP_STATUS_OK);
            m_VideoSurfaces[i].m_RenderState.surface = VDP_INVALID_HANDLE;
            m_VideoSurfaces[i].m_Surface = VDP_INVALID_HANDLE;
        }
    }
}

void VDPAU::safeGetProcAddress(VdpFuncId functionId, void** functionPointer)
{
    VdpStatus status;
    status = vdp_get_proc_address(s_VDPDevice, functionId, functionPointer);
    AVG_ASSERT(status == VDP_STATUS_OK);
}

Display* VDPAU::getDisplay()
{
    return s_pXDisplay;
}

void VDPAU::init()
{
    if (!s_VDPDevice) {
        VdpStatus status;

        //s_pXDisplay = glXGetCurrentDisplay();
        s_pXDisplay = XOpenDisplay(0);
        AVG_ASSERT(s_pXDisplay);

        status = vdp_device_create_x11(s_pXDisplay, DefaultScreen(s_pXDisplay), &s_VDPDevice,
            &vdp_get_proc_address);
        AVG_ASSERT(status == VDP_STATUS_OK);

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
    }

    for (int i = 0; i < N_VIDEO_SURFACES; i++) {
        memset(&m_VideoSurfaces[i].m_RenderState, 0, sizeof(vdpau_render_state));
        m_VideoSurfaces[i].m_RenderState.surface = VDP_INVALID_HANDLE;
        m_VideoSurfaces[i].m_Size = IntPoint(-1,-1);
    }
}

AVCodec* VDPAU::openCodec(AVCodecContext* pContext)
{
    AVCodec* pCodec = 0;

    switch (pContext->codec_id) {
        case CODEC_ID_MPEG1VIDEO:
            pCodec = avcodec_find_decoder_by_name("mpeg1video_vdpau");
            pCodec->id = CODEC_ID_MPEG1VIDEO;
            break;
        case CODEC_ID_MPEG2VIDEO:
            pCodec = avcodec_find_decoder_by_name("mpegvideo_vdpau");
            break;
        case CODEC_ID_H264:
            pCodec = avcodec_find_decoder_by_name("h264_vdpau");
            break;
        case CODEC_ID_WMV3:
            pCodec = avcodec_find_decoder_by_name("wmv3_vdpau");
            break;
        case CODEC_ID_VC1:
            pCodec = avcodec_find_decoder_by_name("vc1_vdpau");
            break;
        default:
            pCodec = 0;
    }
    if (!pCodec) {
        pCodec = avcodec_find_decoder(pContext->codec_id);
    } else {
        pContext->get_buffer = VDPAU::getBuffer;
        pContext->release_buffer = VDPAU::releaseBuffer;
        pContext->draw_horiz_band = VDPAU::drawHorizBand;
        pContext->get_format = VDPAU::getFormat;
        pContext->slice_flags = SLICE_FLAG_CODED_ORDER | SLICE_FLAG_ALLOW_FIELD;
    }
    return pCodec;
}

int VDPAU::getBuffer(AVCodecContext* pContext, AVFrame* frame)
{
    AVCCOpaque* opaque = (AVCCOpaque*)pContext->opaque;
    FrameAge* frameAge = opaque->getFrameAge();
    VDPAU* vdpau = opaque->getVDPAU();
    return vdpau->getBufferInternal(pContext, frame, frameAge);
}

int VDPAU::getBufferInternal(AVCodecContext* pContext, AVFrame* frame, FrameAge* frameAge)
{
    VdpStatus status;
    for (int i = 0; i < N_VIDEO_SURFACES; i++) {
        struct vdpau_render_state* pRenderState = &m_VideoSurfaces[i].m_RenderState;
        if (!(pRenderState->state & FF_VDPAU_STATE_USED_FOR_REFERENCE)) {
            frame->data[0] = (uint8_t*)pRenderState;
            frame->type = FF_BUFFER_TYPE_USER;

            if (frame->reference) { //I-P frame
                frame->age = frameAge->m_IPAge0;
                frameAge->m_IPAge0 = frameAge->m_IPAge1;
                frameAge->m_IPAge1 = 1;
                frameAge->m_Age++;
            } else {
                frame->age = frameAge->m_Age;
                frameAge->m_IPAge0++;
                frameAge->m_IPAge1++;
                frameAge->m_Age = 1;
            }
            pRenderState->state |= FF_VDPAU_STATE_USED_FOR_REFERENCE;

            if (m_VideoSurfaces[i].m_Size.x != pContext->width ||
                    m_VideoSurfaces[i].m_Size.y != pContext->height)
            {
                // allocate a new surface, freeing the old one, if any
                if (m_VideoSurfaces[i].m_Surface != VDP_INVALID_HANDLE) {
                    status = vdp_video_surface_destroy(m_VideoSurfaces[i].m_Surface);
                    AVG_ASSERT(status == VDP_STATUS_OK);
                }
                status = vdp_video_surface_create(s_VDPDevice, VDP_CHROMA_TYPE_420,
                        pContext->width, pContext->height, &m_VideoSurfaces[i].m_Surface);
                AVG_ASSERT(status == VDP_STATUS_OK);
                m_VideoSurfaces[i].m_Size.x = pContext->width;
                m_VideoSurfaces[i].m_Size.y = pContext->height;
                m_VideoSurfaces[i].m_RenderState.surface = m_VideoSurfaces[i].m_Surface;
            }
            return 0;
        }
    }
    return 1;
}

// does not release the render structure, that will be unlocked after getting data
void VDPAU::releaseBuffer(struct AVCodecContext* pContext, AVFrame* frame)
{
    vdpau_render_state* pRenderState = (vdpau_render_state*)frame->data[0];
    frame->data[0] = 0;
}


//main rendering routine
void VDPAU::drawHorizBand(struct AVCodecContext* pContext, const AVFrame* src,
        int offset[4], int y, int type, int height)
{
    AVCCOpaque* pOpaque = (AVCCOpaque*)pContext->opaque;
    VDPAU* pVDPAU = pOpaque->getVDPAU();
    pVDPAU->render(pContext, src);
}

::PixelFormat VDPAU::getFormat(AVCodecContext* pContext, const ::PixelFormat* pFmt)
{
    switch (pContext->codec_id) {
        case CODEC_ID_H264:
            return PIX_FMT_VDPAU_H264;
        case CODEC_ID_MPEG1VIDEO:
            return PIX_FMT_VDPAU_MPEG1;
        case CODEC_ID_MPEG2VIDEO:
            return PIX_FMT_VDPAU_MPEG2;
        case CODEC_ID_WMV3:
            return PIX_FMT_VDPAU_WMV3;
        case CODEC_ID_VC1:
            return PIX_FMT_VDPAU_VC1;
        default:
            return pFmt[0];
    }
}

void VDPAU::render(AVCodecContext* pContext,const AVFrame* frame)
{
    vdpau_render_state* pRenderState = (vdpau_render_state*)frame->data[0];
    VdpStatus  status;
    IntPoint size(pContext->width, pContext->height);

    if (pContext->pix_fmt != m_PixFmt || size != m_Size) {
        VdpDecoderProfile profile = 0;
        switch (pContext->pix_fmt) {
            case PIX_FMT_VDPAU_MPEG1:
                profile = VDP_DECODER_PROFILE_MPEG1;
                break;
            case PIX_FMT_VDPAU_MPEG2:
                profile = VDP_DECODER_PROFILE_MPEG2_MAIN;
                break;
            case PIX_FMT_VDPAU_H264:
                profile = VDP_DECODER_PROFILE_H264_HIGH;
                break;
            case PIX_FMT_VDPAU_WMV3:
                profile = VDP_DECODER_PROFILE_VC1_SIMPLE;
                break;
            case PIX_FMT_VDPAU_VC1:
                profile = VDP_DECODER_PROFILE_VC1_SIMPLE;
                break;
            default:
                AVG_ASSERT(false);
        }
        if (m_VDPMixer) {
            status = vdp_video_mixer_destroy(m_VDPMixer);
            AVG_ASSERT(status == VDP_STATUS_OK);
            m_VDPMixer = VDP_INVALID_HANDLE;
        }    
        if (m_VDPDecoder) {
            status = vdp_decoder_destroy(m_VDPDecoder);
            AVG_ASSERT(status == VDP_STATUS_OK);
            m_VDPDecoder = VDP_INVALID_HANDLE;
        }
        status = vdp_decoder_create(s_VDPDevice, profile, size.x, size.y, 16,
                &m_VDPDecoder);
        AVG_ASSERT(status == VDP_STATUS_OK);
        
        m_PixFmt = pContext->pix_fmt;
        m_Size = size;

        VdpVideoMixerFeature features[] = {
            VDP_VIDEO_MIXER_FEATURE_DEINTERLACE_TEMPORAL,
            VDP_VIDEO_MIXER_FEATURE_DEINTERLACE_TEMPORAL_SPATIAL,
        };
        VdpVideoMixerParameter params[] = { 
             VDP_VIDEO_MIXER_PARAMETER_VIDEO_SURFACE_WIDTH,
             VDP_VIDEO_MIXER_PARAMETER_VIDEO_SURFACE_HEIGHT,
             VDP_VIDEO_MIXER_PARAMETER_CHROMA_TYPE,
             VDP_VIDEO_MIXER_PARAMETER_LAYERS
        };
        VdpChromaType chroma = VDP_CHROMA_TYPE_420;
        int  num_layers = 0;
        void const* paramValues [] = { &m_Size.x, &m_Size.y, &chroma, &num_layers };

        status = vdp_video_mixer_create(s_VDPDevice, 2, features, 4, params, paramValues,
                &m_VDPMixer);
        AVG_ASSERT(status == VDP_STATUS_OK);
    }

    status = vdp_decoder_render(m_VDPDecoder, pRenderState->surface,
            (VdpPictureInfo const*)&(pRenderState->info),
            pRenderState->bitstream_buffers_used, pRenderState->bitstream_buffers);
    AVG_ASSERT(status == VDP_STATUS_OK);
}

void VDPAU::unlockSurface(vdpau_render_state* pRenderState)
{
    pRenderState->state &= ~FF_VDPAU_STATE_USED_FOR_REFERENCE;
}

}


