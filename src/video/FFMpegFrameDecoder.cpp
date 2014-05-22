//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2014 Ulrich von Zadow
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

#include "FFMpegFrameDecoder.h"
#include "FFMpegDemuxer.h"
#include "VideoInfo.h"
#ifdef AVG_ENABLE_VDPAU
#include "VDPAUDecoder.h"
#endif

#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/ScopeTimer.h"
#include "../base/ObjectCounter.h"
#include "../base/ProfilingZoneID.h"
#include "../base/StringHelper.h"

#include <iostream>
#include <sstream>
#ifndef _WIN32
#include <unistd.h>
#endif

using namespace std;

namespace avg {

FFMpegFrameDecoder::FFMpegFrameDecoder(AVStream* pStream)
    : m_pSwsContext(0),
      m_pStream(pStream),
      m_bEOF(false),
      m_StartTimestamp(-1),
      m_LastFrameTime(-1),
      m_bUseStreamFPS(true)
{
    m_TimeUnitsPerSecond = float(1.0/av_q2d(pStream->time_base));
    m_FPS = getStreamFPS(pStream);
    
    ObjectCounter::get()->incRef(&typeid(*this));
}

FFMpegFrameDecoder::~FFMpegFrameDecoder()
{
    if (m_pSwsContext) {
        sws_freeContext(m_pSwsContext);
        m_pSwsContext = 0;
    }
    ObjectCounter::get()->decRef(&typeid(*this));
}

static ProfilingZoneID DecodePacketProfilingZone("Decode packet", true);

bool FFMpegFrameDecoder::decodePacket(AVPacket* pPacket, AVFrame* pFrame,
        bool bFrameAfterSeek)
{
    ScopeTimer timer(DecodePacketProfilingZone);
    int bGotPicture = 0;
    AVCodecContext* pContext = m_pStream->codec;
    AVG_ASSERT(pPacket);
    avcodec_decode_video2(pContext, pFrame, &bGotPicture, pPacket);
    if (bGotPicture) {
        m_LastFrameTime = getFrameTime(pPacket->dts, bFrameAfterSeek);
    }
    av_free_packet(pPacket);
    delete pPacket;
    return (bGotPicture != 0);
}

bool FFMpegFrameDecoder::decodeLastFrame(AVFrame* pFrame)
{
    // EOF. Decode the last data we got.
    int bGotPicture = 0;
    AVCodecContext* pContext = m_pStream->codec;
    AVPacket packet;
    av_init_packet(&packet);
    packet.data = 0;
    packet.size = 0;
    avcodec_decode_video2(pContext, pFrame, &bGotPicture, &packet);
    m_bEOF = true;

    // We don't have a timestamp for the last frame, so we'll
    // calculate it based on the frame before.
    m_LastFrameTime += 1.0f/m_FPS;
    return (bGotPicture != 0);
}


static ProfilingZoneID ConvertImageLibavgProfilingZone(
        "FFMpeg: colorspace conv (libavg)", true);
static ProfilingZoneID ConvertImageSWSProfilingZone(
        "FFMpeg: colorspace conv (SWS)", true);
static ProfilingZoneID SetAlphaProfilingZone("FFMpeg: set alpha channel", true);

void FFMpegFrameDecoder::convertFrameToBmp(AVFrame* pFrame, BitmapPtr pBmp)
{
    AVPicture destPict;
    unsigned char * pDestBits = pBmp->getPixels();
    destPict.data[0] = pDestBits;
    destPict.linesize[0] = pBmp->getStride();
    AVPixelFormat destFmt;
    switch (pBmp->getPixelFormat()) {
        case R8G8B8X8:
        case R8G8B8A8:
            destFmt = PIX_FMT_RGBA;
            break;
        case B8G8R8X8:
        case B8G8R8A8:
            destFmt = PIX_FMT_BGRA;
            break;
        case R8G8B8:
            destFmt = PIX_FMT_RGB24;
            break;
        case B8G8R8:
            destFmt = PIX_FMT_BGR24;
            break;
        case YCbCr422:
            destFmt = PIX_FMT_YUYV422;
            break;
        default:
            AVG_ASSERT_MSG(false, (string("FFMpegFrameDecoder: Dest format ") +
                    toString(pBmp->getPixelFormat()) + " not supported.").c_str());
            destFmt = PIX_FMT_BGRA;
    }
    AVCodecContext const* pContext = m_pStream->codec;
    if (destFmt == PIX_FMT_BGRA && (pContext->pix_fmt == PIX_FMT_YUV420P || 
                pContext->pix_fmt == PIX_FMT_YUVJ420P))
    {
        ScopeTimer timer(ConvertImageLibavgProfilingZone);
        BitmapPtr pBmpY(new Bitmap(pBmp->getSize(), I8, pFrame->data[0],
                pFrame->linesize[0], false));
        BitmapPtr pBmpU(new Bitmap(pBmp->getSize(), I8, pFrame->data[1],
                pFrame->linesize[1], false));
        BitmapPtr pBmpV(new Bitmap(pBmp->getSize(), I8, pFrame->data[2],
                pFrame->linesize[2], false));
        pBmp->copyYUVPixels(*pBmpY, *pBmpU, *pBmpV, 
                pContext->pix_fmt == PIX_FMT_YUVJ420P);
    } else {
        if (!m_pSwsContext) {
            m_pSwsContext = sws_getContext(pContext->width, pContext->height, 
                    pContext->pix_fmt, pContext->width, pContext->height, destFmt, 
                    SWS_BICUBIC, 0, 0, 0);
            AVG_ASSERT(m_pSwsContext);
        }
        {
            ScopeTimer timer(ConvertImageSWSProfilingZone);
            sws_scale(m_pSwsContext, pFrame->data, pFrame->linesize, 0, 
                    pContext->height, destPict.data, destPict.linesize);
        }
        if (pBmp->getPixelFormat() == B8G8R8X8 || pBmp->getPixelFormat() == R8G8B8X8) {
            ScopeTimer timer(SetAlphaProfilingZone);
            // Make sure the alpha channel is white.
            // TODO: This is slow. Make OpenGL do it.
            unsigned char * pLine = pBmp->getPixels();
            IntPoint size = pBmp->getSize();
            for (int y = 0; y < size.y; ++y) {
                unsigned char * pPixel = pLine;
                for (int x = 0; x < size.x; ++x) {
                    pPixel[3] = 0xFF;
                    pPixel += 4;
                }
                pLine = pLine + pBmp->getStride();
            }
        }
    }
}

void FFMpegFrameDecoder::copyPlaneToBmp(BitmapPtr pBmp, unsigned char * pData, int stride)
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

void FFMpegFrameDecoder::handleSeek()
{
    m_LastFrameTime = -1.0f;
    avcodec_flush_buffers(m_pStream->codec);
    m_bEOF = false;
    if (m_StartTimestamp == -1) {
        m_StartTimestamp = 0;
    }
}

float FFMpegFrameDecoder::getCurTime() const
{
    return m_LastFrameTime;
}

float FFMpegFrameDecoder::getFPS() const
{
    return m_FPS;
}

void FFMpegFrameDecoder::setFPS(float fps)
{
    m_bUseStreamFPS = (fps == 0);
    if (fps == 0) {
        m_FPS = getStreamFPS(m_pStream);
    } else {
        m_FPS = fps;
    }
}

bool FFMpegFrameDecoder::isEOF() const
{
    return m_bEOF;
}

float FFMpegFrameDecoder::getFrameTime(long long dts, bool bFrameAfterSeek)
{
    bool bUseStreamFPS = m_bUseStreamFPS;
    if (dts == (long long)AV_NOPTS_VALUE) {
        bUseStreamFPS = false;
        dts = 0;
    }
    if (m_StartTimestamp == -1) {
        m_StartTimestamp = dts;
    }
    float frameTime;
    if (bUseStreamFPS || bFrameAfterSeek) {
        frameTime = float(dts-m_StartTimestamp)/m_TimeUnitsPerSecond;
    } else {
        if (m_LastFrameTime == -1) {
            frameTime = 0;
        } else {
            frameTime = m_LastFrameTime + 1.0f/m_FPS;
        }
    }
    return frameTime;
}

}

