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


#include <iostream>

#include "VDPAU.h"

using std::cerr;
using std::endl;

namespace avg {

#ifdef HAVE_VDPAU
VdpGetProcAddress                        *vdp_get_proc_address;
VdpDeviceDestroy                        *vdp_device_destroy;
VdpVideoSurfaceCreate                    *vdp_video_surface_create;
VdpVideoSurfaceDestroy                    *vdp_video_surface_destroy;
VdpDecoderCreate                        *m_vdp_decoder_create;
VdpDecoderDestroy                        *m_vdp_decoder_destroy;
VdpDecoderRender                        *m_vdp_decoder_render;
VdpOutputSurfaceCreate                    *vdp_output_surface_create;
VdpOutputSurfaceDestroy                    *vdp_output_surface_destroy;
VdpOutputSurfaceGetBitsNative            *vdp_output_surface_get_bits_native;
VdpOutputSurfaceGetParameters            *vdp_output_surface_get_parameters;
VdpVideoSurfaceGetBitsYCbCr                *vdp_video_surface_get_bits_y_cb_cr;
VdpVideoMixerCreate                        *vdp_video_mixer_create;
VdpVideoMixerDestroy                    *vdp_video_mixer_destroy;
VdpVideoMixerRender                        *vdp_video_mixer_render;
VdpPresentationQueueCreate                *vdp_presentation_queue_create;
VdpPresentationQueueDestroy                *vdp_presentation_queue_destroy;
VdpPresentationQueueGetTime                *vdp_presentation_queue_get_time;
VdpPresentationQueueTargetCreateX11        *vdp_presentation_queue_target_create_x11;
VdpPresentationQueueQuerySurfaceStatus    *vdp_presentation_queue_query_surface_status;
VdpPresentationQueueDisplay                *vdp_presentation_queue_display;
VdpPresentationQueueBlockUntilSurfaceIdle
    *vdp_presentation_queue_block_until_surface_idle;
VdpVideoSurfaceGetParameters *vdp_video_surface_get_parameters;


inline bool vdpCheck(VdpStatus status, int line)
{
    if (status != VDP_STATUS_OK) {
        cerr << "VDPerror, status=" << status << " line=" << line << endl;
        return false;
    }
    else return true;
}

VDPAU::VDPAU():
    m_vdpDevice(0),
    m_textureContext(0),
    m_drawable(0),
    m_xPixmap(0),
    m_glxPixmap(0),
    m_pXDisplay(0),
    m_pXVisual(0),
    m_vdpDecoder(0),
    m_vdpMixer(0),
    m_vdpPresentationQueue(0),
    m_pixFmt(0),
    m_width(0),
    m_height(0)
{
}


GLXPixmap VDPAU::getPixmap()
{
    return m_glxPixmap;
}

Display *VDPAU::getDisplay()
{
    return m_pXDisplay;
}

bool VDPAU::init()
{
    if (m_vdpDevice) {
        return true;
    }
    VdpStatus status;

    //m_pXDisplay = glXGetCurrentDisplay();
    m_pXDisplay = XOpenDisplay(0);
    if (!m_pXDisplay) {
        return false;
    }

    status = vdp_device_create_x11(m_pXDisplay, DefaultScreen(m_pXDisplay), &m_vdpDevice,
        &vdp_get_proc_address);
    if (!vdpCheck(status,__LINE__)) {
         return false;
    }
    status = vdp_get_proc_address(m_vdpDevice, VDP_FUNC_ID_DEVICE_DESTROY,
        (void **)&vdp_device_destroy);
    if (!vdpCheck(status,__LINE__)) {
         return false;
    }
    status = vdp_get_proc_address(m_vdpDevice, VDP_FUNC_ID_OUTPUT_SURFACE_CREATE,
        (void **)&vdp_output_surface_create);
    if (!vdpCheck(status,__LINE__)) {
         return false;
    }
    status = vdp_get_proc_address(m_vdpDevice, VDP_FUNC_ID_OUTPUT_SURFACE_DESTROY,
        (void **)&vdp_output_surface_destroy);
    if (!vdpCheck(status,__LINE__)) {
         return false;
    }
    status = vdp_get_proc_address(m_vdpDevice, VDP_FUNC_ID_OUTPUT_SURFACE_GET_BITS_NATIVE,
        (void **)&vdp_output_surface_get_bits_native);
    if (!vdpCheck(status,__LINE__)) {
         return false;
    }
    status = vdp_get_proc_address(m_vdpDevice, VDP_FUNC_ID_VIDEO_SURFACE_CREATE, 
        (void **)&vdp_video_surface_create);
    if (!vdpCheck(status,__LINE__)) {
         return false;
    }
    status = vdp_get_proc_address(m_vdpDevice, VDP_FUNC_ID_VIDEO_SURFACE_DESTROY,
       (void **)&vdp_video_surface_destroy);
    if (!vdpCheck(status,__LINE__)) {
         return false;
    }
    status = vdp_get_proc_address(m_vdpDevice, VDP_FUNC_ID_DECODER_CREATE,
        (void **)&m_vdp_decoder_create);
    if (!vdpCheck(status,__LINE__)) {
         return false;
    }
    status = vdp_get_proc_address(m_vdpDevice, VDP_FUNC_ID_DECODER_DESTROY,
        (void **)&m_vdp_decoder_destroy);
    if (!vdpCheck(status,__LINE__)) {
         return false;
    }
    status = vdp_get_proc_address(m_vdpDevice, VDP_FUNC_ID_DECODER_RENDER,
        (void **)&m_vdp_decoder_render);
    if (!vdpCheck(status,__LINE__)) {
         return false;
    }
    status = vdp_get_proc_address(m_vdpDevice, VDP_FUNC_ID_VIDEO_SURFACE_GET_BITS_Y_CB_CR,
        (void **)&vdp_video_surface_get_bits_y_cb_cr);
    if (!vdpCheck(status,__LINE__)) {
         return false;
    }
    status = vdp_get_proc_address(m_vdpDevice, VDP_FUNC_ID_VIDEO_MIXER_CREATE,
         (void **)&vdp_video_mixer_create);
    if (!vdpCheck(status,__LINE__)) {
         return false;
    }
    status = vdp_get_proc_address(m_vdpDevice, VDP_FUNC_ID_VIDEO_MIXER_DESTROY,
        (void **)&vdp_video_mixer_destroy);
    if (!vdpCheck(status,__LINE__)) {
         return false;
    }
    status = vdp_get_proc_address(m_vdpDevice, VDP_FUNC_ID_VIDEO_MIXER_RENDER,
        (void **)&vdp_video_mixer_render);
    if (!vdpCheck(status,__LINE__)) {
         return false;
    }
    status = vdp_get_proc_address(m_vdpDevice, VDP_FUNC_ID_PRESENTATION_QUEUE_CREATE, 
                                                (void **)&vdp_presentation_queue_create);
    if (!vdpCheck(status,__LINE__)) {
         return false;
    }
    status = vdp_get_proc_address(m_vdpDevice, VDP_FUNC_ID_PRESENTATION_QUEUE_DESTROY, 
                                                (void **)&vdp_presentation_queue_destroy);
    if (!vdpCheck(status,__LINE__)) {
         return false;
    }
    status = vdp_get_proc_address(m_vdpDevice, 
        VDP_FUNC_ID_PRESENTATION_QUEUE_TARGET_CREATE_X11,
        (void **)&vdp_presentation_queue_target_create_x11);
    if (!vdpCheck(status,__LINE__)) {
         return false;
    }
    status = vdp_get_proc_address(m_vdpDevice, 
        VDP_FUNC_ID_PRESENTATION_QUEUE_QUERY_SURFACE_STATUS, 
        (void **)&vdp_presentation_queue_query_surface_status);
    if (!vdpCheck(status,__LINE__)) {
         return false;
    }
    status = vdp_get_proc_address(m_vdpDevice, VDP_FUNC_ID_PRESENTATION_QUEUE_DISPLAY,
       (void **)&vdp_presentation_queue_display);
    if (!vdpCheck(status,__LINE__)) {
         return false;
    }
    status = vdp_get_proc_address(m_vdpDevice, VDP_FUNC_ID_PRESENTATION_QUEUE_GET_TIME,
        (void **)&vdp_presentation_queue_get_time);
    if (!vdpCheck(status,__LINE__)) {
         return false;
    }
    status = vdp_get_proc_address(m_vdpDevice,
        VDP_FUNC_ID_PRESENTATION_QUEUE_BLOCK_UNTIL_SURFACE_IDLE, 
        (void **)&vdp_presentation_queue_block_until_surface_idle);
    if (!vdpCheck(status,__LINE__)) {
         return false;
    }
    status = vdp_get_proc_address(m_vdpDevice, VDP_FUNC_ID_VIDEO_SURFACE_GET_PARAMETERS,
       (void **)&vdp_video_surface_get_parameters);
    if (!vdpCheck(status,__LINE__)) {
         return false;
    }
    status = vdp_get_proc_address(m_vdpDevice, VDP_FUNC_ID_OUTPUT_SURFACE_GET_PARAMETERS,
        (void **)&vdp_output_surface_get_parameters);
    if (!vdpCheck(status,__LINE__)) {
         return false;
    }
    
    for (int i = 0; i < N_VIDEO_SURFACES; i++) {
        memset(&m_videoSurfaces[i].m_render, 0, sizeof(vdpau_render_state));
        m_videoSurfaces[i].m_render.surface = VDP_INVALID_HANDLE;
        m_videoSurfaces[i].m_surface = VDP_INVALID_HANDLE;
        m_videoSurfaces[i].m_width = m_videoSurfaces[i].m_height = -1;
    }
/*   
    GLXContext context = glXGetCurrentContext();
    if (!context) {
        cerr << "glXGetCurrentContext Error" << endl;
        return false;
    }
    GLXFBConfig *fbconfig;
    int num_configs;    
    m_drawable = glXGetCurrentDrawable();

    unsigned int fbconfig_id = 0;
    glXQueryDrawable(m_pXDisplay, m_drawable, GLX_FBCONFIG_ID, &fbconfig_id);

    int fbc_attribs[] = { GLX_FBCONFIG_ID, fbconfig_id, None };
    int nelements = 0;
    fbconfig = glXChooseFBConfig(m_pXDisplay, 0, fbc_attribs, &nelements);
    m_textureContext = glXCreateNewContext(m_pXDisplay, *fbconfig,
        GLX_RGBA_TYPE, context, 1);

    XVisualInfo *xv = glXGetVisualFromFBConfig(m_pXDisplay, fbconfig[0]);
    if (!xv) {
        cerr << "glXGetVisualFromFBConfig error" << endl;
        return false;
    }

    m_xPixmap = XCreatePixmap(m_pXDisplay, m_drawable, 1920, 1080, 24);
    int pixmapAttribs[] = {
        GLX_TEXTURE_TARGET_EXT, GLX_TEXTURE_2D_EXT,
        GLX_TEXTURE_FORMAT_EXT, GLX_TEXTURE_FORMAT_RGBA_EXT,
        None
    };
    //target pixmap
    int attribs[] = {
                GLX_RENDER_TYPE,  GLX_RGBA_BIT,
                GLX_RED_SIZE,     8,
                GLX_GREEN_SIZE,   8,
                GLX_BLUE_SIZE,    8,
                GLX_ALPHA_SIZE,   8,
                GLX_DOUBLEBUFFER, 1,
                GLX_BUFFER_SIZE,  32,
                GLX_DEPTH_SIZE,   8,
                GLX_DRAWABLE_TYPE, GLX_PIXMAP_BIT,
                GLX_BIND_TO_TEXTURE_RGBA_EXT, 1,
                GLX_Y_INVERTED_EXT, 1,
                GLX_X_RENDERABLE, 1,    
                //GLX_DEPTH_SIZE, 24,
                //GLX_PBUFFER_WIDTH, 16,
                //GLX_PBUFFER_HEIGHT, 16,
                //GLX_SAMPLE_BUFFERS_ARB, 1,
                //GLX_SAMPLES_ARB, 4
            None };

    fbconfig = glXChooseFBConfig(m_pXDisplay, 0, attribs, &num_configs);
    m_glxPixmap = glXCreatePixmap(m_pXDisplay, *fbconfig, m_xPixmap, pixmapAttribs);
    
    m_pixFmt = -1;
    m_width = -1;
    m_height = -1;

    VdpPresentationQueueTarget vdp_presentation_queue_target;
    status = vdp_presentation_queue_target_create_x11(m_vdpDevice, m_xPixmap,
        &vdp_presentation_queue_target);
    if (!vdpCheck(status,__LINE__)) {
         return false;
    }
    status = vdp_presentation_queue_create(m_vdpDevice, vdp_presentation_queue_target,
       &m_vdpPresentationQueue);
    if (!vdpCheck(status,__LINE__)) {
         return false;
    }
*/
    return true;
}

AVCodec *VDPAU::openCodec(AVCodecContext *enc)
{
    AVCodec *codec = NULL;
    if (enc->codec_id == CODEC_ID_MPEG2VIDEO || enc->codec_id == CODEC_ID_H264) {
        if (enc->codec_id == CODEC_ID_MPEG2VIDEO)
           codec = avcodec_find_decoder_by_name("mpegvideo_vdpau");
        if (enc->codec_id == CODEC_ID_H264)
            codec = avcodec_find_decoder_by_name("h264_vdpau");
        if (!codec) {
            perror("unable to find vdpau decoder");
            codec = avcodec_find_decoder(enc->codec_id);
        } else {
             enc->get_buffer = VDPAU::getBuffer;
             enc->release_buffer = VDPAU::releaseBuffer;
             enc->draw_horiz_band = VDPAU::drawHorizBand;
             enc->slice_flags = SLICE_FLAG_CODED_ORDER | SLICE_FLAG_ALLOW_FIELD;
        }
    } else {
        codec = avcodec_find_decoder(enc->codec_id);
    }
    return codec;
}

int VDPAU::getBuffer(struct AVCodecContext *c, AVFrame *frame)
{
    AVCCOpaque *opaque = (AVCCOpaque *)c->opaque;
    FrameAge *frameAge = opaque->getFrameAge();
    VDPAU *vdpau = opaque->getVDPAU();
    return vdpau->getBufferInternal(c, frame, frameAge);
}

int VDPAU::getBufferInternal(AVCodecContext *c, AVFrame *frame, FrameAge *frameAge)
{
    VdpStatus    status;
    for (int i = 0; i < N_VIDEO_SURFACES; i++) {
        struct vdpau_render_state *render = &m_videoSurfaces[i].m_render;
        if (!(render->state & FF_VDPAU_STATE_USED_FOR_REFERENCE)) {
            frame->data[0] = (uint8_t *)render;
            frame->type = FF_BUFFER_TYPE_USER;

            if (frame->reference) { //I-P frame
                frame->age = frameAge->m_IPAge0;
                frameAge->m_IPAge0 = frameAge->m_IPAge1;
                frameAge->m_IPAge1 = 1;
                frameAge->m_age++;
            } else {
                frame->age = frameAge->m_age;
                frameAge->m_IPAge0++;
                frameAge->m_IPAge1++;
                frameAge->m_age = 1;
            }
            render->state |= FF_VDPAU_STATE_USED_FOR_REFERENCE;

            if (m_videoSurfaces[i].m_width != c->width || 
                  m_videoSurfaces[i].m_height != c->height) {
                // allocate a new surface, freeing the old one, if any
                if (m_videoSurfaces[i].m_surface != VDP_INVALID_HANDLE) {
                    status = vdp_video_surface_destroy(m_videoSurfaces[i].m_surface);
                    if (!vdpCheck(status,__LINE__)) {
                        return 0;
                    }
                }
                status = vdp_video_surface_create(m_vdpDevice, VDP_CHROMA_TYPE_420,
                    c->width, c->height, &m_videoSurfaces[i].m_surface);
                if (!vdpCheck(status,__LINE__)) {
                    return 0;
                }
                m_videoSurfaces[i].m_width = c->width;
                m_videoSurfaces[i].m_height = c->height;
                m_videoSurfaces[i].m_render.surface = m_videoSurfaces[i].m_surface;
            }
            return 0;
        }
    }
    return 1;
}

// release the render structure
void VDPAU::releaseBuffer(struct AVCodecContext *c, AVFrame *frame)
{
    vdpau_render_state *render = (vdpau_render_state *)frame->data[0];
    render->state &= ~FF_VDPAU_STATE_USED_FOR_REFERENCE;
    frame->data[0] = NULL;
}

//main rendering routine
void VDPAU::drawHorizBand(struct AVCodecContext *c, const AVFrame *src, int offset[4],
   int y, int type, int height)
{
    AVCCOpaque *opaque = (AVCCOpaque *)c->opaque;
    VDPAU *vdpau = opaque->getVDPAU();
    vdpau->render(c, src);
}

void VDPAU::render(AVCodecContext *context,const AVFrame *frame)
{
    struct vdpau_render_state    *render = (struct vdpau_render_state *)frame->data[0];
    VdpStatus  status;
    int  width = context->width;
    int  height = context->height;

    if (context->pix_fmt != m_pixFmt || width != m_width || height != m_height) {
        VdpDecoderProfile profile = 0;
        switch(context->pix_fmt) {
            case PIX_FMT_VDPAU_MPEG1: profile = VDP_DECODER_PROFILE_MPEG1; break;
            case PIX_FMT_VDPAU_MPEG2: profile = VDP_DECODER_PROFILE_MPEG2_MAIN; break;
            case PIX_FMT_VDPAU_H264:  profile = VDP_DECODER_PROFILE_H264_HIGH; break;
            default:
                cerr << "unsupported pixel format:" << context->pix_fmt << endl;
        }
        if (m_vdpMixer) {
            status = vdp_video_mixer_destroy(m_vdpMixer);
            if (!vdpCheck(status,__LINE__))  {
                return;
            }
            m_vdpMixer = VDP_INVALID_HANDLE;
        }    
        if (m_vdpDecoder) {
            status = m_vdp_decoder_destroy(m_vdpDecoder);
            if (!vdpCheck(status,__LINE__)) {
                return;
            }
            m_vdpDecoder = VDP_INVALID_HANDLE;
        }
        status = m_vdp_decoder_create(m_vdpDevice, profile, width, height, 16, 
            &m_vdpDecoder);
        if (!vdpCheck(status,__LINE__)) {
            return;
        }
        
        m_pixFmt = context->pix_fmt;
        m_width = width;
        m_height = height;

        VdpVideoMixerFeature     features[] = {
            VDP_VIDEO_MIXER_FEATURE_DEINTERLACE_TEMPORAL,
            VDP_VIDEO_MIXER_FEATURE_DEINTERLACE_TEMPORAL_SPATIAL,
        };
        VdpVideoMixerParameter    params[] = { 
             VDP_VIDEO_MIXER_PARAMETER_VIDEO_SURFACE_WIDTH,
             VDP_VIDEO_MIXER_PARAMETER_VIDEO_SURFACE_HEIGHT,
             VDP_VIDEO_MIXER_PARAMETER_CHROMA_TYPE,
             VDP_VIDEO_MIXER_PARAMETER_LAYERS
        };
        VdpChromaType chroma = VDP_CHROMA_TYPE_420;
        int  num_layers = 0;
        void const *paramValues [] = { &width, &height, &chroma, &num_layers };

        status = vdp_video_mixer_create(m_vdpDevice, 2, features, 4, params, paramValues,
           &m_vdpMixer);
        if (!vdpCheck(status,__LINE__)) {
            return;
        }
    }

    status = m_vdp_decoder_render(m_vdpDecoder, render->surface, 
        (VdpPictureInfo const *)&(render->info),render->bitstream_buffers_used,
        render->bitstream_buffers);
    if (!vdpCheck(status,__LINE__)) {
        return;
    }
}

bool VDPAU::videoToPresentationQueue(OutputSurface &surface,int field)
{
    VdpStatus    status;
    VdpTime time;

    int width = surface.m_width;
    int height = surface.m_height;
    status = vdp_output_surface_create(m_vdpDevice, VDP_RGBA_FORMAT_B8G8R8A8,
       width, height, &surface.m_outputSurface);
    if (!vdpCheck(status,__LINE__)) {
        return false;
    }

    VdpOutputSurface vdpOutputSurface = surface.m_outputSurface;
    surface.m_pVDPAU = this;

    VdpRect destVideoRect = { 0, 0, width, height };

    VdpVideoMixerPictureStructure structure = (field == 0 ?
        VDP_VIDEO_MIXER_PICTURE_STRUCTURE_TOP_FIELD :
        VDP_VIDEO_MIXER_PICTURE_STRUCTURE_BOTTOM_FIELD);
    structure = VDP_VIDEO_MIXER_PICTURE_STRUCTURE_BOTTOM_FIELD;

    status = vdp_presentation_queue_block_until_surface_idle(m_vdpPresentationQueue,
        vdpOutputSurface,&time);
    if (!vdpCheck(status,__LINE__)) {
        return false;
    }

    status = vdp_video_mixer_render(m_vdpMixer, VDP_INVALID_HANDLE, NULL, structure, 0,
    0, surface.m_videoSurface, 0, 0, 0, vdpOutputSurface, 0, &destVideoRect, 0, 0);
    if (!vdpCheck(status,__LINE__)) {
        return false;
    }
    VdpTime        display_time = 0;
    /*
    VdpTime vdp_time;
    status = vdp_presentation_queue_get_time(m_vdpPresentationQueue, &vdp_time);
    if (!vdpCheck(status,__LINE__)) {
        return false;
    }
    display_time = vdp_time + (22000000*1);
    */
    status = vdp_presentation_queue_display(m_vdpPresentationQueue, vdpOutputSurface,
       0, 0, display_time);
    status = vdp_presentation_queue_display(m_vdpPresentationQueue, 
        vdpOutputSurface, 0, 0, 0);
    if (!vdpCheck(status,__LINE__)) {
        return false;
    }
    VdpPresentationQueueStatus queueStatus;
    status = vdp_presentation_queue_query_surface_status(m_vdpPresentationQueue,
        vdpOutputSurface, &queueStatus, &time);
    if (!vdpCheck(status,__LINE__)) {
        return false;
    }
    return true;
}

#else        // HAVE_VDPAU

VDPAU::VDPAU()
{
}

AVCodec *VDPAU::openCodec(AVCodecContext *enc)
{
    return NULL;
}

bool VDPAU::init()
{
    return false;
}
#endif        // HAVE_VDPAU

}


