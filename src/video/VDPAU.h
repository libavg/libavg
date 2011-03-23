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

#include <vdpau/vdpau.h>
#include <libavcodec/vdpau.h>

namespace avg{


struct FrameAge
{
    int    m_age;
    int    m_IPAge0;
    int    m_IPAge1;
    FrameAge():
        m_age(256*256*256*64),
        m_IPAge0(256*256*256*64),
        m_IPAge1(256*256*256*64)
    {
    }
};


#define N_VIDEO_SURFACES            64

struct videoSurface
{
    struct vdpau_render_state    m_render;
    VdpVideoSurface              m_surface;
    int                          m_width;
    int                          m_height;
};

extern VdpVideoSurfaceGetParameters *vdp_video_surface_get_parameters;
extern VdpVideoSurfaceGetBitsYCbCr  *vdp_video_surface_get_bits_y_cb_cr;
extern VdpOutputSurfaceDestroy *vdp_output_surface_destroy;



class VDPAU;

struct OutputSurface
{
    OutputSurface(): m_outputSurface(VDP_INVALID_HANDLE),
        m_videoSurface(VDP_INVALID_HANDLE), m_pVDPAU(0), m_width(0), m_height(0)
    {
    }
    ~OutputSurface()
    {
        if (m_outputSurface) {
            vdp_output_surface_destroy(m_outputSurface);
        }
    }
    void unlock()
    {
        m_pRender->state &= ~FF_VDPAU_STATE_USED_FOR_REFERENCE;
    }
    
    VdpOutputSurface m_outputSurface;
    VdpVideoSurface m_videoSurface;
    vdpau_render_state *m_pRender;
    VDPAU *m_pVDPAU;
    int   m_width;
    int   m_height;
};


class VDPAU{

public:
    VDPAU();
    ~VDPAU()
    {
    }
    AVCodec *openCodec(AVCodecContext *enc);
    bool init();

    bool videoToPresentationQueue(OutputSurface &surface,int field);
    GLXPixmap getPixmap();
    Display *getDisplay();

private:
    static int getBuffer(AVCodecContext *c, AVFrame *frame);
    int getBufferInternal(AVCodecContext *c, AVFrame *frame, FrameAge *age);
    static void releaseBuffer(struct AVCodecContext *c, AVFrame *frame);
    static void drawHorizBand(AVCodecContext *c, const AVFrame *frame, int offset[4],
        int y, int type, int height);
    static ::PixelFormat getFormat(AVCodecContext* pContext, const ::PixelFormat* pFmt);
    void render(AVCodecContext *context,const AVFrame *frame);

    VdpDevice     m_vdpDevice;
    GLXContext    m_textureContext;
    GLXDrawable   m_drawable;
    Pixmap        m_xPixmap;
    GLXPixmap     m_glxPixmap;
    Display       *m_pXDisplay;
    XVisualInfo   *m_pXVisual;
    VdpDecoder    m_vdpDecoder;
    VdpVideoMixer m_vdpMixer;
    VdpPresentationQueue m_vdpPresentationQueue;
    ::PixelFormat  m_pixFmt;
    int    m_width;
    int    m_height;

    videoSurface m_videoSurfaces[N_VIDEO_SURFACES];

};

class AVCCOpaque
{
public:
    AVCCOpaque(VDPAU *vdpau,FrameAge *frameAge = NULL): m_pVDPAU(vdpau),
        m_pFrameAge(frameAge)
    {
    }
    FrameAge *getFrameAge()
    {
        return m_pFrameAge;
    }
    void setFrameAge(FrameAge *age)
    {
       m_pFrameAge = age;
    }
    VDPAU *getVDPAU()
    {
        return m_pVDPAU;
    }
private: 
    VDPAU *m_pVDPAU;
    FrameAge *m_pFrameAge;
};

}
#endif

