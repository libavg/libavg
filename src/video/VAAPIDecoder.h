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
#ifndef _VAAPIDecoder_H_
#define _VAAPIDecoder_H_


#include "../avgconfigwrapper.h"
#include "../base/GLMHelper.h"

#include "WrapFFMpeg.h"
#include "VAAPIHelper.h"
#include "VAAPISurface.h"

#include <libavcodec/vaapi.h>

namespace avg {

struct VAAPISurfaceInfo
{
    VAAPISurfaceInfo(VASurfaceID surfaceID);
    VASurfaceID m_SurfaceID;
    bool m_bUsed;
};

class VAAPIDecoder
{
public:
    VAAPIDecoder();
    ~VAAPIDecoder();
    AVCodec* openCodec(AVCodecContext* pCodec);

    static bool isAvailable();

private:
    // Callbacks
    static int getBuffer(AVCodecContext* pContext, AVFrame* pFrame);
    static void releaseBuffer(AVCodecContext* pContext, AVFrame* pFrame);
    static AVPixelFormat getFormat(AVCodecContext* pContext, const AVPixelFormat* pFmt);

    VAAPISurfaceInfo* getFreeSurface();

    int getBufferInternal(AVCodecContext* pContext, AVFrame* pFrame);
    void releaseBufferInternal(AVCodecContext* pContext, AVFrame* pFrame);
    bool initDecoder(VAProfile profile);
    VAImageFormat* determineImageFormat();

    static bool isSupportedCodec(CodecID codecID);
    static bool hasProfile(VAProfile profile);
    static bool hasEntryPoint(VAProfile profile, VAEntrypoint entryPoint);
    static std::string profileToString(VAProfile profile);
    static std::string entryPointToString(VAEntrypoint entryPoint);
    static std::string imageFmtToString(VAImageFormat* pFormat);

    IntPoint m_Size;
    VAConfigID m_ConfigID;
    VAContextID m_ContextID;
    std::vector<VAAPISurfaceInfo> m_Surfaces;
    vaapi_context* m_pFFMpegVAContext;

    static std::vector<VAProfile> s_Profiles;
};

}
#endif

