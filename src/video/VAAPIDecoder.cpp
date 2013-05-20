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
#include "../graphics/X11Display.h"
#include "../graphics/GLTexture.h"

#ifdef AVG_ENABLE_EGL
#include <va/va_x11.h>
#else
#include <va/va_glx.h>
#endif

#include <iostream>
#include <sstream>

using namespace std;
    
namespace avg {

vector<VAProfile> VAAPIDecoder::s_Profiles;
std::map<unsigned, void *> VAAPIDecoder::s_GLSurfaceMap;

VAAPIDecoder::VAAPIDecoder()
    : m_Size(0, 0),
      m_ConfigID(unsigned(-1)),
      m_ContextID(unsigned(-1)),
      m_pFFMpegVAContext(0),
      m_pImageFmt(0),
      m_pImage(0)
{
}

VAAPIDecoder::~VAAPIDecoder()
{
    if (m_ConfigID != unsigned(-1)) {
        vaDestroyConfig(getDisplay(), m_ConfigID);
    }

    if (m_pImageFmt) {
        delete m_pImageFmt;
    }
    if (m_pImage) {
        vaDestroyImage(getDisplay(), m_pImage->image_id);
    }
    if (m_ContextID != unsigned(-1)) {
        vaDestroyContext(getDisplay(), m_ContextID);
        for (unsigned i=0; i<m_Surfaces.size(); ++i) {
            VASurfaceID surfaceID = m_Surfaces[i].getSurfaceID();
            vaDestroySurfaces(getDisplay(), &surfaceID, 1);
        }
    }

    if (m_pFFMpegVAContext) {
        delete m_pFFMpegVAContext;
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
            cerr << "VAAPI init failed" << endl;
            return 0;
        }
        m_pFFMpegVAContext = new vaapi_context;
        pContext->hwaccel_context = m_pFFMpegVAContext;
        memset(m_pFFMpegVAContext, 0, sizeof(vaapi_context));
        m_pFFMpegVAContext->display = getDisplay();
        m_pFFMpegVAContext->config_id = m_ConfigID;
        m_pFFMpegVAContext->context_id = m_ContextID;

        pContext->get_buffer = VAAPIDecoder::getBuffer;
        pContext->release_buffer = VAAPIDecoder::releaseBuffer;
        pContext->draw_horiz_band = 0;
        pContext->get_format = VAAPIDecoder::getFormat;
        pContext->slice_flags = SLICE_FLAG_CODED_ORDER | SLICE_FLAG_ALLOW_FIELD;
        pContext->thread_count = 1;
        pContext->pix_fmt = PIX_FMT_VAAPI_VLD;
        AVCodec* pCodec = avcodec_find_decoder(pContext->codec_id);
        return pCodec;
    } else {
        return 0;
    }
}

VAImage* VAAPIDecoder::getImage() const
{
    AVG_ASSERT(m_pImage);
    return m_pImage;
}

IntPoint VAAPIDecoder::getSize() const
{
    AVG_ASSERT(m_Size != IntPoint(0,0));
    return m_Size;
}

bool VAAPIDecoder::isAvailable()
{
#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(53, 34, 0)
    return getDisplay() != 0;
#else
    return false;
#endif
}

void VAAPIDecoder::checkError(VAStatus status, const string& sMsg)
{
    if (status != VA_STATUS_SUCCESS) {
        AVG_ASSERT_MSG(false, 
                (string("VAAPI error ('")+sMsg+"'): "+vaErrorStr(status)).c_str());
    }
}

VADisplay VAAPIDecoder::getDisplay()
{
    static bool bIsInitialized = false;
    static VADisplay vaDisplay = 0;
    if (!bIsInitialized) {
        bIsInitialized = true;
        ::Display* pDisplay = getX11Display(0);
#ifdef AVG_ENABLE_EGL
        vaDisplay = vaGetDisplay(pDisplay);
#else        
        vaDisplay = vaGetDisplayGLX(pDisplay);
#endif

        int majorVer;
        int minorVer;
        VAStatus status = vaInitialize(vaDisplay, &majorVer, &minorVer);
        if (status != VA_STATUS_SUCCESS) {
            vaDisplay = 0;
            throw Exception(AVG_ERR_VIDEO_INIT_FAILED, vaErrorStr(status));
        }
    }
    return vaDisplay;
}

void VAAPIDecoder::registerTexture(GLTexturePtr pTex)
{
    unsigned texid = pTex->getID();
    void* pVAGLSurface;
    VAStatus status = vaCreateSurfaceGLX(getDisplay(), GL_TEXTURE_2D, texid, &pVAGLSurface);
    VAAPIDecoder::checkError(status);
    
    s_GLSurfaceMap[texid] = pVAGLSurface;
}

