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
#ifndef _WIN32
#include <unistd.h>
#endif
#include <errno.h>

using namespace std;
using namespace boost;

#define SAMPLE_BUFFER_SIZE ((AVCODEC_MAX_AUDIO_FRAME_SIZE*3))

namespace avg {

bool FFMpegDecoder::m_bInitialized = false;
mutex FFMpegDecoder::s_OpenMutex;

FFMpegDecoder::FFMpegDecoder ()
    : m_pDemuxer(0),
      m_pFormatContext(0),
      m_pVStream(0),
      m_pAStream(0),
      m_VStreamIndex(-1),
      m_AStreamIndex(-1),
#ifdef AVG_ENABLE_SWSCALE
      m_pSwsContext(0),
#endif
      m_pAudioResampleContext(0),
      m_pPacketData(0),
      m_Size(0,0),
      m_StartTimestamp(-1),
      m_LastFrameTime(-1000),
      m_bUseStreamFPS(true),
      m_FPS(0)
{
    ObjectCounter::get()->incRef(&typeid(*this));
    initVideoSupport();

//    m_pRenderToBmpProfilingZone = new ProfilingZone("FFMpeg: renderToBmp");
//    m_pConvertImageProfilingZone = new ProfilingZone("FFMpeg: convert image");
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

int openCodec(AVFormatContext *formatContext, int streamIndex)
{
    AVCodecContext *enc;
#if LIBAVFORMAT_BUILD < ((49<<16)+(0<<8)+0)
    enc = &(formatContext->streams[streamIndex]->codec);
#else
    enc = formatContext->streams[streamIndex]->codec;
#endif
//    enc->debug = 0x0001; // see avcodec.h

    AVCodec * codec = avcodec_find_decoder(enc->codec_id);
    if (!codec ||
        avcodec_open(enc, codec) < 0)
    {
        return -1;
    }
    return 0;
}


void FFMpegDecoder::open(const std::string& sFilename, YCbCrMode ycbcrMode,
        bool bThreadedDemuxer)
{
    mutex::scoped_lock Lock(s_OpenMutex);
    m_bEOF = false;
    m_bEOFPending = false;
    m_StartTimestamp = -1;
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
    
    // Find audio and video streams in the file
    m_VStreamIndex = -1;
    m_AStreamIndex = -1;
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
            case CODEC_TYPE_AUDIO:
                if (m_AStreamIndex < 0)
                    m_AStreamIndex = i;
                break;
            default:
                break;
        }
    }
    assert(!m_pDemuxer);
//    dump_format(m_pFormatContext, 0, m_sFilename.c_str(), 0);
//    dump_stream_info(m_pFormatContext);
    if (m_VStreamIndex < 0 && m_AStreamIndex < 0) {
        throw Exception(AVG_ERR_VIDEO_INIT_FAILED, 
                sFilename + " does not contain any audio or video streams.");
    }
    
    // Create demuxer to handle streams
    if (bThreadedDemuxer) { 
        m_pDemuxer = new AsyncDemuxer(m_pFormatContext);
    } else {
        m_pDemuxer = new FFMpegDemuxer(m_pFormatContext);
    }
    
