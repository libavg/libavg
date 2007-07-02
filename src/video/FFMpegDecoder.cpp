//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
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

#include "FFMpegDecoder.h"
#include "AsyncDemuxer.h"
#include "FFMpegDemuxer.h"

#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/ScopeTimer.h"
#include "../base/ObjectCounter.h"

#include "../graphics/Filterflipuv.h"

#include <iostream>
#include <sstream>
#include <unistd.h>
#include <errno.h>

using namespace std;
using namespace boost;

namespace avg {

bool FFMpegDecoder::m_bInitialized = false;
mutex FFMpegDecoder::s_OpenMutex;   

FFMpegDecoder::FFMpegDecoder ()
    : m_pDemuxer(0),
      m_pFormatContext(0),
      m_pVStream(0),
      m_pPacketData(0),
      m_Size(0,0),
      m_StartTimestamp(-1),
      m_LastFrameTime(-1000)
{
    ObjectCounter::get()->incRef(&typeid(*this));
    initVideoSupport();
}

FFMpegDecoder::~FFMpegDecoder ()
{
    if (m_pFormatContext) {
        close();
    }
    ObjectCounter::get()->decRef(&typeid(*this));
}


void avcodecError(const string & sFilename, int err)
{
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
            s << sFilename << ": Error while opening file (Num:" << err << ")";
            throw Exception(AVG_ERR_VIDEO_INIT_FAILED, s.str());
    }
}


void dump_stream_info(AVFormatContext *s)
{
    if (s->track != 0)
        fprintf(stderr, "  Track: %d\n", s->track);
    if (s->title[0] != '\0')
        fprintf(stderr, "  Title: %s\n", s->title);
    if (s->author[0] != '\0')
        fprintf(stderr, "  Author: %s\n", s->author);
    if (s->album[0] != '\0')
        fprintf(stderr, "  Album: %s\n", s->album);
    if (s->year != 0)
        fprintf(stderr, "  Year: %d\n", s->year);
    if (s->genre[0] != '\0')
        fprintf(stderr, "  Genre: %s\n", s->genre);
}


void FFMpegDecoder::open(const std::string& sFilename, YCbCrMode ycbcrMode,
        bool bThreadedDemuxer)
{
    mutex::scoped_lock Lock(s_OpenMutex);
    m_bEOF = false;
    m_bEOFPending = false;
    AVFormatParameters params;
    int err;
    m_sFilename = sFilename;

    AVG_TRACE(Logger::MEMORY, "Opening " << sFilename);
    memset(&params, 0, sizeof(params));

    err = av_open_input_file(&m_pFormatContext, sFilename.c_str(), 
            0, 0, &params);
    if (err < 0) {
        m_sFilename = "";
        avcodecError(sFilename, err);
    }
    
    err = av_find_stream_info(m_pFormatContext);
    if (err < 0) {
        throw Exception(AVG_ERR_VIDEO_INIT_FAILED, 
                sFilename + ": Could not find codec parameters.");
    }
//    dump_format(m_pFormatContext, 0, sFilename.c_str(), 0);

    av_read_play(m_pFormatContext);
    
    m_VStreamIndex = -1;
    for(unsigned i = 0; i < m_pFormatContext->nb_streams; i++) {
#if LIBAVFORMAT_BUILD < ((49<<16)+(0<<8)+0)
        AVCodecContext *enc = &m_pFormatContext->streams[i]->codec;
#else         
        AVCodecContext *enc = m_pFormatContext->streams[i]->codec;
#endif
        switch(enc->codec_type) {
            case CODEC_TYPE_VIDEO:
                if (m_VStreamIndex < 0)
                    m_VStreamIndex = i;
                break;
            default:
                break;
        }
    }
    assert(!m_pDemuxer);
//    dump_format(m_pFormatContext, 0, m_sFilename.c_str(), 0);
//    dump_stream_info(m_pFormatContext);
    if (m_VStreamIndex < 0) {
        throw Exception(AVG_ERR_VIDEO_INIT_FAILED, 
                sFilename + " does not contain any video streams.");
    }                
    if (bThreadedDemuxer) { 
        m_pDemuxer = new AsyncDemuxer(m_pFormatContext);
    } else {
        m_pDemuxer = new FFMpegDemuxer(m_pFormatContext);
    }
    m_pDemuxer->enableStream(m_VStreamIndex);
    AVCodecContext *enc;
#if LIBAVFORMAT_BUILD < ((49<<16)+(0<<8)+0)
    enc = &(m_pFormatContext->streams[m_VStreamIndex]->codec);
#else
    enc = m_pFormatContext->streams[m_VStreamIndex]->codec;
#endif
//    enc->debug = 0x0001; // see avcodec.h

    AVCodec * codec = avcodec_find_decoder(enc->codec_id);
    if (!codec ||
        avcodec_open(enc, codec) < 0)
    {
        throw Exception(AVG_ERR_VIDEO_INIT_FAILED, 
                sFilename + ": could not open codec (?!).");
    }                
    m_pVStream = m_pFormatContext->streams[m_VStreamIndex];
    m_TimeUnitsPerSecond = 1.0/av_q2d(m_pVStream->time_base);
    m_TimePerFrame = (long long)(1000.0/getFPS());
#if LIBAVFORMAT_BUILD < ((49<<16)+(0<<8)+0)
    m_Size = IntPoint(m_pVStream->codec.width, m_pVStream->codec.height);
#else
    m_Size = IntPoint(m_pVStream->codec->width, m_pVStream->codec->height);
#endif
    m_bFirstPacket = true;
    m_PacketLenLeft = 0;
    m_sFilename = sFilename;
    m_LastFrameTime = -1000;
    m_PF = calcPixelFormat(ycbcrMode);
} 

