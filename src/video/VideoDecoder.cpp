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

#include "VideoDecoder.h"
#ifdef AVG_ENABLE_VDPAU
#include "VDPAUDecoder.h"
#endif

#include "../base/Exception.h"
#include "../base/Logger.h"

#include "../graphics/Bitmap.h"

#include <string>

#include "WrapFFMpeg.h"

using namespace std;
using namespace boost;

namespace avg {

bool VideoDecoder::s_bInitialized = false;
mutex VideoDecoder::s_OpenMutex;

FrameAvailableCode VideoDecoder::renderToBmp(BitmapPtr pBmp, float timeWanted)
{
    std::vector<BitmapPtr> pBmps;
    pBmps.push_back(pBmp);
    return renderToBmps(pBmps, timeWanted);
}

FrameAvailableCode VideoDecoder::renderToVDPAU(vdpau_render_state** ppRenderState)
{
    AVG_ASSERT(false);
    return FA_NEW_FRAME; // Silence compiler warning.
}

void VideoDecoder::logConfig()
{
    bool bVDPAUAvailable = false;
#ifdef AVG_ENABLE_VDPAU
    bVDPAUAvailable = VDPAUDecoder::isAvailable();
#endif
    if (bVDPAUAvailable) {
        AVG_TRACE(Logger::CONFIG, "Hardware video acceleration: VDPAU");
    } else {
        AVG_TRACE(Logger::CONFIG, "Hardware video acceleration: Off");
    }
}

void VideoDecoder::initVideoSupport()
{
    if (!s_bInitialized) {
        av_register_all();
        s_bInitialized = true;
        // Tune libavcodec console spam.
//        av_log_set_level(AV_LOG_DEBUG);
        av_log_set_level(AV_LOG_QUIET);
    }
}

void avcodecError(const string& sFilename, int err)
{
#if LIBAVFORMAT_VERSION_MAJOR > 52
        char buf[256];
        av_strerror(err, buf, 256);
        throw Exception(AVG_ERR_VIDEO_INIT_FAILED, sFilename + ": " + buf);
#else
    switch(err) {
        case AVERROR_NUMEXPECTED:
            throw Exception(AVG_ERR_VIDEO_INIT_FAILED, 
                    sFilename + ": Incorrect image filename syntax (use %%d to specify the image number:");
        case AVERROR_INVALIDDATA:
            throw Exception(AVG_ERR_VIDEO_INIT_FAILED, 
                    sFilename + ": Error while parsing header");
        case AVERROR_NOFMT:
            throw Exception(AVG_ERR_VIDEO_INIT_FAILED, 
                    sFilename + ": Unknown format");
        default:
            stringstream s;
            s << "'" << sFilename <<  "': Error while opening file (Num:" << err << ")";
            throw Exception(AVG_ERR_VIDEO_INIT_FAILED, s.str());
    }
#endif
}

void copyPlaneToBmp(BitmapPtr pBmp, unsigned char * pData, int stride)
{
    unsigned char * pSrc=pData;
    unsigned char * pDest= pBmp->getPixels();
    int destStride = pBmp->getStride();
    int height = pBmp->getSize().y;
    int width = pBmp->getSize().x;
    for (int y = 0; y < height; y++) {
        memcpy(pDest, pSrc, width);
        pSrc += stride;
        pDest += destStride;
    }
}

}


