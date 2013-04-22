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

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <string>

#include <bcm_host.h>

#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../graphics/GLTexture.h"

#include "OpenMaxDecoder.h"
#include "WrapFFMpeg.h"


namespace avg {

OpenMaxDecoder::OpenMaxDecoder():
    m_pILClient(0),
    m_pVideoScheduler(0),
    m_pVideoDecoder(0),
    m_pEglRenderer(0),
    //m_pClock(0),
    m_pDecoderInBuffer(0),
    m_pEglBuffer(0),
    m_bPortSettingsChanged(false),
    m_bSetupTextures(true),
    m_fLastFrameTime(-1.0)
{
    AVG_LOG_CONFIG("Try to init OpenMaxVideo");
    AVG_LOG_DEBUG("Memset tunnels");
    initComponents();
    AVG_LOG_CONFIG("Using OpenMaxVideo acceleration");
}

OpenMaxDecoder::~OpenMaxDecoder()
{
    cleanUp();
}

void fill_buffer_callback(void *userData, COMPONENT_T *comp)
{
    FillCBFunctor* fun = static_cast<FillCBFunctor*>(userData);
    (*fun)(comp);
}

void OpenMaxDecoder::initComponents()
{
    AVG_LOG_CONFIG("INIT Compontents");
    memset(m_Tunnels, 0, sizeof(m_Tunnels));
    bcm_host_init();
    m_pILClient = ilclient_init();
    AVG_ASSERT(m_pILClient);
    checkOMXError(OMX_Init());
    m_cbFunctor = boost::bind(&OpenMaxDecoder::fill_buffer_done_cb, this, _1);
    ilclient_set_fill_buffer_done_callback(m_pILClient, &fill_buffer_callback, &m_cbFunctor);
    checkILError(ilclient_create_component(m_pILClient, &m_pVideoDecoder, "video_decode",
            (ILCLIENT_CREATE_FLAGS_T)(ILCLIENT_DISABLE_ALL_PORTS |
            ILCLIENT_ENABLE_INPUT_BUFFERS)));
    checkILError(ilclient_create_component(m_pILClient, &m_pVideoScheduler,
            "video_scheduler", ILCLIENT_DISABLE_ALL_PORTS));
    checkILError(ilclient_create_component(m_pILClient, &m_pEglRenderer, "egl_render",
            (ILCLIENT_CREATE_FLAGS_T)(ILCLIENT_DISABLE_ALL_PORTS |
            ILCLIENT_ENABLE_OUTPUT_BUFFERS)));
    /*checkILError(ilclient_create_component(m_pILClient, &m_pClock, "clock",
            (ILCLIENT_CREATE_FLAGS_T)(ILCLIENT_DISABLE_ALL_PORTS)));
    */

    /*
    AVG_LOG_CONFIG("Allocate Space for clock");
    memset(&m_clockState, 0, sizeof(m_clockState));
    m_clockState.nSize = sizeof(m_clockState);
    m_clockState.nVersion.nVersion = OMX_VERSION;
    m_clockState.eState = OMX_TIME_ClockStateWaitingForStartTime;
    m_clockState.nWaitMask = 1;
    AVG_ASSERT(m_pClock);
    checkOMXError(OMX_SetParameter(ILC_GET_HANDLE(m_pClock),
            OMX_IndexConfigTimeClockState, &m_clockState));
    AVG_LOG_CONFIG("Done allocating Space for clock");
    */

    AVG_LOG_CONFIG("Finished INIT Compontents");
   }

void OpenMaxDecoder::connectComponents()
{
    AVG_LOG_CONFIG("Connect Compontents");
    set_tunnel(m_Tunnels, m_pVideoDecoder, 131, m_pVideoScheduler, 10);
    set_tunnel(m_Tunnels+1, m_pVideoScheduler, 11, m_pEglRenderer, 220);
    /*set_tunnel(m_Tunnels+2, m_pClock, 80, m_pVideoScheduler, 12);*/
    /*ilclient_change_component_state(m_pClock, OMX_StateExecuting);*/
    ilclient_change_component_state(m_pVideoDecoder, OMX_StateIdle);
    AVG_LOG_CONFIG("Connected Compontents");
}

void OpenMaxDecoder::cleanUp()
{
    if(m_pILClient){
        ilclient_destroy(m_pILClient);
    }
}

void OpenMaxDecoder::checkOMXError(OMX_ERRORTYPE err)
{
    if ( err != OMX_ErrorNone ){
        AVG_LOG_ERROR("OpenMax Error Code: 0x" << hex << err);
        throw Exception(AVG_ERR_VIDEO_INIT_FAILED, "Couldn't initialize OpenMax");
    }
}

void OpenMaxDecoder::checkILError(int err)
{
    if ( err != 0 ){
        std::string msg("Error with ILClient, Error Code: ");
        msg += boost::lexical_cast<std::string>(err);
        throw Exception(AVG_ERR_VIDEO_INIT_FAILED, msg);
    }
}

void OpenMaxDecoder::registerTexture(GLTexturePtr pTexture)
{
    EGLint imageAttributes[] = {
            EGL_GL_TEXTURE_LEVEL_KHR, 0, // mip map level to reference
            EGL_IMAGE_PRESERVED_KHR, EGL_TRUE,
            EGL_NONE
    };
    EGLDisplay eglDisplay = eglGetCurrentDisplay();
    ::EGLContext eglContext = eglGetCurrentContext();
    AVG_ASSERT(eglDisplay != EGL_NO_DISPLAY);
    AVG_ASSERT(eglContext != EGL_NO_CONTEXT);
    m_pEglImage = eglCreateImageKHR(
            eglDisplay,
            eglContext,
            EGL_GL_TEXTURE_2D_KHR,
            reinterpret_cast<EGLClientBuffer>(pTexture->getID()),
            imageAttributes);
    AVG_ASSERT(m_pEglImage != EGL_NO_IMAGE_KHR);
}

void OpenMaxDecoder::fill_buffer_done_cb(COMPONENT_T *comp)
{
  checkOMXError(OMX_FillThisBuffer(ilclient_get_handle(m_pEglRenderer), m_pEglBuffer));
}

void OpenMaxDecoder::decodePacket(AVPacket* pPacket)
{
    if(pPacket) {
        AVG_ASSERT(pPacket->data);
        if(!m_bPortSettingsChanged &&
           ((ilclient_wait_for_event(m_pVideoDecoder, OMX_EventPortSettingsChanged, 131, 0, 0, 1,
                   ILCLIENT_EVENT_ERROR | ILCLIENT_PARAMETER_CHANGED, 10000) == 0) ||
            (ilclient_remove_event(m_pVideoDecoder, OMX_EventPortSettingsChanged, 131, 0, 0, 1) == 0)))
        {
            AVG_LOG_CONFIG("PORT SETTINGS CHANGE");
            m_bPortSettingsChanged = true;
            //Setup decoder->scheduler tunnel
            checkILError(ilclient_setup_tunnel(m_Tunnels, 0, 0));
            checkILError(ilclient_change_component_state(m_pVideoScheduler, OMX_StateExecuting));
            AVG_LOG_CONFIG("EXECUTE VIDEO SCHEDULER");
            //Setup scheduler->render tunnel
            checkILError(ilclient_setup_tunnel(m_Tunnels+1, 0, 1000));
            checkILError(ilclient_change_component_state(m_pEglRenderer, OMX_StateIdle));
            checkOMXError(OMX_SendCommand(ILC_GET_HANDLE(m_pEglRenderer), OMX_CommandPortEnable, 221, NULL));
            checkOMXError(OMX_UseEGLImage(ILC_GET_HANDLE(m_pEglRenderer), &m_pEglBuffer, 221, NULL, m_pEglImage));
            checkILError(ilclient_change_component_state(m_pEglRenderer, OMX_StateExecuting));
            checkOMXError(OMX_FillThisBuffer(ILC_GET_HANDLE(m_pEglRenderer), m_pEglBuffer));
            AVG_LOG_CONFIG("EXECUTE VIDEO RENDER");
            AVG_LOG_CONFIG("PORT SETTINGS CHANGED");
        }
        unsigned int pSize = pPacket->size;
        unsigned int bytesRead = 0;
        while(pSize != 0){
            m_pDecoderInBuffer = ilclient_get_input_buffer(m_pVideoDecoder, 130, 1);
            AVG_ASSERT(m_pDecoderInBuffer);

            m_pDecoderInBuffer->nFilledLen = (pSize > m_pDecoderInBuffer->nAllocLen) ?
                    m_pDecoderInBuffer->nAllocLen : pSize;
            memcpy(m_pDecoderInBuffer->pBuffer, pPacket->data+bytesRead,
                    m_pDecoderInBuffer->nFilledLen);
            pSize -= m_pDecoderInBuffer->nFilledLen;
            bytesRead += m_pDecoderInBuffer->nFilledLen;
            if(pSize == 0){
                m_pDecoderInBuffer->nFlags = OMX_BUFFERFLAG_ENDOFFRAME;
            }else{
                m_pDecoderInBuffer->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN;
            }
                m_pDecoderInBuffer->nOffset = 0;
                checkOMXError(OMX_EmptyThisBuffer(ILC_GET_HANDLE(m_pVideoDecoder),
                        m_pDecoderInBuffer));
        }
    }
}

FrameAvailableCode OpenMaxDecoder::renderToTexture(AVPacket *pPacket,
        GLTexturePtr pTextures[4], float timeWanted)
{
    decodePacket(pPacket);
    return FA_NEW_FRAME;
}

AVCodec* OpenMaxDecoder::openCodec(AVCodecContext* pContext)
{
    AVG_LOG_CONFIG("Open Codec");
    AVCodec* pCodec = avcodec_find_decoder(pContext->codec_id);
    AVG_ASSERT(pCodec);

    connectComponents();
    //TODO: Set format according to codec info
    AVG_LOG_CONFIG("MEMSET VIDEO CONFIG");
    memset(&m_videoFormat, 0, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
    m_videoFormat.nSize = sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE);
    m_videoFormat.nVersion.nVersion = OMX_VERSION;
    m_videoFormat.nPortIndex = 130;
    m_videoFormat.eCompressionFormat = OMX_VIDEO_CodingAVC;
    AVG_LOG_CONFIG("SETUP VIDEO DONE");

    checkOMXError(OMX_SetParameter(ILC_GET_HANDLE(m_pVideoDecoder),
            OMX_IndexParamVideoPortFormat, &m_videoFormat));
    checkILError(ilclient_enable_port_buffers(m_pVideoDecoder, 130, NULL, NULL, NULL));
    checkILError(ilclient_change_component_state(m_pVideoDecoder, OMX_StateExecuting));
    AVG_LOG_CONFIG("Opened Codec");
    return pCodec;
}

}
