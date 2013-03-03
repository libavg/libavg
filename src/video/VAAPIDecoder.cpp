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

    AVCodec* pCodec = 0;
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
    if (pCodec) {
        pContext->get_buffer = VAAPIDecoder::getBuffer;
        pContext->release_buffer = VAAPIDecoder::releaseBuffer;
        pContext->draw_horiz_band = VAAPIDecoder::drawHorizBand;
        pContext->get_format = VAAPIDecoder::getFormat;
        pContext->slice_flags = SLICE_FLAG_CODED_ORDER | SLICE_FLAG_ALLOW_FIELD;
        m_Size = IntPoint(pContext->width, pContext->height);
    }
    return pCodec;
}

bool VAAPIDecoder::isAvailable()
{
#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(53, 34, 0)
    return false;
#else
    return false;
#endif
}

int VAAPIDecoder::getBuffer(AVCodecContext* pContext, AVFrame* pFrame)
{
    return 0;
}

void VAAPIDecoder::releaseBuffer(struct AVCodecContext* pContext, AVFrame* pFrame)
{
}


AVPixelFormat VAAPIDecoder::getFormat(AVCodecContext* pContext, const AVPixelFormat* pFmt)
{
    if (pContext->codec_id == CODEC_ID_MPEG2VIDEO || pContext->codec_id == CODEC_ID_MPEG4 ||
            pContext->codec_id == CODEC_ID_H263 || pContext->codec_id == CODEC_ID_H264 ||
            pContext->codec_id == CODEC_ID_WMV3 || pContext->codec_id == CODEC_ID_VC1)
    {
        return PIX_FMT_VAAPI_VLD;
    } else {
        return pFmt[0];
    }
}

}


