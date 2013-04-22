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

#ifndef _OpenMaxDecoder_H_
#define _OpenMaxDecoder_H_

#include <boost/function.hpp>

#include "WrapFFMpeg.h"
#include "VideoDecoder.h"

extern "C" {
    #include <IL/OMX_Core.h>
    #include <IL/OMX_Broadcom.h>
    #include <ilclient.h>
}

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include "GLES/gl.h"

namespace avg {

typedef boost::function<void (COMPONENT_T * comp) > FillCBFunctor;
class AVG_API OpenMaxDecoder
{
public:
    OpenMaxDecoder();
    ~OpenMaxDecoder();
    AVCodec* openCodec(AVCodecContext* pCodec);

    FrameAvailableCode renderToTexture(GLTexturePtr pTextures[4], float timeWanted) {};
    FrameAvailableCode renderToTexture(AVPacket* pPacket, GLTexturePtr pTextures[4],
            float timeWanted);
    void registerTexture(GLTexturePtr pTexture);
    void decodePacket(AVPacket* pPacket);

    void fill_buffer_done_cb(COMPONENT_T *comp);
private:
    void initComponents();
    void connectComponents();
    void cleanUp();

    void checkOMXError(OMX_ERRORTYPE err);
    void checkILError(int err);

    ILCLIENT_T *m_pILClient;
    COMPONENT_T *m_pVideoScheduler;
    COMPONENT_T *m_pVideoDecoder;
    COMPONENT_T *m_pEglRenderer;
    //COMPONENT_T *m_pClock;

    TUNNEL_T m_Tunnels[3];

    OMX_VIDEO_PARAM_PORTFORMATTYPE m_videoFormat;
    OMX_VIDEO_PORTDEFINITIONTYPE m_videoPortDefinition;
    OMX_BUFFERHEADERTYPE *m_pDecoderInBuffer;
    OMX_BUFFERHEADERTYPE *m_pEglBuffer;
    OMX_TIME_CONFIG_CLOCKSTATETYPE m_clockState;
    bool m_bPortSettingsChanged;
    bool m_bSetupTextures;
    float m_fLastFrameTime;
    EGLImageKHR m_pEglImage;
    FillCBFunctor m_cbFunctor;
};

}

#endif