void FFMpegDecoder::close() 
{
    mutex::scoped_lock Lock(s_OpenMutex);
    AVG_TRACE(Logger::MEMORY, "Closing " << m_sFilename);
    delete m_pDemuxer;
    m_pDemuxer = 0;
    AVCodecContext * enc;
#if LIBAVFORMAT_BUILD < ((49<<16)+(0<<8)+0)
    enc = &(m_pFormatContext->streams[m_VStreamIndex]->codec);
#else
    enc = m_pFormatContext->streams[m_VStreamIndex]->codec;
#endif
    if (!m_bFirstPacket) {
        av_free_packet(m_pPacket);
        delete m_pPacket;
    }
    m_pPacketData = 0;
    avcodec_close(enc);
    m_pVStream = 0;
    av_close_input_file(m_pFormatContext);
    m_pFormatContext = 0;
}

void FFMpegDecoder::seek(int DestFrame) 
{
    if (m_bFirstPacket) {
        AVFrame Frame;
        long long FrameTime;
        readFrame(Frame, FrameTime);
    }
    m_pDemuxer->seek(DestFrame, m_StartTimestamp, m_VStreamIndex);
    m_LastFrameTime = -1000;
    m_bEOF = false;
}

IntPoint FFMpegDecoder::getSize()
{
    return m_Size;
}

int FFMpegDecoder::getNumFrames()
{
    // This is broken for some videos, but the code here is correct.
    // So fix ffmpeg :-).
#if LIBAVFORMAT_BUILD < ((49<<16)+(0<<8)+0)
    return m_pVStream->r_frame_rate*(m_pVStream->duration/AV_TIME_BASE);
#else
    double timeUnitsPerSecond = 1/av_q2d(m_pVStream->time_base);
    return (m_pVStream->r_frame_rate.num/m_pVStream->r_frame_rate.den)*
            int(m_pVStream->duration/timeUnitsPerSecond);
#endif 
}

double FFMpegDecoder::getFPS()
{
#if LIBAVFORMAT_BUILD < ((49<<16)+(0<<8)+0)
    return m_pVStream->r_frame_rate;
#else
    return (m_pVStream->r_frame_rate.num/m_pVStream->r_frame_rate.den);
#endif 
}

static ProfilingZone RenderToBmpProfilingZone("      FFMpeg: renderToBmp");

FrameAvailableCode FFMpegDecoder::renderToBmp(BitmapPtr pBmp, long long TimeWanted)
{
    ScopeTimer Timer(RenderToBmpProfilingZone);
    AVFrame Frame;
    FrameAvailableCode FrameAvailable = readFrameForTime(Frame, TimeWanted);
    if (!m_bEOF && FrameAvailable == FA_NEW_FRAME) {
        convertFrameToBmp(Frame, pBmp);
        return FA_NEW_FRAME;
    }
    return FA_USE_LAST_FRAME;
}

void copyPlaneToBmp(BitmapPtr pBmp, unsigned char * pData, int Stride)
{
    unsigned char * pSrc=pData;
    unsigned char * pDest= pBmp->getPixels();
    int DestStride = pBmp->getStride();
    int Height = pBmp->getSize().y;
    int Width = pBmp->getSize().x;
    for (int y=0; y<Height; y++) {
        memcpy(pDest, pSrc, Width);
        pSrc+=Stride;
        pDest+=DestStride;
    }

}