    // Enable video stream demuxing
    if(m_VStreamIndex >= 0)
    {
        m_pVStream = m_pFormatContext->streams[m_VStreamIndex];
        m_pDemuxer->enableStream(m_VStreamIndex);
        
        // Set video parameters
        m_TimeUnitsPerSecond = 1.0/av_q2d(m_pVStream->time_base);
        if (m_bUseStreamFPS) {
            m_FPS = calcStreamFPS();
        }
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
    
    // Enable audio stream demuxing
    if(m_AStreamIndex >= 0)
    {
        m_pAStream = m_pFormatContext->streams[m_AStreamIndex];
        m_pDemuxer->enableStream(m_AStreamIndex);
        
        m_AudioPacket = 0;
        m_AudioPacketData = 0;
        m_AudioPacketSize = 0;
        
        m_SampleBufferSize = SAMPLE_BUFFER_SIZE;
        m_pSampleBuffer = new char[SAMPLE_BUFFER_SIZE];
        m_SampleBufferStart = 0;
        m_SampleBufferEnd = 0;

        m_ResampleBufferSize = 0;
        m_pResampleBuffer = 0;
        m_ResampleBufferStart = 0;
        m_ResampleBufferEnd = 0;
        
        m_AudioClock = 0;
        m_AudioClockStart = 0;
        //if(m_pAStream->start_time != AV_NOPTS_VALUE)
        //    m_AudioClockStart = 
        //            (long long)(1000.0 * av_q2d(m_pAStream->time_base) * m_pAStream->start_time);
    }
    
    // Open codecs for audio and video
    if ((m_VStreamIndex != -1 && openCodec(m_pFormatContext, m_VStreamIndex) == -1) ||
        (m_AStreamIndex != -1 && openCodec(m_pFormatContext, m_AStreamIndex) == -1))
    {
    	throw Exception(AVG_ERR_VIDEO_INIT_FAILED, 
    	                sFilename + ": could not open codec (?!).");
    }
} 

void FFMpegDecoder::close() 
{
    mutex::scoped_lock Lock(s_OpenMutex);
    mutex::scoped_lock Lock2(m_AudioMutex);
    AVG_TRACE(Logger::MEMORY, "Closing " << m_sFilename);
    
    delete m_pDemuxer;
    m_pDemuxer = 0;
    
    // Close audio and video codecs
    if(m_pVStream)
    {
        avcodec_close(m_pVStream->codec);
        if (!m_bFirstPacket) {
            av_free_packet(m_pPacket);
            delete m_pPacket;
        }
        m_pPacketData = 0;
        m_pVStream = 0;
        m_VStreamIndex = -1;
    }

    if(m_pAStream)
    {
        avcodec_close(m_pAStream->codec);
        if (m_AudioPacket) {
            av_free_packet(m_AudioPacket);
            delete m_AudioPacket;
            m_AudioPacket = 0;
        }
        if (m_pSampleBuffer) {
            delete m_pSampleBuffer;
            m_pSampleBuffer = 0;
        }
        if(m_pResampleBuffer) {
            delete m_pResampleBuffer;
            m_pResampleBuffer = 0;
        }
        if(m_pAudioResampleContext) {
            audio_resample_close(m_pAudioResampleContext);
            m_pAudioResampleContext = 0;
        }
        
        m_AudioPacketData = 0;
        m_AudioPacketSize = 0;
        
        m_SampleBufferStart = 0;
        m_SampleBufferEnd = 0;
        m_SampleBufferSize = 0;

        m_ResampleBufferStart = 0;
        m_ResampleBufferEnd = 0;
        m_ResampleBufferSize = 0;
        
        m_AudioClock = 0;
        m_AudioClockStart = 0;
        
        m_pAStream = 0;
        m_AStreamIndex = -1;
    }
    
    av_close_input_file(m_pFormatContext);
    m_pFormatContext = 0;
    
#ifdef AVG_ENABLE_SWSCALE
    if(m_pSwsContext) {
        sws_freeContext(m_pSwsContext);
        m_pSwsContext = 0;
    }
#endif
}

void FFMpegDecoder::seek(int DestFrame) 
{
//    cerr << "seek to " << DestFrame << endl;
    if (m_bFirstPacket) {
        AVFrame Frame;
        long long FrameTime;
        readFrame(Frame, FrameTime);
    }
    m_pDemuxer->seek(DestFrame, m_StartTimestamp, m_VStreamIndex);
    if (m_bUseStreamFPS) {
        m_LastFrameTime = -1000;
    } else {
        m_LastFrameTime = (long long)((DestFrame-1)*(1000/m_FPS));
    }
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
    return m_FPS;
}

void FFMpegDecoder::setFPS(double FPS)
{
    m_bUseStreamFPS = (FPS == 0);
    if (FPS == 0) {
        m_FPS = calcStreamFPS();
    } else {
        m_FPS = FPS;
    }
}

FrameAvailableCode FFMpegDecoder::renderToBmp(BitmapPtr pBmp, long long TimeWanted)
{
//    ScopeTimer Timer(*m_pRenderToBmpProfilingZone);
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


FrameAvailableCode FFMpegDecoder::renderToYCbCr420p(BitmapPtr pBmpY, BitmapPtr pBmpCb, 
        BitmapPtr pBmpCr, long long TimeWanted)
{
//    ScopeTimer Timer(*m_pRenderToBmpProfilingZone);
    AVFrame Frame;
    FrameAvailableCode FrameAvailable = readFrameForTime(Frame, TimeWanted);
    if (!m_bEOF && FrameAvailable == FA_NEW_FRAME) {
//        ScopeTimer Timer(*m_pConvertImageProfilingZone);
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

int FFMpegDecoder::copyRawAudio(unsigned char* buf, int size)
{
    int bytesWritten = min(m_SampleBufferEnd - m_SampleBufferStart, size);
    memcpy(buf, m_pSampleBuffer + m_SampleBufferStart, bytesWritten);
    
    m_SampleBufferStart += bytesWritten;
    
    if(m_SampleBufferStart == m_SampleBufferEnd)
    {
        m_SampleBufferStart = 0;
        m_SampleBufferEnd = 0;
        m_SampleBufferSize = SAMPLE_BUFFER_SIZE;
    }
    
    return bytesWritten;
}

int FFMpegDecoder::copyResampledAudio(unsigned char* buf, int size, int outputChannels, int outputRate)
{
    int bytesWritten = 0;
                        
    // If there is no buffered resampled data, resample some more
    if(m_ResampleBufferStart >= m_ResampleBufferEnd)
        resampleAudio(outputChannels, outputRate);
    
    // If we have some data in the resample buffer, copy it over
    if(m_ResampleBufferStart < m_ResampleBufferEnd)
    {
        bytesWritten = min(m_ResampleBufferEnd - m_ResampleBufferStart, size);
        memcpy(buf, m_pResampleBuffer + m_ResampleBufferStart, bytesWritten);
        
        m_ResampleBufferStart += bytesWritten;
        if(m_ResampleBufferStart >= m_ResampleBufferEnd)
        {
            m_ResampleBufferStart = 0;
            m_ResampleBufferEnd = 0;
        }
        
        if(m_SampleBufferStart == m_SampleBufferEnd)
        {
            m_SampleBufferStart = 0;
            m_SampleBufferEnd = 0;
            m_SampleBufferSize = SAMPLE_BUFFER_SIZE;
        }
    }
    
    return bytesWritten;
}

void FFMpegDecoder::resampleAudio(int channels, int rate)
{
    if (!m_pAudioResampleContext)
        m_pAudioResampleContext = audio_resample_init(
                channels, m_pAStream->codec->channels, 
                rate, m_pAStream->codec->sample_rate);
    
    int inputSamples = 
        (m_SampleBufferEnd - m_SampleBufferStart) / 
        (2 * m_pAStream->codec->channels);
    
    if(!m_pResampleBuffer)
    {
        m_ResampleBufferSize = (int)(4 * inputSamples * 
            ((float)rate / (float)m_pAStream->codec->sample_rate) + 16);
        m_pResampleBuffer = new char[m_ResampleBufferSize];
    }
    
    int outputSamples = audio_resample(m_pAudioResampleContext,
            (int16_t*)m_pResampleBuffer,
            (int16_t*)(m_pSampleBuffer + m_SampleBufferStart), 
            inputSamples);
    
    // Adjust buffer pointers
    m_ResampleBufferEnd += outputSamples * 2 * channels;
    m_SampleBufferStart += inputSamples * 2 * m_pAStream->codec->channels;
}

int FFMpegDecoder::decodeAudio()
{
    // Save current size of the audio buffer
    int lastSampleBufferSize = m_SampleBufferSize;
    
    // Decode some data from packet into the audio buffer
    int packetBytesDecoded = avcodec_decode_audio2(
            m_pAStream->codec, 
            (int16_t*)(m_pSampleBuffer + m_SampleBufferEnd),
            &m_SampleBufferSize, 
            m_AudioPacketData,
            m_AudioPacketSize);
    
    // Skip frame on error
    if (packetBytesDecoded < 0)
        return -1;
    
    // Did not get any data, try again
    if (packetBytesDecoded == 0)
        return 0;
    
    // Adjust audio buffer pointers
    m_SampleBufferEnd += m_SampleBufferSize;
    m_SampleBufferSize = lastSampleBufferSize - m_SampleBufferSize;
                
    // Adjust packet data pointers
    m_AudioPacketData += packetBytesDecoded;
    m_AudioPacketSize -= packetBytesDecoded;
}

void FFMpegDecoder::fillAudioFrame(unsigned char* outputAudioBuffer,
        int outputAudioBufferSize, int outputChannels, int outputRate)
{
    mutex::scoped_lock Lock(m_AudioMutex);
    
    if(!m_pAStream)
        return;
    
    int packetBytesDecoded;
    int bytesProduced;

    while(1)
    {
        // While there is data left in the encoded packet, decode it
        while(m_AudioPacketSize > 0)
        {
            // While there is data left in the sample buffer, consume it
            while(m_SampleBufferStart < m_SampleBufferEnd)
            {
                // If the output format is different from the decoded format,
                // then convert it, else copy it over
                if (m_pAStream->codec->sample_rate != outputRate ||
                    m_pAStream->codec->channels != outputChannels)
                {
                    bytesProduced = copyResampledAudio(outputAudioBuffer,
                            outputAudioBufferSize, outputChannels, outputRate);
                }
                else
                {
                    bytesProduced = copyRawAudio(outputAudioBuffer,
                            outputAudioBufferSize);
                }
                
                outputAudioBuffer += bytesProduced;
                outputAudioBufferSize -= bytesProduced;

                m_AudioClock += (long long)(1000.0 * bytesProduced / 
                        (2 * outputChannels * outputRate));
                
                if(outputAudioBufferSize == 0)
                    return;
            }
            
            packetBytesDecoded = decodeAudio();
            
            // Skip frame on error
            if (packetBytesDecoded < 0)
                break;
            
            // Did not get any data, try again
            if (packetBytesDecoded == 0)
                continue;
        }
        
        // We have decoded all data in the packet, free it
        if(m_AudioPacket)
        {
            av_free_packet(m_AudioPacket);
            delete m_AudioPacket;
        }
        
        // Get a new packet from the audio stream
        m_AudioPacket = m_pDemuxer->getPacket(m_AStreamIndex);
        
        if(!m_AudioPacket)
            return;
        
        //if(m_AudioPacket->dts != AV_NOPTS_VALUE)
            m_AudioClock = (long long)(1000.0 * av_q2d(m_pAStream->time_base) * 
                    m_AudioPacket->dts - m_AudioClockStart);
        
        // Initialize packet data pointers
        m_AudioPacketData = m_AudioPacket->data;
        m_AudioPacketSize = m_AudioPacket->size;
    }
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
//        ScopeTimer Timer(*m_pConvertImageProfilingZone);
#ifdef AVG_ENABLE_SWSCALE
        if (!m_pSwsContext) {
            m_pSwsContext = sws_getContext(enc->width, enc->height, enc->pix_fmt,
                        enc->width, enc->height, DestFmt, SWS_BICUBIC, 
                        NULL, NULL, NULL);
            if(!m_pSwsContext) {
                AVG_TRACE(Logger::ERROR, "FFMpegDecoder: sws initialization failed.");
            }
        }
        sws_scale(m_pSwsContext, Frame.data, Frame.linesize, 0, 
            enc->height, DestPict.data, DestPict.linesize);
#else
        int rc = img_convert(&DestPict, DestFmt,
                (AVPicture*)&Frame, enc->pix_fmt,
                enc->width, enc->height);
        if (rc != 0) {
            AVG_TRACE(Logger::ERROR, "FFFMpegDecoder: img_convert failed.");
        }
#endif
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

    // If this video has audio, sync to that instead of the requested time
    if(m_pAStream)
        TimeWanted = m_AudioClock;
/*
    bool bDebug = (m_sFilename == "/home/uzadow/wos_videos/c-wars/thumbs/cwars-scene3.avi");
    if (bDebug) {
        cerr << m_sFilename << " readFrameForTime " << TimeWanted 
                << ", LastFrameTime= " << m_LastFrameTime << ", diff= " 
                << TimeWanted-m_LastFrameTime << endl;
    }
*/    
    if (TimeWanted == -1) {
        readFrame(Frame, FrameTime);
    } else {
        double TimePerFrame = 1000/m_FPS;
        if (TimeWanted-m_LastFrameTime < 0.5*TimePerFrame) {
/*            
            if (bDebug) {
                cerr << "   LastFrameTime = " << m_LastFrameTime << ", display again."
                        <<  endl;
            }
*/            
            // The last frame is still current. Display it again.
            return FA_USE_LAST_FRAME;
        } else {
            while (FrameTime-TimeWanted < -0.5*TimePerFrame && !m_bEOF) {
                readFrame(Frame, FrameTime);
/*                
                if (bDebug) {
                    cerr << "   readFrame returned time " << FrameTime << "." <<  endl;
                }
*/                
            }
/*
            if (bDebug) {
                cerr << "  frame ok." << endl;
            }
*/            
        }
    }
    return FA_NEW_FRAME;
}

void FFMpegDecoder::readFrame(AVFrame& Frame, long long& FrameTime)
{
    assert(m_pDemuxer);
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
                    FrameTime = m_LastFrameTime+(long long)(1000.0/m_FPS);
                    m_LastFrameTime = FrameTime;
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
    long long FrameTime;
    if (m_bUseStreamFPS) {
        FrameTime = (long long)((1000*pPacket->dts)/m_TimeUnitsPerSecond)-m_StartTimestamp;
    } else {
        if (m_LastFrameTime == -1000) {
            FrameTime = 0;
        } else {
            FrameTime = m_LastFrameTime + (long long)(1000.0/m_FPS);
        }
    }
    m_LastFrameTime = FrameTime;
    return FrameTime;
}

double FFMpegDecoder::calcStreamFPS()
{
#if LIBAVFORMAT_BUILD < ((49<<16)+(0<<8)+0)
    return m_pVStream->r_frame_rate;
#else
    return (m_pVStream->r_frame_rate.num/m_pVStream->r_frame_rate.den);
#endif 
}

}

