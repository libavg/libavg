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

#include "SyncDecoder.h"
#include "AsyncDemuxer.h"
#include "FFMpegDemuxer.h"

#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/ScopeTimer.h"
#include "../base/ObjectCounter.h"
#include "../base/ProfilingZoneID.h"
#include "../base/StringHelper.h"

#include "../audio/AudioBuffer.h"

#include "../graphics/BitmapLoader.h"

#include <iostream>
#include <sstream>
#ifndef _WIN32
#include <unistd.h>
#endif

using namespace std;
using namespace boost;

namespace avg {

SyncDecoder::SyncDecoder()
    : m_State(CLOSED),
      m_pFormatContext(0),
      m_PF(NO_PIXELFORMAT),
      m_pSwsContext(0),
      m_Size(0,0),
      m_bUseStreamFPS(true),
      m_pDemuxer(0),
      m_pVStream(0),
      m_VStreamIndex(-1),
      m_bFirstPacket(false),
      m_VideoStartTimestamp(-1),
      m_LastVideoFrameTime(-1),
      m_FPS(0)
{
    ObjectCounter::get()->incRef(&typeid(*this));
    initVideoSupport();
}

SyncDecoder::~SyncDecoder()
{
    if (m_pFormatContext) {
        close();
    }
    ObjectCounter::get()->decRef(&typeid(*this));
}

int SyncDecoder::openCodec(int streamIndex)
{
    AVCodecContext* pContext;
    pContext = m_pFormatContext->streams[streamIndex]->codec;
//    pContext->debug = 0x0001; // see avcodec.h

    AVCodec * pCodec = 0;

    if (!pCodec) {
        pCodec = avcodec_find_decoder(pContext->codec_id);
    }
    if (!pCodec) {
        return -1;
    }
#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(53, 8, 0)
    int rc = avcodec_open2(pContext, pCodec, 0);
#else
    int rc = avcodec_open(pContext, pCodec);
#endif

    if (rc < 0) {
        return -1;
    }
    return 0;
}

void SyncDecoder::open(const string& sFilename, bool bUseHardwareAcceleration, 
        bool bEnableSound)
{
    mutex::scoped_lock lock(s_OpenMutex);
    m_bVideoEOF = false;
    m_bEOFPending = false;
    m_VideoStartTimestamp = -1;
    int err;
    m_sFilename = sFilename;

    AVG_TRACE(Logger::MEMORY, "Opening " << sFilename);
#if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(53,2,0)
    err = avformat_open_input(&m_pFormatContext, sFilename.c_str(), 0, 0);
#else
    AVFormatParameters params;
    memset(&params, 0, sizeof(params));
    err = av_open_input_file(&m_pFormatContext, sFilename.c_str(), 
            0, 0, &params);
#endif
    if (err < 0) {
        m_sFilename = "";
        m_pFormatContext = 0;
        avcodecError(sFilename, err);
    }
    
#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(53, 8, 0)
    err = avformat_find_stream_info(m_pFormatContext, 0);
#else
    err = av_find_stream_info(m_pFormatContext);
#endif

    if (err < 0) {
        m_sFilename = "";
        m_pFormatContext = 0;
        throw Exception(AVG_ERR_VIDEO_INIT_FAILED, 
                sFilename + ": Could not find codec parameters.");
    }
    if (strcmp(m_pFormatContext->iformat->name, "image2") == 0) {
        m_sFilename = "";
        m_pFormatContext = 0;
        throw Exception(AVG_ERR_VIDEO_INIT_FAILED, 
                sFilename + ": Image files not supported as videos.");
    }
    av_read_play(m_pFormatContext);
    
    // Find audio and video streams in the file
    m_VStreamIndex = -1;
    for (unsigned i = 0; i < m_pFormatContext->nb_streams; i++) {
        AVCodecContext* pContext = m_pFormatContext->streams[i]->codec;
        switch (pContext->codec_type) {
            case AVMEDIA_TYPE_VIDEO:
                if (m_VStreamIndex < 0) {
                    m_VStreamIndex = i;
                }
                break;
            default:
                break;
        }
    }
    
    // Enable video stream demuxing
    if (m_VStreamIndex >= 0) {
        m_pVStream = m_pFormatContext->streams[m_VStreamIndex];
        m_State = OPENED;
        
        // Set video parameters
        m_TimeUnitsPerSecond = float(1.0/av_q2d(m_pVStream->time_base));
        if (m_bUseStreamFPS) {
            m_FPS = getNominalFPS();
        }
        m_Size = IntPoint(m_pVStream->codec->width, m_pVStream->codec->height);
        m_bFirstPacket = true;
        m_sFilename = sFilename;
        m_LastVideoFrameTime = -1;

        int rc = openCodec(m_VStreamIndex);
        if (rc == -1) {
            m_VStreamIndex = -1;
            char szBuf[256];
            avcodec_string(szBuf, sizeof(szBuf), m_pVStream->codec, 0);
            m_pVStream = 0;
            throw Exception(AVG_ERR_VIDEO_INIT_FAILED, 
                    sFilename + ": unsupported video codec ("+szBuf+").");
        }
        m_PF = calcPixelFormat(true);
        m_bVideoSeekDone = false;
    }
    if (!m_pVStream) {
        throw Exception(AVG_ERR_VIDEO_INIT_FAILED, sFilename + ": no video streams found.");
    }

    m_State = OPENED;
}

void SyncDecoder::startDecoding(bool bDeliverYCbCr, const AudioParams* pAP)
{
    AVG_ASSERT(m_State == OPENED);

    if (m_VStreamIndex < 0) {
        throw Exception(AVG_ERR_VIDEO_INIT_FAILED, 
                m_sFilename + " does not contain any valid audio or video streams.");
    }
    m_PF = calcPixelFormat(bDeliverYCbCr);

    AVG_ASSERT(!m_pDemuxer);
    vector<int> streamIndexes;
    streamIndexes.push_back(m_VStreamIndex);
    m_pDemuxer = new FFMpegDemuxer(m_pFormatContext, streamIndexes);
    
    m_State = DECODING;
}

void SyncDecoder::close() 
{
    mutex::scoped_lock lock(s_OpenMutex);
    AVG_TRACE(Logger::MEMORY, "Closing " << m_sFilename);
    
    delete m_pDemuxer;
    m_pDemuxer = 0;
    
    // Close audio and video codecs
    if (m_pVStream) {
        avcodec_close(m_pVStream->codec);
        m_pVStream = 0;
        m_VStreamIndex = -1;
    }

    if (m_pFormatContext) {
#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(53, 21, 0)
        avformat_close_input(&m_pFormatContext);
#else
        av_close_input_file(m_pFormatContext);
        m_pFormatContext = 0;
#endif
    }
    
    if (m_pSwsContext) {
        sws_freeContext(m_pSwsContext);
        m_pSwsContext = 0;
    }
    m_State = CLOSED;
}

VideoDecoder::DecoderState SyncDecoder::getState() const
{
    return m_State;
}

VideoInfo SyncDecoder::getVideoInfo() const
{
    AVG_ASSERT(m_State != CLOSED);
    float duration = 0;
    if (m_pVStream) {
        duration = getDuration();
    }
    VideoInfo info(m_pFormatContext->iformat->name, duration, m_pFormatContext->bit_rate,
            true, false);
    info.setVideoData(m_Size, getStreamPF(), getNumFrames(), getNominalFPS(), m_FPS,
            m_pVStream->codec->codec->name, false, getDuration(SS_VIDEO));
    return info;
}

void SyncDecoder::seek(float destTime) 
{
    AVG_ASSERT(m_State == DECODING);

    if (m_bFirstPacket) {
        AVFrame frame;
        readFrame(frame);
    }
    dynamic_cast<FFMpegDemuxer*>(m_pDemuxer)
            ->seek(destTime + m_VideoStartTimestamp/m_TimeUnitsPerSecond);
    m_bVideoSeekDone = true;
    m_bVideoEOF = false;
    m_LastVideoFrameTime = -1.0f;
    avcodec_flush_buffers(getCodecContext());
}

void SyncDecoder::loop()
{
    seek(0);
}

IntPoint SyncDecoder::getSize() const
{
    AVG_ASSERT(m_State != CLOSED);
    return m_Size;
}

int SyncDecoder::getCurFrame() const
{
    AVG_ASSERT(m_State != CLOSED);
    return int(m_LastVideoFrameTime*getNominalFPS()+0.5);
}

int SyncDecoder::getNumFramesQueued() const
{
    return 0;
}

float SyncDecoder::getCurTime(StreamSelect stream) const
{
    AVG_ASSERT(m_State != CLOSED);
    AVG_ASSERT(stream != SS_AUDIO);
    return m_LastVideoFrameTime;
}

float SyncDecoder::getDuration(StreamSelect streamSelect) const
{
    AVG_ASSERT(m_State != CLOSED);
    AVG_ASSERT(streamSelect != SS_AUDIO);
    long long duration;
    AVRational time_base;
    duration = m_pVStream->duration;
    time_base = m_pVStream->time_base;

    if (duration == (long long)AV_NOPTS_VALUE) {
        return 0;
    } else {
        return float(duration)*float(av_q2d(time_base));
    }
}

float SyncDecoder::getNominalFPS() const
{
    AVG_ASSERT(m_State != CLOSED);
    return float(av_q2d(m_pVStream->r_frame_rate));
}

float SyncDecoder::getFPS() const
{
    AVG_ASSERT(m_State != CLOSED);
    return m_FPS;
}

void SyncDecoder::setFPS(float fps)
{
    m_bUseStreamFPS = (fps == 0);
    if (fps == 0) {
        m_FPS = calcStreamFPS();
    } else {
        m_FPS = fps;
    }
}

static ProfilingZoneID RenderToBmpProfilingZone("FFMpeg: renderToBmp", true);
static ProfilingZoneID CopyImageProfilingZone("FFMpeg: copy image", true);
static ProfilingZoneID VDPAUCopyProfilingZone("FFMpeg: VDPAU copy", true);

FrameAvailableCode SyncDecoder::renderToBmps(vector<BitmapPtr>& pBmps, float timeWanted)
{
    AVG_ASSERT(m_State == DECODING);
    ScopeTimer timer(RenderToBmpProfilingZone);
    AVFrame frame;
    FrameAvailableCode frameAvailable;
    if (timeWanted == -1) {
        readFrame(frame);
        frameAvailable = FA_NEW_FRAME;
    } else {
        frameAvailable = readFrameForTime(frame, timeWanted);
    }
    if (!m_bVideoEOF && frameAvailable == FA_NEW_FRAME) {
        if (pixelFormatIsPlanar(m_PF)) {
            ScopeTimer timer(CopyImageProfilingZone);
            for (unsigned i = 0; i < pBmps.size(); ++i) {
                copyPlaneToBmp(pBmps[i], frame.data[i], frame.linesize[i]);
            }
        } else {
            convertFrameToBmp(frame, pBmps[0]);
        }
        return FA_NEW_FRAME;
    }
    // TODO: clean this up.
    return FA_USE_LAST_FRAME;
}

void SyncDecoder::throwAwayFrame(float timeWanted)
{
    AVG_ASSERT(m_State == DECODING);
    AVFrame frame;
    readFrameForTime(frame, timeWanted);
}

bool SyncDecoder::isEOF(StreamSelect stream) const
{
    AVG_ASSERT(m_State == DECODING);
    AVG_ASSERT(stream != SS_AUDIO);
    return m_bVideoEOF;
}

PixelFormat SyncDecoder::calcPixelFormat(bool bUseYCbCr)
{
    AVCodecContext const* pContext = getCodecContext();
    if (bUseYCbCr) {
        switch(pContext->pix_fmt) {
            case PIX_FMT_YUV420P:
#ifdef AVG_ENABLE_VDPAU
            case PIX_FMT_VDPAU_H264:
            case PIX_FMT_VDPAU_MPEG1:
            case PIX_FMT_VDPAU_MPEG2:
            case PIX_FMT_VDPAU_WMV3:
            case PIX_FMT_VDPAU_VC1:
#endif
                return YCbCr420p;
            case PIX_FMT_YUVJ420P:
                return YCbCrJ420p;
            case PIX_FMT_YUVA420P:
                return YCbCrA420p;
            default:
                break;
        }
    }
    bool bAlpha = (pContext->pix_fmt == PIX_FMT_BGRA ||
            pContext->pix_fmt == PIX_FMT_YUVA420P);
    return BitmapLoader::get()->getDefaultPixelFormat(bAlpha);
}

static ProfilingZoneID ConvertImageLibavgProfilingZone(
        "FFMpeg: colorspace conv (libavg)", true);
static ProfilingZoneID ConvertImageSWSProfilingZone(
        "FFMpeg: colorspace conv (SWS)", true);
static ProfilingZoneID SetAlphaProfilingZone("FFMpeg: set alpha channel", true);

void SyncDecoder::convertFrameToBmp(AVFrame& frame, BitmapPtr pBmp)
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
            AVG_ASSERT_MSG(false, (string("SyncDecoder: Dest format ") +
                    toString(pBmp->getPixelFormat()) + " not supported.").c_str());
            destFmt = PIX_FMT_BGRA;
    }
    AVCodecContext const* pContext = getCodecContext();
    if (destFmt == PIX_FMT_BGRA && (pContext->pix_fmt == PIX_FMT_YUV420P || 
                pContext->pix_fmt == PIX_FMT_YUVJ420P))
    {
        ScopeTimer timer(ConvertImageLibavgProfilingZone);
        BitmapPtr pBmpY(new Bitmap(pBmp->getSize(), I8, frame.data[0],
                    frame.linesize[0], false));
        BitmapPtr pBmpU(new Bitmap(pBmp->getSize(), I8, frame.data[1],
                    frame.linesize[1], false));
        BitmapPtr pBmpV(new Bitmap(pBmp->getSize(), I8, frame.data[2],
                    frame.linesize[2], false));
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
            sws_scale(m_pSwsContext, frame.data, frame.linesize, 0, 
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
       
PixelFormat SyncDecoder::getPixelFormat() const
{
    AVG_ASSERT(m_State != CLOSED);
    return m_PF;
}

int SyncDecoder::getNumFrames() const
{
    AVG_ASSERT(m_State != CLOSED);
    // This is broken for some videos.
    int numFrames =  int(m_pVStream->nb_frames);
    if (numFrames > 0) {
        return numFrames;
    } else {
        return int(getDuration() * calcStreamFPS());
    }
}

FrameAvailableCode SyncDecoder::readFrameForTime(AVFrame& frame, float timeWanted)
{
    AVG_ASSERT(m_State == DECODING);
    AVG_ASSERT(timeWanted != -1);
    float timePerFrame = 1.0f/m_FPS;
    if (!m_bVideoSeekDone && timeWanted-m_LastVideoFrameTime < 0.5f*timePerFrame) {
        // The last frame is still current. Display it again.
        return FA_USE_LAST_FRAME;
    } else {
        bool bInvalidFrame = true;
        while (bInvalidFrame && !m_bVideoEOF) {
            float frameTime = readFrame(frame);
            bInvalidFrame = frameTime-timeWanted < -0.5f*timePerFrame;
        }
    }
    if (m_bVideoSeekDone) {
        m_bVideoSeekDone = false;
    }
    return FA_NEW_FRAME;
}

static ProfilingZoneID DecodeProfilingZone("FFMpeg: decode", true);

float SyncDecoder::readFrame(AVFrame& frame)
{
    AVG_ASSERT(m_State == DECODING);
    ScopeTimer timer(DecodeProfilingZone); 

    if (m_bEOFPending) {
        m_bVideoEOF = true;
        m_bEOFPending = false;
        return m_LastVideoFrameTime;
    }
    AVCodecContext* pContext = getCodecContext();
    int bGotPicture = 0;
    AVPacket* pPacket = 0;
    float frameTime = -32768;
    bool bDone = false;
    while (!bGotPicture && !bDone) {
        int seqNum;
        m_pDemuxer->isSeekDone(m_VStreamIndex, seqNum);
        pPacket = m_pDemuxer->getPacket(m_VStreamIndex);
        m_bFirstPacket = false;
        if (pPacket) {
#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(52, 31, 0)
            int len1 = avcodec_decode_video2(pContext, &frame, &bGotPicture, pPacket);
#else
            int len1 = avcodec_decode_video(pContext, &frame, &bGotPicture, pPacket->data,
                    pPacket->size);
#endif
            if (len1 > 0) {
                AVG_ASSERT(len1 == pPacket->size);
            }
            if (bGotPicture) {
                frameTime = getFrameTime(pPacket->dts);
            }
            av_free_packet(pPacket);
            delete pPacket;
        } else {
            // No more packets -> EOF. Decode the last data we got.
#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(52, 31, 0)
            AVPacket packet;
            packet.data = 0;
            packet.size = 0;
            avcodec_decode_video2(pContext, &frame, &bGotPicture, &packet);
#else
            avcodec_decode_video(pContext, &frame, &bGotPicture, 0, 0);
#endif
            if (bGotPicture) {
                m_bEOFPending = true;
            } else {
                m_bVideoEOF = true;
            }
            // We don't have a timestamp for the last frame, so we'll
            // calculate it based on the frame before.
            frameTime = m_LastVideoFrameTime+1.0f/m_FPS;
            m_LastVideoFrameTime = frameTime;
        }
        if (m_bVideoEOF) {
            bDone = true;
        }
    }
    AVG_ASSERT(frameTime != -32768);
    return frameTime;
/*
    cerr << "coded_picture_number: " << frame.coded_picture_number <<
            ", display_picture_number: " << frame.display_picture_number <<
            ", pts: " << frame.pts << endl;

    cerr << "key_frame: " << frame.key_frame << 
           ", pict_type: " << frame.pict_type << endl;
    AVFrac spts = m_pVStream->pts;
    cerr << "Stream.pts: " << spts.val + float(spts.num)/spts.den << endl;
*/
}

float SyncDecoder::getFrameTime(long long dts)
{
    if (dts == (long long)AV_NOPTS_VALUE) {
        dts = 0;
    }
    if (m_VideoStartTimestamp == -1) {
        m_VideoStartTimestamp = dts;
    }
    float frameTime;
    if (m_bUseStreamFPS || m_bVideoSeekDone) {
        frameTime = float(dts-m_VideoStartTimestamp)/m_TimeUnitsPerSecond;
    } else {
        if (m_LastVideoFrameTime == -1) {
            frameTime = 0;
        } else {
            frameTime = m_LastVideoFrameTime + 1.0f/m_FPS;
        }
    }
    m_LastVideoFrameTime = frameTime;
    return frameTime;
}

float SyncDecoder::calcStreamFPS() const
{
    return (float(m_pVStream->r_frame_rate.num)/m_pVStream->r_frame_rate.den);
}

string SyncDecoder::getStreamPF() const
{
    AVCodecContext const* pCodec = getCodecContext();
    AVPixelFormat pf = pCodec->pix_fmt;
    const char* psz = av_get_pix_fmt_name(pf);
    string s;
    if (psz) {
        s = psz;
    }
    return s;
}

AVCodecContext const* SyncDecoder::getCodecContext() const
{
    return m_pVStream->codec;
}

AVCodecContext* SyncDecoder::getCodecContext()
{
    return m_pVStream->codec;
}

}