static ProfilingZone ConvertImageProfilingZone("        FFMpeg: convert image");

FrameAvailableCode FFMpegDecoder::renderToYCbCr420p(BitmapPtr pBmpY, BitmapPtr pBmpCb, 
        BitmapPtr pBmpCr, long long TimeWanted)
{
    ScopeTimer Timer(RenderToBmpProfilingZone);
    AVFrame Frame;
    FrameAvailableCode FrameAvailable = readFrameForTime(Frame, TimeWanted);
    if (!m_bEOF && FrameAvailable == FA_NEW_FRAME) {
        ScopeTimer Timer(ConvertImageProfilingZone);
        copyPlaneToBmp(pBmpY, Frame.data[0], Frame.linesize[0]);
        copyPlaneToBmp(pBmpCb, Frame.data[1], Frame.linesize[1]);
        copyPlaneToBmp(pBmpCr, Frame.data[2], Frame.linesize[2]);
        return FA_NEW_FRAME;
    }
    return FA_USE_LAST_FRAME;
}
        
long long FFMpegDecoder::getCurFrameTime()
{
    return m_LastFrameTime;
}

bool FFMpegDecoder::isEOF()
{
    return m_bEOF;
}

PixelFormat FFMpegDecoder::calcPixelFormat(YCbCrMode ycbcrMode)
{
#if LIBAVFORMAT_BUILD < ((49<<16)+(0<<8)+0)
        AVCodecContext *enc = &m_pVStream->codec;
#else
        AVCodecContext *enc = m_pVStream->codec;
#endif
    if (ycbcrMode == OGL_SHADER) {
        switch(enc->pix_fmt) {
            case PIX_FMT_YUV420P:
                return YCbCr420p;
            case PIX_FMT_YUVJ420P:
                return YCbCrJ420p;
            default:
                break;
        }
    }
    if ((ycbcrMode == OGL_MESA || ycbcrMode == OGL_APPLE) &&
         enc->pix_fmt == PIX_FMT_YUV420P) 
    {
        return YCbCr422;
    }
    if (enc->pix_fmt == PIX_FMT_RGBA32) {
        return B8G8R8A8;
    }
    return B8G8R8X8;
}

static ProfilingZone ImgConvertProfilingZone("        FFMpeg: img_convert");
        
void FFMpegDecoder::convertFrameToBmp(AVFrame& Frame, BitmapPtr pBmp)
{
    AVPicture DestPict;
    unsigned char * pDestBits = pBmp->getPixels();
    DestPict.data[0] = pDestBits;
    DestPict.linesize[0] = pBmp->getStride();
    int DestFmt;
    switch(pBmp->getPixelFormat()) {
        case R8G8B8X8:
        case R8G8B8A8:
            // XXX: Unused and broken.
            DestFmt = PIX_FMT_RGBA32;
            break;
        case B8G8R8X8:
        case B8G8R8A8:
            DestFmt = PIX_FMT_RGBA32;
            break;
        case R8G8B8:
            DestFmt = PIX_FMT_RGB24;
            break;
        case B8G8R8:
            DestFmt = PIX_FMT_BGR24;
            break;
        case YCbCr422:
            DestFmt = PIX_FMT_YUV422;
            break;
        default:
            AVG_TRACE(Logger::ERROR, "FFMpegDecoder: Dest format " 
                    << pBmp->getPixelFormatString() << " not supported.");
            assert(false);
    }
#if LIBAVFORMAT_BUILD < ((49<<16)+(0<<8)+0)
    AVCodecContext *enc = &m_pVStream->codec;
#else
    AVCodecContext *enc = m_pVStream->codec;
#endif
    {
        ScopeTimer Timer(ImgConvertProfilingZone);
        int rc = img_convert(&DestPict, DestFmt,
                (AVPicture*)&Frame, enc->pix_fmt,
                enc->width, enc->height);
        if (rc != 0) {
            AVG_TRACE(Logger::ERROR, "FFFMpegDecoder: img_convert failed.");
        }
    }
}
       
PixelFormat FFMpegDecoder::getPixelFormat()
{
    return m_PF;
}

void FFMpegDecoder::initVideoSupport()
{
    if (!m_bInitialized) {
        av_register_all();
        m_bInitialized = true;
        // Tune libavcodec console spam.
//        av_log_set_level(AV_LOG_DEBUG);
        av_log_set_level(AV_LOG_QUIET);
    }
}