void VAAPIDecoder::deregisterTexture(GLTexturePtr pTex)
{
    unsigned texid = pTex->getID();
    void* pVAGLSurface = s_GLSurfaceMap[texid];
    VAStatus status = vaDestroySurfaceGLX(getDisplay(), pVAGLSurface);
    VAAPIDecoder::checkError(status);
    s_GLSurfaceMap.erase(texid);
}

void * VAAPIDecoder::getVAGLSurface(GLTexturePtr pTex)
{
    unsigned texid = pTex->getID();
    void* pVAGLSurface = s_GLSurfaceMap[texid];
    return pVAGLSurface;
}

int VAAPIDecoder::getBuffer(AVCodecContext* pContext, AVFrame* pFrame)
{
    VAAPIDecoder* pVAAPIDecoder = (VAAPIDecoder*)pContext->opaque;
    return pVAAPIDecoder->getBufferInternal(pContext, pFrame);
}

void VAAPIDecoder::releaseBuffer(struct AVCodecContext* pContext, AVFrame* pFrame)
{
    VAAPIDecoder* pVAAPIDecoder = (VAAPIDecoder*)pContext->opaque;
    return pVAAPIDecoder->releaseBufferInternal(pContext, pFrame);
}

AVPixelFormat VAAPIDecoder::getFormat(AVCodecContext* pContext, const AVPixelFormat* pFmt)
{
    return PIX_FMT_VAAPI_VLD;
}

VAAPISurface* VAAPIDecoder::getFreeSurface()
{
/*
    int numSurfacesUsed = 0;
    for (unsigned i = 0; i<m_Surfaces.size(); i++) {
        VAAPISurface* pSurface = &m_Surfaces[i];
        if (pSurface->isUsed()) {
            numSurfacesUsed++;
        }
    }
    cerr << numSurfacesUsed << endl;
*/
    for (unsigned i = 0; i<m_Surfaces.size(); i++) {
        VAAPISurface* pSurface = &m_Surfaces[i];
        if (!pSurface->isUsed()) {
            pSurface->incRef();
            return pSurface;
        }
    }
    AVG_ASSERT(false);
    return 0;
}

int VAAPIDecoder::getBufferInternal(AVCodecContext* pContext, AVFrame* pFrame)
{
    VAAPISurface* pVAAPISurface = getFreeSurface();
    uint8_t* surfaceID = (uint8_t*)(uintptr_t)pVAAPISurface->getSurfaceID();

    VAStatus status = vaSyncSurface(getDisplay(), pVAAPISurface->getSurfaceID());
    checkError(status);

    pFrame->type = FF_BUFFER_TYPE_USER;
    pFrame->data[0] = surfaceID;
    pFrame->data[1] = 0;
    pFrame->data[2] = 0;
    pFrame->data[3] = surfaceID;
    pFrame->linesize[0] = 0;
    pFrame->linesize[1] = 0;
    pFrame->linesize[2] = 0;
    pFrame->linesize[3] = 0;
    pFrame->opaque = (void*)pVAAPISurface;
    return 0;
    
}

void VAAPIDecoder::releaseBufferInternal(struct AVCodecContext* pContext, AVFrame* pFrame)
{
    VAAPISurface* pSurface = (VAAPISurface*)(pFrame->opaque);
    pSurface->decRef();

    pFrame->data[0] = 0;
    pFrame->data[3] = 0;
    pFrame->opaque = 0;
}

bool VAAPIDecoder::initDecoder(VAProfile profile)
{
    if (!hasProfile(profile)) {
        cerr << "  Profile " << profileToString(profile) << " not available." << endl;
        return false;
    }

    if (!hasEntryPoint(profile, VAEntrypointVLD)) {
        return false;
    }

    int numSurfaces = 2+1;
    if (profile == VAProfileH264High) {
        numSurfaces = 16+1;
    }

    VAConfigAttrib attrib;
    attrib.type = VAConfigAttribRTFormat;
    VAStatus status = vaGetConfigAttributes(getDisplay(), profile, VAEntrypointVLD,
            &attrib, 1);
    checkError(status, "initDecoder: vaGetConfigAttributes");
    if ((attrib.value & VA_RT_FORMAT_YUV420) == 0) {
        return false;
    }
    status = vaCreateConfig(getDisplay(), profile, VAEntrypointVLD, &attrib, 1, 
            &m_ConfigID);
    checkError(status, "initDecoder: vaCreateConfig");

    VASurfaceID surfaceIDs[40];
    status = vaCreateSurfaces(getDisplay(), m_Size.x, m_Size.y, VA_RT_FORMAT_YUV420,
            numSurfaces, surfaceIDs);
    checkError(status, "initDecoder: vaCreateSurfaces");
        
    status = vaCreateContext(getDisplay(), m_ConfigID, m_Size.x, m_Size.y,
            VA_PROGRESSIVE, surfaceIDs, numSurfaces, &m_ContextID);
    if (status != VA_STATUS_SUCCESS) {
        cerr << "vaCreateContext failed, disabling VAAPI." << endl;
        return false;
    }

    determineImageFormat(surfaceIDs[0]);
//    cerr << "Decoding to image format " << imageFmtToString(m_pImageFmt) << endl;

    m_pImage = new VAImage();
    status = vaCreateImage(getDisplay(), m_pImageFmt, m_Size.x, m_Size.y, m_pImage);
    checkError(status, "initDecoder: vaCreateImage");

    AVG_ASSERT(m_Surfaces.size() == 0);
    for (int i=0; i<numSurfaces; ++i) {
        m_Surfaces.push_back(VAAPISurface(surfaceIDs[i], this));
    }

    return true;
}

