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
    : m_Size(-1,-1),
      m_ConfigID(unsigned(-1)),
      m_pFFMpegVAContext(0)
{
}

VAAPIDecoder::~VAAPIDecoder()
{
    if (m_ConfigID != unsigned(-1)) {
        vaDestroyConfig(getVAAPIDisplay(), m_ConfigID);
    }
}

AVCodec* VAAPIDecoder::openCodec(AVCodecContext* pContext)
{
    if (!isAvailable()) {
        return 0;
    }

    if (isSupportedCodec(pContext->codec_id)) {
        int profile = -1;
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
                AVG_ASSERT(false);
        }
        m_Size = IntPoint(pContext->width, pContext->height);
        bool bOK = initDecoder((VAProfile)profile);
        if (!bOK) {
            return 0;
        }
        m_pFFMpegVAContext = new vaapi_context;
        pContext->hwaccel_context = m_pFFMpegVAContext;
        memset(m_pFFMpegVAContext, 0, sizeof(vaapi_context));
        m_pFFMpegVAContext->display = getVAAPIDisplay();
        m_pFFMpegVAContext->config_id = m_ConfigID;
        m_pFFMpegVAContext->context_id = m_ContextID;

        pContext->get_buffer = VAAPIDecoder::getBuffer;
        pContext->release_buffer = VAAPIDecoder::releaseBuffer;
        pContext->draw_horiz_band = 0;
        pContext->get_format = VAAPIDecoder::getFormat;
        pContext->slice_flags = SLICE_FLAG_CODED_ORDER | SLICE_FLAG_ALLOW_FIELD;
        pContext->pix_fmt = PIX_FMT_VAAPI_VLD;
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
    pFrame->data[0] = 0;
    pFrame->data[1] = 0;
    pFrame->data[2] = 0;
    pFrame->data[3] = 0;
}

AVPixelFormat VAAPIDecoder::getFormat(AVCodecContext* pContext, const AVPixelFormat* pFmt)
{
    cerr << "getFormat" << endl;
    return PIX_FMT_VAAPI_VLD;
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
    uint8_t *surface = (uint8_t*)(uintptr_t)m_Surface;
    pFrame->type = FF_BUFFER_TYPE_USER;
    pFrame->data[0] = surface;
    pFrame->data[1] = 0;
    pFrame->data[2] = 0;
    pFrame->data[3] = surface;
    pFrame->linesize[0] = 0;
    pFrame->linesize[1] = 0;
    pFrame->linesize[2] = 0;
    pFrame->linesize[3] = 0;
    return 0;
    
}

bool VAAPIDecoder::initDecoder(VAProfile profile)
{
    cerr << "VAAPIDecoder::initDecoder" << endl;
/*    
    VAContextID context_id = 0;
    VAStatus status;
*/

    if (!hasProfile(profile)) {
        return false;
    }

    if (!hasEntryPoint(profile, VAEntrypointVLD)) {
        return false;
    }

    VAConfigAttrib attrib;
    attrib.type = VAConfigAttribRTFormat;
    VAStatus status = vaGetConfigAttributes(getVAAPIDisplay(), profile, VAEntrypointVLD,
            &attrib, 1);
    AVG_ASSERT(status == VA_STATUS_SUCCESS);
    if ((attrib.value & VA_RT_FORMAT_YUV420) == 0) {
        return false;
    }
    status = vaCreateConfig(getVAAPIDisplay(), profile, VAEntrypointVLD, &attrib, 1, 
            &m_ConfigID);
    AVG_ASSERT(status == VA_STATUS_SUCCESS);

    
    status = vaCreateSurfaces(getVAAPIDisplay(), m_Size.x, m_Size.y, VA_RT_FORMAT_YUV420,
            1, &m_Surface);
    AVG_ASSERT(status == VA_STATUS_SUCCESS);
        
    status = vaCreateContext(getVAAPIDisplay(), m_ConfigID, m_Size.x, m_Size.y,
            VA_PROGRESSIVE, &m_Surface, 1, &m_ContextID);
    AVG_ASSERT(status == VA_STATUS_SUCCESS);


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

bool VAAPIDecoder::hasEntryPoint(VAProfile profile, VAEntrypoint entryPoint)
{
    int numEntryPoints = vaMaxNumEntrypoints(getVAAPIDisplay());
    VAEntrypoint *pEntryPoints =
            (VAEntrypoint*)malloc(numEntryPoints*sizeof(VAEntrypoint));
    VAStatus status = vaQueryConfigEntrypoints(getVAAPIDisplay(), profile, pEntryPoints, 
            &numEntryPoints);
    AVG_ASSERT(status == VA_STATUS_SUCCESS);
    cerr << "VAAPI entry points available for " << profileToString(profile) << ":" << endl;
    bool bEntryPointFound = false;
    for (int i=0; i<numEntryPoints; ++i) {
        cerr << "  " << entryPointToString(pEntryPoints[i]) << endl;
        if (pEntryPoints[i] == entryPoint) {
            bEntryPointFound = true;
        }
    }
    free(pEntryPoints);
    return bEntryPointFound;
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

string VAAPIDecoder::entryPointToString(VAEntrypoint entryPoint)
{
    switch (entryPoint) {
#define ENTRYPOINT(entryPoint) \
        case VAEntrypoint##entryPoint: return "VAEntrypoint" #entryPoint
        ENTRYPOINT(VLD);
        ENTRYPOINT(IZZ);
        ENTRYPOINT(IDCT);
        ENTRYPOINT(MoComp);
        ENTRYPOINT(Deblocking);
#if VA_CHECK_VERSION(0,32,0)
        ENTRYPOINT(EncSlice);
        ENTRYPOINT(EncPicture);
#endif
#undef ENTRYPOINT
    default: break;
    }
    return "<unknown>";
}

}

