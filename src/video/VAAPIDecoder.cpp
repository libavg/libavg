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
#include "VAAPIDecoder.h"

#include "../base/Exception.h"

#include <iostream>

using namespace std;
    
namespace avg {

std::vector<VAProfile> VAAPIDecoder::s_Profiles;

VAAPIDecoder::VAAPIDecoder()
    : m_Size(-1,-1)
{
}

VAAPIDecoder::~VAAPIDecoder()
{
}

AVCodec* VAAPIDecoder::openCodec(AVCodecContext* pContext)
{
    if (!isAvailable()) {
        return 0;
    }

/*    
    switch (pContext->codec_id) {
        case CODEC_ID_MPEG2VIDEO:
            pCodec = avcodec_find_decoder_by_name("mpeg2_vaapi");
            break;
        case CODEC_ID_MPEG4:
            pCodec = avcodec_find_decoder_by_name("mpeg4_vaapi");
            break;
        case CODEC_ID_H263:
            pCodec = avcodec_find_decoder_by_name("h263_vaapi");
            break;
        case CODEC_ID_H264:
            pCodec = avcodec_find_decoder_by_name("h264_vaapi");
            break;
        case CODEC_ID_WMV3:
            pCodec = avcodec_find_decoder_by_name("wmv3_vaapi");
            break;
        case CODEC_ID_VC1:
            pCodec = avcodec_find_decoder_by_name("vc1_vaapi");
            break;
        default:
            pCodec = 0;
    }
*/
    if (isSupportedCodec(pContext->codec_id)) {
        pContext->get_buffer = VAAPIDecoder::getBuffer;
        pContext->release_buffer = VAAPIDecoder::releaseBuffer;
        pContext->draw_horiz_band = 0;
        pContext->get_format = VAAPIDecoder::getFormat;
        pContext->slice_flags = SLICE_FLAG_CODED_ORDER | SLICE_FLAG_ALLOW_FIELD;
        m_Size = IntPoint(pContext->width, pContext->height);
        
        AVCodec* pCodec = avcodec_find_decoder(pContext->codec_id);
        return pCodec;
    } else {
        return 0;
    }
}

bool VAAPIDecoder::isAvailable()
{
#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(53, 34, 0)
    return getVAAPIDisplay() != 0;
#else
    return false;
#endif
}

int VAAPIDecoder::getBuffer(AVCodecContext* pContext, AVFrame* pFrame)
{
    cerr << "getBuffer" << endl;
    VAAPIDecoder* pVAAPIDecoder = (VAAPIDecoder*)pContext->opaque;
    return pVAAPIDecoder->getBufferInternal(pContext, pFrame);
}

void VAAPIDecoder::releaseBuffer(struct AVCodecContext* pContext, AVFrame* pFrame)
{
    cerr << "releaseBuffer" << endl;
}

AVPixelFormat VAAPIDecoder::getFormat(AVCodecContext* pContext, const AVPixelFormat* pFmt)
{
    cerr << "getFormat" << endl;
    int profile;
    switch (pContext->codec_id) {
        case CODEC_ID_MPEG2VIDEO:
            profile = VAProfileMPEG2Main;
            break;
        case CODEC_ID_MPEG4:
        case CODEC_ID_H263:
            profile = VAProfileMPEG4AdvancedSimple;
            break;
        case CODEC_ID_H264:
            profile = VAProfileH264High;
            break;
        case CODEC_ID_WMV3:
            profile = VAProfileVC1Main;
            break;
        case CODEC_ID_VC1:
            profile = VAProfileVC1Advanced;
            break;
        default:
            profile = -1;
    }
    if (profile != -1) {
        VAAPIDecoder* pVAAPIDecoder = (VAAPIDecoder*)pContext->opaque;
        bool bOK = pVAAPIDecoder->initDecoder((VAProfile)profile, VAEntrypointVLD);
        if (bOK) {
            return PIX_FMT_VAAPI_VLD;
        }
    }
    return pFmt[0];
}
/*
VAAPISurface* VAAPIDecoder::getFreeSurface()
{
    for (unsigned i = 0; i<m_Surfaces.size(); i++) {
        VAAPISurface *pSurface = &m_Surfaces[i];
        if (!pSurface->m_RefCount) {
            return pSurface;
        }
    }
}
*/
int VAAPIDecoder::getBufferInternal(AVCodecContext* pContext, AVFrame* pFrame)
{
    return 0;
}

bool VAAPIDecoder::initDecoder(VAProfile profile, VAEntrypoint entrypoint)
{
    cerr << "VAAPIDecoder::initDecoder" << endl;

    if (!hasProfile(profile)) {
        return false;
    }
/*    
    VAConfigAttrib attrib;
    VAConfigID config_id = 0;
    VAContextID context_id = 0;
    VASurfaceID surface_id = 0;
    VAStatus status;

    if (!has_profile(vaapi, profile))
        return -1;
    if (!has_entrypoint(vaapi, profile, entrypoint))
        return -1;

#if USE_GLX
    if (display_type() == DISPLAY_GLX) {
        GLXContext * const glx = glx_get_context();

        if (!glx)
            return -1;

        if (glx_init_texture(picture_width, picture_height) < 0)
            return -1;

        if (vaapi_glx_create_surface(glx->texture_target, glx->texture) < 0)
            return -1;
    }
#endif

    if (vaapi->profile != profile || vaapi->entrypoint != entrypoint) {
        if (vaapi->config_id)
            vaDestroyConfig(vaapi->display, vaapi->config_id);

        attrib.type = VAConfigAttribRTFormat;
        status = vaGetConfigAttributes(vaapi->display, profile, entrypoint,
                                       &attrib, 1);
        if (!vaapi_check_status(status, "vaGetConfigAttributes()"))
            return -1;
        if ((attrib.value & VA_RT_FORMAT_YUV420) == 0)
            return -1;

        status = vaCreateConfig(vaapi->display, profile, entrypoint,
                                &attrib, 1, &config_id);
        if (!vaapi_check_status(status, "vaCreateConfig()"))
            return -1;
    }
    else
        config_id = vaapi->config_id;

    if (vaapi->picture_width != picture_width || vaapi->picture_height != picture_height) {
        if (vaapi->surface_id)
            vaDestroySurfaces(vaapi->display, &vaapi->surface_id, 1);

        status = vaCreateSurfaces(vaapi->display, picture_width, picture_height,
                                  VA_RT_FORMAT_YUV420, 1, &surface_id);
        if (!vaapi_check_status(status, "vaCreateSurfaces()"))
            return -1;

        if (vaapi->context_id)
            vaDestroyContext(vaapi->display, vaapi->context_id);

        status = vaCreateContext(vaapi->display, config_id,
                                 picture_width, picture_height,
                                 VA_PROGRESSIVE,
                                 &surface_id, 1,
                                 &context_id);
        if (!vaapi_check_status(status, "vaCreateContext()"))
            return -1;
    }
    else {
        context_id = vaapi->context_id;
        surface_id = vaapi->surface_id;
    }

    vaapi->config_id      = config_id;
    vaapi->context_id     = context_id;
    vaapi->surface_id     = surface_id;
    vaapi->profile        = profile;
    vaapi->entrypoint     = entrypoint;
    vaapi->picture_width  = picture_width;
    vaapi->picture_height = picture_height;
*/    
    return true;
}

bool VAAPIDecoder::isSupportedCodec(CodecID codecID)
{
    return (codecID == CODEC_ID_MPEG2VIDEO || codecID == CODEC_ID_MPEG4 || 
            codecID == CODEC_ID_H263 || codecID == CODEC_ID_H264 || 
            codecID == CODEC_ID_WMV3 || codecID == CODEC_ID_VC1);
}

bool VAAPIDecoder::hasProfile(VAProfile profile)
{
    if (s_Profiles.size() == 0) {
        int numProfiles = vaMaxNumProfiles(getVAAPIDisplay());
        VAProfile *pProfiles = (VAProfile*)malloc(numProfiles*sizeof(VAProfile));
        VAStatus status = vaQueryConfigProfiles(getVAAPIDisplay(), pProfiles, &numProfiles);
        AVG_ASSERT(status == VA_STATUS_SUCCESS);
        cerr << "VAAPI Profiles available: " << endl;
        for (int i=0; i<numProfiles; ++i) {
            s_Profiles.push_back(pProfiles[i]);
            cerr << "  " << profileToString(pProfiles[i]) << endl;
        }
        free(pProfiles);
    }
    for (unsigned i=0; i<s_Profiles.size(); ++i) {
        if (s_Profiles[i] == profile) {
            return true;
        }
    }
    return false;
}

string VAAPIDecoder::profileToString(VAProfile profile)
{
    switch (profile) {
#define PROFILE(profile) \
        case VAProfile##profile: return "VAProfile" #profile
        PROFILE(MPEG2Simple);
        PROFILE(MPEG2Main);
        PROFILE(MPEG4Simple);
        PROFILE(MPEG4AdvancedSimple);
        PROFILE(MPEG4Main);
#if VA_CHECK_VERSION(0,32,0)
        PROFILE(JPEGBaseline);
        PROFILE(H263Baseline);
        PROFILE(H264ConstrainedBaseline);
#endif
        PROFILE(H264Baseline);
        PROFILE(H264Main);
        PROFILE(H264High);
        PROFILE(VC1Simple);
        PROFILE(VC1Main);
        PROFILE(VC1Advanced);
#undef PROFILE
    default: break;
    }
    return "<unknown>";
    
}

}