FrameAvailableCode FFMpegDecoder::readFrameForTime(AVFrame& Frame, long long TimeWanted)
{
    // XXX: This code is sort-of duplicated in AsyncVideoDecoder::getBmpsForTime()
    long long FrameTime = -1000;
    cerr << "readFrameForTime " << TimeWanted << ", LastFrameTime= " << m_LastFrameTime << ", diff= " << TimeWanted-m_LastFrameTime << endl;
    if (TimeWanted == -1) {
        readFrame(Frame, FrameTime);
    } else {
        if (TimeWanted-m_LastFrameTime < 0.5*m_TimePerFrame) {
            cerr << "   LastFrameTime = " << m_LastFrameTime << ", display again." <<  endl;
            // The last frame is still current. Display it again.
            return FA_USE_LAST_FRAME;
        } else {
            while (FrameTime-TimeWanted < -0.5*m_TimePerFrame && !m_bEOF) {
                readFrame(Frame, FrameTime);
//                cerr << "   readFrame returned time " << FrameTime << "." <<  endl;
            }
//            cerr << "  frame ok." << endl;
        }
    }
    m_LastFrameTime = FrameTime;
    return FA_NEW_FRAME;
}

void FFMpegDecoder::readFrame(AVFrame& Frame, long long& FrameTime)
{
    if (m_bEOF) {
        return;
    }
    if (m_bEOFPending) {
        m_bEOF = true;
        m_bEOFPending = false;
        return;
    }
#if LIBAVFORMAT_BUILD < ((49<<16)+(0<<8)+0)
    AVCodecContext *enc = &m_pVStream->codec;
#else
    AVCodecContext *enc = m_pVStream->codec;
#endif
    if (enc->codec_id == CODEC_ID_RAWVIDEO) {
        AVPacket * pPacket;
        pPacket = m_pDemuxer->getPacket(m_VStreamIndex);
        if (!pPacket) {
            m_bEOF = true;
            return;
        }
        avpicture_fill((AVPicture*)&Frame, pPacket->data, 
                enc->pix_fmt, 
                enc->width, enc->height);
        av_free_packet(pPacket);
        delete pPacket;
    } else {
        int gotPicture = 0;
        while (!gotPicture) {
            if (m_PacketLenLeft <= 0) {
                if (!m_bFirstPacket) {
                    av_free_packet(m_pPacket);
                    delete m_pPacket;
                }
                m_bFirstPacket = false;
                m_pPacket = m_pDemuxer->getPacket(m_VStreamIndex);
                if (!m_pPacket) {
                    avcodec_decode_video(enc, &Frame, &gotPicture, NULL, 0);
                    if (gotPicture) {
                        m_bEOFPending = true;
                    } else {
                        m_bEOF = true;
                    }
                    // We don't have a timestamp for the last frame, so we'll
                    // calculate it based on the frame before.
                    FrameTime = m_LastFrameTime+m_TimePerFrame;
                    return;
                }
                m_PacketLenLeft = m_pPacket->size;
                m_pPacketData = m_pPacket->data;
            }
            int Len1 = avcodec_decode_video(enc, &Frame,
                    &gotPicture, m_pPacketData, m_PacketLenLeft);
            if (Len1 < 0) {
//                AVG_TRACE(Logger::WARNING, "Error decoding " <<
//                        m_sFilename);
                m_PacketLenLeft = 0;
            } else {
                m_pPacketData += Len1;
                m_PacketLenLeft -= Len1;
            }
        }
        FrameTime = getFrameTime(m_pPacket);
/*
        cerr << "coded_picture_number: " << Frame.coded_picture_number <<
                ", display_picture_number: " << Frame.display_picture_number <<
                ", pts: " << Frame.pts << endl;

        cerr << "key_frame: " << Frame.key_frame << 
               ", pict_type: " << Frame.pict_type << endl;
        AVFrac spts = m_pVStream->pts;
        cerr << "Stream.pts: " << spts.val + double(spts.num)/spts.den << endl;
*/    
    }
}

long long FFMpegDecoder::getFrameTime(AVPacket* pPacket)
{
    if (m_StartTimestamp == -1) {
        m_StartTimestamp = (long long)((1000*pPacket->dts)/m_TimeUnitsPerSecond);
    }
    int64_t PacketTimestamp = (pPacket->dts);
    return (long long)((1000*PacketTimestamp)/m_TimeUnitsPerSecond)-m_StartTimestamp;
}


}

