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
#include "VDPAUDecoder.h"
#include "VDPAUHelper.h"
#include "FrameAge.h"

#include "../base/Exception.h"

#include <iostream>

using namespace std;

namespace avg {

VDPAUDecoder::VDPAUDecoder()
    : m_VDPDecoder(VDP_INVALID_HANDLE),
      m_VDPMixer(VDP_INVALID_HANDLE),
      m_PixFmt(PIX_FMT_NONE),
      m_Size(-1,-1)
{
    for (int i = 0; i < N_VIDEO_SURFACES; i++) {
        m_VideoSurfaces[i].m_Surface = VDP_INVALID_HANDLE;
    }
}

VDPAUDecoder::~VDPAUDecoder()
{
    if (m_VDPMixer != VDP_INVALID_HANDLE) {
        vdp_video_mixer_destroy(m_VDPMixer);
    }
    if (m_VDPDecoder != VDP_INVALID_HANDLE) {
        vdp_decoder_destroy(m_VDPDecoder);
    }
    for (int i = 0; i < N_VIDEO_SURFACES; i++) {
        if (m_VideoSurfaces[i].m_Surface != VDP_INVALID_HANDLE) {
            vdp_video_surface_destroy(m_VideoSurfaces[i].m_Surface);
        }
    }
}

bool VDPAUDecoder::isAvailable()
{
    return getVDPAUDevice() != 0;
}

AVCodec* VDPAUDecoder::openCodec(AVCodecContext* pContext)
{
    getVDPAUDevice();

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
    if (!pCodec || !getVDPAUDevice()) {
        pCodec = avcodec_find_decoder(pContext->codec_id);
    } else {
        pContext->get_buffer = VDPAUDecoder::getBuffer;
        pContext->release_buffer = VDPAUDecoder::releaseBuffer;
        pContext->draw_horiz_band = VDPAUDecoder::drawHorizBand;
        pContext->get_format = VDPAUDecoder::getFormat;
        pContext->slice_flags = SLICE_FLAG_CODED_ORDER | SLICE_FLAG_ALLOW_FIELD;
    }
    for (int i = 0; i < N_VIDEO_SURFACES; i++) {
        memset(&m_VideoSurfaces[i].m_RenderState, 0, sizeof(vdpau_render_state));
        m_VideoSurfaces[i].m_RenderState.surface = VDP_INVALID_HANDLE;
        m_VideoSurfaces[i].m_Size = IntPoint(-1,-1);
    }
    return pCodec;
}

FrameAge* VDPAUDecoder::getFrameAge()
{
    return m_pFrameAge;
}

void VDPAUDecoder::setFrameAge(FrameAge* pFrameAge)
{
    m_pFrameAge = pFrameAge;
}

int VDPAUDecoder::getBuffer(AVCodecContext* pContext, AVFrame* pFrame)
{
    VDPAUDecoder* pVDPAUDecoder = (VDPAUDecoder*)pContext->opaque;
    return pVDPAUDecoder->getBufferInternal(pContext, pFrame);
}

int VDPAUDecoder::getFreeSurfaceIndex()
{
    for (int i = 0; i < N_VIDEO_SURFACES; i++) {
        vdpau_render_state* pRenderState = &m_VideoSurfaces[i].m_RenderState;
        if (!(pRenderState->state & FF_VDPAU_STATE_USED_FOR_REFERENCE)) {
            return i;
        }
    }
    AVG_ASSERT(false);
    return -1;
}

int VDPAUDecoder::getBufferInternal(AVCodecContext* pContext, AVFrame* pFrame)
{
    VdpStatus status;
    int surfaceIndex = getFreeSurfaceIndex();
    
    VideoSurface* pVideoSurface = &m_VideoSurfaces[surfaceIndex];
    vdpau_render_state* pRenderState = &pVideoSurface->m_RenderState;
    pFrame->data[0] = (uint8_t*)pRenderState;
    pFrame->type = FF_BUFFER_TYPE_USER;

#if LIBAVFORMAT_VERSION_MAJOR <= 52
    if (pFrame->reference) { //I-P frame
        pFrame->age = pFrameAge->m_IPAge0;
        pFrameAge->m_IPAge0 = pFrameAge->m_IPAge1;
        pFrameAge->m_IPAge1 = 1;
        pFrameAge->m_Age++;
    } else {
        pFrame->age = pFrameAge->m_Age;
        pFrameAge->m_IPAge0++;
        pFrameAge->m_IPAge1++;
        pFrameAge->m_Age = 1;
    }
#endif
    pRenderState->state |= FF_VDPAU_STATE_USED_FOR_REFERENCE;

    if (pVideoSurface->m_Size.x != pContext->width ||
            pVideoSurface->m_Size.y != pContext->height)
    {
        // allocate a new surface, freeing the old one, if any
        if (pVideoSurface->m_Surface != VDP_INVALID_HANDLE) {
            status = vdp_video_surface_destroy(pVideoSurface->m_Surface);
            AVG_ASSERT(status == VDP_STATUS_OK);
        }
        status = vdp_video_surface_create(getVDPAUDevice(), VDP_CHROMA_TYPE_420,
                pContext->width, pContext->height, &pVideoSurface->m_Surface);
        AVG_ASSERT(status == VDP_STATUS_OK);
        pVideoSurface->m_Size.x = pContext->width;
        pVideoSurface->m_Size.y = pContext->height;
        pVideoSurface->m_RenderState.surface = pVideoSurface->m_Surface;
    }
    return 0;
}

// does not release the render structure, that will be unlocked after getting data
void VDPAUDecoder::releaseBuffer(struct AVCodecContext* pContext, AVFrame* pFrame)
{
    pFrame->data[0] = 0;
}


// main rendering routine
void VDPAUDecoder::drawHorizBand(struct AVCodecContext* pContext, const AVFrame* src,
        int offset[4], int y, int type, int height)
{
    VDPAUDecoder* pVDPAUDecoder = (VDPAUDecoder*)pContext->opaque;
    pVDPAUDecoder->render(pContext, src);
}

::PixelFormat VDPAUDecoder::getFormat(AVCodecContext* pContext, const ::PixelFormat* pFmt)
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

void VDPAUDecoder::render(AVCodecContext* pContext, const AVFrame* pFrame)
{
    vdpau_render_state* pRenderState = (vdpau_render_state*)pFrame->data[0];
    VdpStatus status;
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
        if (m_VDPMixer != VDP_INVALID_HANDLE) {
            status = vdp_video_mixer_destroy(m_VDPMixer);
            AVG_ASSERT(status == VDP_STATUS_OK);
            m_VDPMixer = VDP_INVALID_HANDLE;
        }    
        if (m_VDPDecoder != VDP_INVALID_HANDLE) {
            status = vdp_decoder_destroy(m_VDPDecoder);
            AVG_ASSERT(status == VDP_STATUS_OK);
            m_VDPDecoder = VDP_INVALID_HANDLE;
        }
        status = vdp_decoder_create(getVDPAUDevice(), profile, size.x, size.y, 16,
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
        int  numLayers = 0;
        void const* paramValues [] = { &m_Size.x, &m_Size.y, &chroma, &numLayers };

        status = vdp_video_mixer_create(getVDPAUDevice(), 2, features, 4, params, 
                paramValues, &m_VDPMixer);
        AVG_ASSERT(status == VDP_STATUS_OK);
    }

    status = vdp_decoder_render(m_VDPDecoder, pRenderState->surface,
            (VdpPictureInfo const*)&(pRenderState->info),
            pRenderState->bitstream_buffers_used, pRenderState->bitstream_buffers);
    AVG_ASSERT(status == VDP_STATUS_OK);
}

void VDPAUDecoder::unlockSurface(vdpau_render_state* pRenderState)
{
    pRenderState->state &= ~FF_VDPAU_STATE_USED_FOR_REFERENCE;
}

}