void VAAPIDecoder::determineImageFormat(const VASurfaceID& surface)
{
    int numFmts = vaMaxNumImageFormats(getDisplay());
    VAImageFormat* pFmts = new VAImageFormat[numFmts];
    AVG_ASSERT(pFmts);

    VAStatus status = vaQueryImageFormats(getDisplay(), pFmts, &numFmts);
    checkError(status);
    AVG_ASSERT(m_pImageFmt == 0);
/*
    cerr << "Image formats available: " << endl;
    for (int i=0; i<numFmts; ++i) {
        cerr << "  " << imageFmtToString(&(pFmts[i])) << endl;
    }
*/
    m_pImageFmt = checkImageFormat(surface, numFmts, pFmts, VA_FOURCC_YV12);
    if (!m_pImageFmt) {
        m_pImageFmt = checkImageFormat(surface, numFmts, pFmts, VA_FOURCC_NV12);
    }
    // TODO: Disable acceleration if image format not supported.
    AVG_ASSERT(m_pImageFmt);
}
    
VAImageFormat* VAAPIDecoder::checkImageFormat(const VASurfaceID& surface, int numFmts,
        VAImageFormat* pFmts, unsigned long fourcc)
{
    for (int i=0; i<numFmts; ++i) {
        if (pFmts[i].fourcc == fourcc) {
            VAImageFormat* pImageFmt = new VAImageFormat;
            *pImageFmt = pFmts[i];

            // Check if the format is supported by creating an image and calling vaGetImage
            // on it.
            VAImage* pImage = new VAImage();
            VAStatus status = vaCreateImage(getDisplay(), pImageFmt, m_Size.x, m_Size.y, 
                    pImage);
            checkError(status);
            status = vaGetImage(getDisplay(), surface, 0, 0, m_Size.x, m_Size.y, 
                    pImage->image_id);
            vaDestroyImage(getDisplay(), pImage->image_id);
            if (status == VA_STATUS_SUCCESS) {
                // Format is supported for vaGetImage -> use it.
                return pImageFmt;
            } else {
                delete pImageFmt;
            }
        }
    }
    return 0;
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
        int numProfiles = vaMaxNumProfiles(getDisplay());
        VAProfile *pProfiles = (VAProfile*)malloc(numProfiles*sizeof(VAProfile));
        VAStatus status = vaQueryConfigProfiles(getDisplay(), pProfiles, &numProfiles);
        checkError(status);
//        cerr << "VAAPI Profiles available: " << endl;
        for (int i=0; i<numProfiles; ++i) {
            s_Profiles.push_back(pProfiles[i]);
//            cerr << "  " << profileToString(pProfiles[i]) << endl;
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
    int numEntryPoints = vaMaxNumEntrypoints(getDisplay());
    VAEntrypoint *pEntryPoints =
            (VAEntrypoint*)malloc(numEntryPoints*sizeof(VAEntrypoint));
    VAStatus status = vaQueryConfigEntrypoints(getDisplay(), profile, pEntryPoints, 
            &numEntryPoints);
    checkError(status);
//    cerr << "VAAPI entry points available for " << profileToString(profile) << ":" << endl;
    bool bEntryPointFound = false;
    for (int i=0; i<numEntryPoints; ++i) {
//        cerr << "  " << entryPointToString(pEntryPoints[i]) << endl;
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

string VAAPIDecoder::imageFmtToString(VAImageFormat* pFormat)
{
    stringstream ss;
    char fourcc[5];
    memcpy(fourcc, &(pFormat->fourcc), 4);
    fourcc[4] = 0;

    return fourcc;
}

}

