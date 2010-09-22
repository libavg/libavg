//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2008 Ulrich von Zadow
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
#include "../graphics/Filterfliprgba.h"

#include <iostream>
#include <sstream>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <errno.h>

using namespace std;
using namespace boost;

#define SAMPLE_BUFFER_SIZE ((AVCODEC_MAX_AUDIO_FRAME_SIZE*3))
#define VOLUME_FADE_SAMPLES 100

#if LIBAVFORMAT_BUILD < ((50<<16)+(0<<8)+0)
#define PIX_FMT_BGRA PIX_FMT_RGBA32
#define PIX_FMT_YUYV422 PIX_FMT_YUV422
#endif

namespace avg {

bool FFMpegDecoder::m_bInitialized = false;
mutex FFMpegDecoder::s_OpenMutex;

FFMpegDecoder::FFMpegDecoder ()
    : m_State(CLOSED),
      m_pFormatContext(0),
      m_PF(NO_PIXELFORMAT),
      m_pSwsContext(0),
      m_Size(0,0),
      m_bUseStreamFPS(true),
      m_AStreamIndex(-1),
      m_pSampleBuffer(0),
      m_pResampleBuffer(0),
      m_pAudioResampleContext(0),
      m_Volume(1.0),
      m_LastVolume(1.0),
      m_pDemuxer(0),
      m_pVStream(0),
      m_pAStream(0),
      m_VStreamIndex(-1),
      m_bFirstPacket(false),
      m_VideoStartTimestamp(-1),
      m_LastVideoFrameTime(-1000),
      m_FPS(0)
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
            s << "'" << sFilename <<  "': Error while opening file (Num:" << err << ")";
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

void FFMpegDecoder::open(const std::string& sFilename, bool bThreadedDemuxer)
{
    mutex::scoped_lock Lock(s_OpenMutex);
    m_bThreadedDemuxer = bThreadedDemuxer;
    m_bAudioEOF = false;
    m_bVideoEOF = false;
    m_bEOFPending = false;
    m_VideoStartTimestamp = -1;
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
                if (m_VStreamIndex < 0) {
                    m_VStreamIndex = i;
                }
                break;
            case CODEC_TYPE_AUDIO:
                // Ignore the audio stream if we're using sync demuxing. 
                if (m_AStreamIndex < 0 && bThreadedDemuxer) {
                    m_AStreamIndex = i;
                }
                break;
            default:
                break;
        }
    }
    AVG_ASSERT(!m_pDemuxer);
//    dump_format(m_pFormatContext, 0, m_sFilename.c_str(), 0);
//    dump_stream_info(m_pFormatContext);
    
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
        m_State = OPENED;
        m_pDemuxer->enableStream(m_VStreamIndex);
        
        // Set video parameters
        m_TimeUnitsPerSecond = 1.0/av_q2d(m_pVStream->time_base);
        if (m_bUseStreamFPS) {
            m_FPS = getNominalFPS();
        }
#if LIBAVFORMAT_BUILD < ((49<<16)+(0<<8)+0)
        m_Size = IntPoint(m_pVStream->codec.width, m_pVStream->codec.height);
#else
        m_Size = IntPoint(m_pVStream->codec->width, m_pVStream->codec->height);
#endif
        m_bFirstPacket = true;
        m_sFilename = sFilename;
        m_LastVideoFrameTime = -1000;
    
        int rc = openCodec(m_pFormatContext, m_VStreamIndex);
        if (rc == -1) {
            m_VStreamIndex = -1;
            char szBuf[256];
            avcodec_string(szBuf, sizeof(szBuf), m_pVStream->codec, 0);
            throw Exception(AVG_ERR_VIDEO_INIT_FAILED, 
                    sFilename + ": unsupported codec ("+szBuf+").");
        }
        m_PF = calcPixelFormat(true);
    }
    // Enable audio stream demuxing.
    if (m_AStreamIndex >= 0)
    {
        m_pAStream = m_pFormatContext->streams[m_AStreamIndex];
        
        m_AudioPacket = 0;
        m_AudioPacketData = 0;
        m_AudioPacketSize = 0;
        
        m_LastAudioFrameTime = 0;
        m_AudioStartTimestamp = 0;
        
        if (m_pAStream->start_time != AV_NOPTS_VALUE) {
            m_AudioStartTimestamp = (long long)(1000.0 * av_q2d(m_pAStream->time_base) 
                    * m_pAStream->start_time);
        }
        m_EffectiveSampleRate = (int)(m_pAStream->codec->sample_rate);
        int rc = openCodec(m_pFormatContext, m_AStreamIndex);
        if (rc == -1) {
            m_AStreamIndex = -1;
            char szBuf[256];
            avcodec_string(szBuf, sizeof(szBuf), m_pAStream->codec, 0);
            m_pAStream = 0; 
            AVG_TRACE(Logger::WARNING, 
                    sFilename + ": unsupported codec ("+szBuf+"). Disabling audio.");
        }
    }
    m_State = OPENED;
}

void FFMpegDecoder::startDecoding(bool bDeliverYCbCr, const AudioParams* pAP)
{
    AVG_ASSERT(m_State == OPENED);
    if (m_VStreamIndex >= 0) {
        m_PF = calcPixelFormat(bDeliverYCbCr);
    }
    bool bAudioEnabled = (pAP && m_bThreadedDemuxer);
    if (bAudioEnabled) {
        m_AP = *pAP;
    } else {
        m_AStreamIndex = -1;
        if (m_pAStream) {
            avcodec_close(m_pAStream->codec);
        }
        m_pAStream = 0;
    }
    if (m_AStreamIndex >= 0)
    {
        if (m_pAStream->codec->channels > m_AP.m_Channels) {
            AVG_TRACE(Logger::WARNING, 
                    m_sFilename << ": unsupported number of channels (" << 
                            m_pAStream->codec->channels << "). Disabling audio.");
            m_AStreamIndex = -1;
            m_pAStream = 0; 
        } else {
            m_pSampleBuffer = (char*)av_mallocz(SAMPLE_BUFFER_SIZE);
            m_SampleBufferStart = 0;
            m_SampleBufferEnd = 0;
            m_SampleBufferLeft = SAMPLE_BUFFER_SIZE;

            m_ResampleBufferSize = 0;
            m_pResampleBuffer = 0;
            m_ResampleBufferStart = 0;
            m_ResampleBufferEnd = 0;
        
            m_pDemuxer->enableStream(m_AStreamIndex);
        }
    }
    if (m_VStreamIndex < 0 && m_AStreamIndex < 0) {
        throw Exception(AVG_ERR_VIDEO_INIT_FAILED, 
                m_sFilename + " does not contain any valid audio or video streams.");
    }
    m_State = DECODING;
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
        if(m_pAudioResampleContext) {
            audio_resample_close(m_pAudioResampleContext);
            m_pAudioResampleContext = 0;
        }
        
        if (m_pSampleBuffer) {
            av_free(m_pSampleBuffer);
            m_pSampleBuffer = 0;
        }
        if(m_pResampleBuffer) {
            av_free(m_pResampleBuffer);
            m_pResampleBuffer = 0;
        }

        m_AudioPacketData = 0;
        m_AudioPacketSize = 0;
        
        m_SampleBufferStart = 0;
        m_SampleBufferEnd = 0;
        m_SampleBufferLeft = 0;

        m_ResampleBufferStart = 0;
        m_ResampleBufferEnd = 0;
        m_ResampleBufferSize = 0;
        
        m_LastAudioFrameTime = 0;
        m_AudioStartTimestamp = 0;
        
        m_pAStream = 0;
        m_AStreamIndex = -1;
    }
    if (m_pFormatContext) {
        av_close_input_file(m_pFormatContext);
        m_pFormatContext = 0;
    }
    
    if(m_pSwsContext) {
        sws_freeContext(m_pSwsContext);
        m_pSwsContext = 0;
    }
    m_State = CLOSED;
}

IVideoDecoder::DecoderState FFMpegDecoder::getState() const
{
    return m_State;
}

VideoInfo FFMpegDecoder::getVideoInfo() const
{
    AVG_ASSERT(m_State != CLOSED);
    long long duration = 0;
    if (m_pVStream || m_pAStream) {
        duration = getDuration();
    }
    VideoInfo info(duration, m_pFormatContext->bit_rate, m_pVStream != 0,
            m_pAStream != 0);
    if (m_pVStream) {
        info.setVideoData(m_Size, getStreamPF(), getNumFrames(), getNominalFPS(), m_FPS,
                m_pVStream->codec->codec->name);
    }
    if (m_pAStream) {
        AVCodecContext * pACodec = m_pAStream->codec;
        info.setAudioData(pACodec->codec->name, pACodec->sample_rate,
                pACodec->channels);
    }
    return info;
}

void FFMpegDecoder::seek(long long DestTime) 
{
    if (m_bFirstPacket && m_pVStream) {
        AVFrame Frame;
        long long FrameTime;
        readFrame(Frame, FrameTime);
    }
    m_pDemuxer->seek(DestTime + getStartTime());
    if (m_pVStream) {
        m_LastVideoFrameTime = DestTime - (long long)(1000.0/m_FPS);
    }
    if (m_pAStream) {
        mutex::scoped_lock Lock(m_AudioMutex);
        m_LastAudioFrameTime = double(DestTime);
        m_SampleBufferStart = m_SampleBufferEnd = 0;
        m_SampleBufferLeft = SAMPLE_BUFFER_SIZE;
        m_ResampleBufferStart = m_ResampleBufferEnd = 0;
        m_AudioPacketSize = 0;
    }
    m_bVideoEOF = false;
    m_bAudioEOF = false;
}

void FFMpegDecoder::loop()
{
    seek(0);
}

IntPoint FFMpegDecoder::getSize() const
{
    AVG_ASSERT(m_State != CLOSED);
    return m_Size;
}

int FFMpegDecoder::getCurFrame() const
{
    AVG_ASSERT(m_State != CLOSED);
    return int(getCurTime(SS_VIDEO)*getNominalFPS()/1000.0+0.5);
}

int FFMpegDecoder::getNumFramesQueued() const
{
    return 0;
}

long long FFMpegDecoder::getCurTime(StreamSelect Stream) const
{
    AVG_ASSERT(m_State != CLOSED);
    switch(Stream) {
        case SS_DEFAULT:
        case SS_VIDEO:
            AVG_ASSERT(m_pVStream);
            return m_LastVideoFrameTime;
        case SS_AUDIO:
            AVG_ASSERT(m_pAStream);
            return (long long)m_LastAudioFrameTime;
        default:
            return -1;
    }
}

long long FFMpegDecoder::getDuration() const
{
    AVG_ASSERT(m_State != CLOSED);
    long long duration;
    AVRational time_base;
    if (m_pVStream) {
        duration=m_pVStream->duration;
        time_base=m_pVStream->time_base;
    } else {
        duration=m_pAStream->duration;
        time_base=m_pAStream->time_base;
    }
#if LIBAVFORMAT_BUILD < ((49<<16)+(0<<8)+0)
    return (long long)(1000*duration/AV_TIME_BASE);
#else
    return (long long)(1000*duration*av_q2d(time_base));
#endif 
}

double FFMpegDecoder::getNominalFPS() const
{
    AVG_ASSERT(m_State != CLOSED);
#if LIBAVFORMAT_BUILD < ((49<<16)+(0<<8)+0)
    return m_pVStream->r_frame_rate;
#else
    return av_q2d(m_pVStream->r_frame_rate);
#endif 
}

double FFMpegDecoder::getFPS() const
{
    AVG_ASSERT(m_State != CLOSED);
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

double FFMpegDecoder::getVolume() const
{
    AVG_ASSERT(m_State != CLOSED);
    return m_Volume;
}

void FFMpegDecoder::setVolume(double Volume)
{
    m_Volume = Volume;
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

static ProfilingZoneID RenderToBmpProfilingZone("FFMpeg: renderToBmp");
static ProfilingZoneID CopyImageProfilingZone("FFMpeg: copy image");

FrameAvailableCode FFMpegDecoder::renderToBmps(vector<BitmapPtr>& pBmps, 
        long long timeWanted)
{
    AVG_ASSERT(m_State == DECODING);
    ScopeTimer Timer(RenderToBmpProfilingZone);
    AVFrame frame;
    FrameAvailableCode frameAvailable = readFrameForTime(frame, timeWanted);
    if (!m_bVideoEOF && frameAvailable == FA_NEW_FRAME) {
        if (pixelFormatIsPlanar(m_PF)) {
            ScopeTimer Timer(CopyImageProfilingZone);
            for (unsigned i=0; i < pBmps.size(); ++i) {
                copyPlaneToBmp(pBmps[i], frame.data[i], frame.linesize[i]);
            }
        } else {
            convertFrameToBmp(frame, pBmps[0]);
        }
        return FA_NEW_FRAME;
    }
    return FA_USE_LAST_FRAME;
}

void FFMpegDecoder::throwAwayFrame(long long timeWanted)
{
    AVG_ASSERT(m_State == DECODING);
    AVFrame Frame;
    readFrameForTime(Frame, timeWanted);
}

bool FFMpegDecoder::isEOF(StreamSelect Stream) const
{
    AVG_ASSERT(m_State == DECODING);
    switch(Stream) {
        case SS_AUDIO:
            return (!m_pAStream || m_bAudioEOF);
        case SS_VIDEO:
            return (!m_pVStream || m_bVideoEOF);
        case SS_ALL:
            return isEOF(SS_VIDEO) && isEOF(SS_AUDIO);
        default:
            return false;
    }
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
        m_SampleBufferLeft = SAMPLE_BUFFER_SIZE;
    }
    
    return bytesWritten;
}

int FFMpegDecoder::copyResampledAudio(unsigned char* buf, int size)
{
    int bytesWritten = 0;
                        
    // If there is no buffered resampled data, resample some more
    if(m_ResampleBufferStart >= m_ResampleBufferEnd)
        resampleAudio();
    
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
            m_SampleBufferLeft = SAMPLE_BUFFER_SIZE;
        }
    }
    
    return bytesWritten;
}

void FFMpegDecoder::resampleAudio()
{
    if (!m_pAudioResampleContext) {
        m_pAudioResampleContext = audio_resample_init(
                m_AP.m_Channels, m_pAStream->codec->channels, 
                m_AP.m_SampleRate, m_EffectiveSampleRate);
    }
    
    if (!m_pResampleBuffer) {
        m_ResampleBufferSize = (int)(SAMPLE_BUFFER_SIZE * 
                ((double)m_AP.m_SampleRate / (double)m_EffectiveSampleRate));
        m_pResampleBuffer = (char*)av_mallocz(m_ResampleBufferSize);
    }
    
    int inputSamples = 
        (m_SampleBufferEnd - m_SampleBufferStart) / 
        (2 * m_pAStream->codec->channels);
    
    int outputSamples = audio_resample(m_pAudioResampleContext,
            (short*)m_pResampleBuffer,
            (short*)(m_pSampleBuffer + m_SampleBufferStart), 
            inputSamples);
    
    // Adjust buffer pointers
    m_ResampleBufferEnd += outputSamples * 2 * m_AP.m_Channels;
    m_SampleBufferStart += inputSamples * 2 * m_pAStream->codec->channels;
}

int FFMpegDecoder::decodeAudio()
{
    // Save current size of the audio buffer
    int lastSampleBufferSize = m_SampleBufferLeft;

#if LIBAVCODEC_BUILD > ((51<<16)+(11<<8)+0)
    // Decode some data from packet into the audio buffer
    int packetBytesDecoded = avcodec_decode_audio2(
            m_pAStream->codec, 
            (short*)(m_pSampleBuffer + m_SampleBufferEnd),
            &m_SampleBufferLeft, 
            m_AudioPacketData,
            m_AudioPacketSize);
#else
    int packetBytesDecoded = avcodec_decode_audio(
            m_pAStream->codec, 
            (short*)(m_pSampleBuffer + m_SampleBufferEnd),
            &m_SampleBufferLeft, 
            m_AudioPacketData,
            m_AudioPacketSize);
#endif

    // Skip frame on error
    if (packetBytesDecoded < 0)
        return -1;
    
    // Did not get any data, try again
    if (packetBytesDecoded == 0)
        return 0;
    
    // Adjust audio buffer pointers
    m_SampleBufferEnd += m_SampleBufferLeft;
    m_SampleBufferLeft = lastSampleBufferSize - m_SampleBufferLeft;
                
    // Adjust packet data pointers
    m_AudioPacketData += packetBytesDecoded;
    m_AudioPacketSize -= packetBytesDecoded;
    return packetBytesDecoded;
}

int FFMpegDecoder::fillAudioBuffer(AudioBufferPtr pBuffer)
{
    AVG_ASSERT(m_State == DECODING);
    mutex::scoped_lock Lock(m_AudioMutex);

    unsigned char* outputAudioBuffer = (unsigned char*)(pBuffer->getData());
    int outputAudioBufferSize = pBuffer->getNumBytes();

    AVG_ASSERT (m_pAStream);
    if (m_bAudioEOF) {
        return 0;
    }
    
    int packetBytesDecoded;
    int bytesProduced;
    unsigned char* pCurBufferPos = outputAudioBuffer;
    int bufferLeft = outputAudioBufferSize;
    bool bFormatMatch = (m_EffectiveSampleRate == m_AP.m_SampleRate &&
                         m_pAStream->codec->channels == m_AP.m_Channels);

    while(true)
    {
        while(true)
        {
            // Consume any data left in the sample buffers
            while(m_SampleBufferStart < m_SampleBufferEnd ||
                  m_ResampleBufferStart < m_ResampleBufferEnd)
            {
                // If the output format is different from the decoded format,
                // then convert it, else copy it over
                if (bFormatMatch) {
                    bytesProduced =
                        copyRawAudio(pCurBufferPos, bufferLeft);
                } else {
                    bytesProduced =
                        copyResampledAudio(pCurBufferPos, bufferLeft);
                }
                
                pCurBufferPos += bytesProduced;
                bufferLeft -= bytesProduced;

                m_LastAudioFrameTime += (1000.0 * bytesProduced /
                        (2 * m_AP.m_Channels * m_AP.m_SampleRate));
                if (bufferLeft == 0) {
                    volumize(pBuffer);
                    return pBuffer->getNumFrames();
                }
            }
            
            if (m_AudioPacketSize <= 0)
                break;
            
            packetBytesDecoded = decodeAudio();
            
            // Skip frame on error
            if (packetBytesDecoded < 0)
                break;
            
            // Did not get any data, try again
            if (packetBytesDecoded == 0)
                continue;
        }
        
        // We have decoded all data in the packet, free it
        if (m_AudioPacket) {
            av_free_packet(m_AudioPacket);
            delete m_AudioPacket;
        }
        
        // Get a new packet from the audio stream
        m_AudioPacket = m_pDemuxer->getPacket(m_AStreamIndex);
        
        if(!m_AudioPacket) {
            m_bAudioEOF = true;
            return pBuffer->getNumFrames()-bufferLeft/(pBuffer->getFrameSize());
        }
/* 
        if(m_AudioPacket->dts != AV_NOPTS_VALUE) {
            long long dts = (long long)(1000.0 * av_q2d(m_pAStream->time_base) * 
                    m_AudioPacket->dts - m_AudioStartTimestamp);
        }
*/
        // Initialize packet data pointers
        m_AudioPacketData = m_AudioPacket->data;
        m_AudioPacketSize = m_AudioPacket->size;
    }
}

PixelFormat FFMpegDecoder::calcPixelFormat(bool bUseYCbCr)
{
#if LIBAVFORMAT_BUILD < ((49<<16)+(0<<8)+0)
        AVCodecContext *enc = &m_pVStream->codec;
#else
        AVCodecContext *enc = m_pVStream->codec;
#endif
    if (bUseYCbCr) {
        switch(enc->pix_fmt) {
            case PIX_FMT_YUV420P:
                return YCbCr420p;
            case PIX_FMT_YUVJ420P:
                return YCbCrJ420p;
            case PIX_FMT_YUVA420P:
                return YCbCrA420p;
            default:
                break;
        }
    }
    if (enc->pix_fmt == PIX_FMT_BGRA || enc->pix_fmt == PIX_FMT_YUVA420P) {
        return B8G8R8A8;
    }
    return B8G8R8X8;
}

static ProfilingZoneID ConvertImageLibavgProfilingZone(
            "FFMpeg: colorspace conv (libavg)");
static ProfilingZoneID ConvertImageSWSProfilingZone("FFMpeg: colorspace conv (SWS)");
static ProfilingZoneID SetAlphaProfilingZone("FFMpeg: set alpha channel");

void FFMpegDecoder::convertFrameToBmp(AVFrame& Frame, BitmapPtr pBmp)
{
    AVPicture DestPict;
    unsigned char * pDestBits = pBmp->getPixels();
    DestPict.data[0] = pDestBits;
    DestPict.linesize[0] = pBmp->getStride();
#if LIBAVFORMAT_BUILD >= ((50<<16)+(0<<8)+0)
    ::PixelFormat DestFmt;
#else
    int DestFmt;
#endif
    switch(pBmp->getPixelFormat()) {
        case R8G8B8X8:
        case R8G8B8A8:
            // XXX: Unused and broken.
            DestFmt = PIX_FMT_BGRA;
            break;
        case B8G8R8X8:
        case B8G8R8A8:
            DestFmt = PIX_FMT_BGRA;
            break;
        case R8G8B8:
            DestFmt = PIX_FMT_RGB24;
            break;
        case B8G8R8:
            DestFmt = PIX_FMT_BGR24;
            break;
        case YCbCr422:
            DestFmt = PIX_FMT_YUYV422;
            break;
        default:
            AVG_TRACE(Logger::ERROR, "FFMpegDecoder: Dest format " 
                    << pBmp->getPixelFormat() << " not supported.");
            AVG_ASSERT(false);
            DestFmt = PIX_FMT_BGRA;
    }
#if LIBAVFORMAT_BUILD < ((49<<16)+(0<<8)+0)
    AVCodecContext *enc = &m_pVStream->codec;
#else
    AVCodecContext *enc = m_pVStream->codec;
#endif
    {
        if (DestFmt == PIX_FMT_BGRA && (enc->pix_fmt == PIX_FMT_YUV420P || 
                enc->pix_fmt == PIX_FMT_YUVJ420P)) {
            ScopeTimer Timer(ConvertImageLibavgProfilingZone);
            BitmapPtr pBmpY(new Bitmap(pBmp->getSize(), I8, Frame.data[0],
                    Frame.linesize[0], false));
            BitmapPtr pBmpU(new Bitmap(pBmp->getSize(), I8, Frame.data[1],
                    Frame.linesize[1], false));
            BitmapPtr pBmpV(new Bitmap(pBmp->getSize(), I8, Frame.data[2],
                    Frame.linesize[2], false));
            pBmp->copyYUVPixels(*pBmpY, *pBmpU, *pBmpV, enc->pix_fmt == PIX_FMT_YUVJ420P);
        } else {
            if (!m_pSwsContext) {
                m_pSwsContext = sws_getContext(enc->width, enc->height, enc->pix_fmt,
                            enc->width, enc->height, DestFmt, SWS_BICUBIC, 
                            NULL, NULL, NULL);
                AVG_ASSERT(m_pSwsContext);
            }
            {
                ScopeTimer Timer(ConvertImageSWSProfilingZone);
                sws_scale(m_pSwsContext, Frame.data, Frame.linesize, 0, 
                    enc->height, DestPict.data, DestPict.linesize);
            }
            if (pBmp->getPixelFormat() == B8G8R8X8) {
                ScopeTimer Timer(SetAlphaProfilingZone);
                // Make sure the alpha channel is white.
                // TODO: This is slow. Make OpenGL do it.
                unsigned char * pLine = pBmp->getPixels();
                IntPoint size = pBmp->getSize();
                for (int y = 0; y<size.y; ++y) {
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
}
       
PixelFormat FFMpegDecoder::getPixelFormat() const
{
    AVG_ASSERT(m_State != CLOSED);
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

int FFMpegDecoder::getNumFrames() const
{
    AVG_ASSERT(m_State != CLOSED);
    // This is broken for some videos, but the code here is correct.
    // So fix ffmpeg :-).
#if LIBAVFORMAT_BUILD < ((49<<16)+(0<<8)+0)
    return m_pVStream->r_frame_rate*(m_pVStream->duration/AV_TIME_BASE);
#else
    return m_pVStream->nb_frames;
#endif 
}

FrameAvailableCode FFMpegDecoder::readFrameForTime(AVFrame& Frame, long long timeWanted)
{
    AVG_ASSERT(m_State == DECODING);
    // XXX: This code is sort-of duplicated in AsyncVideoDecoder::getBmpsForTime()
    long long FrameTime = -1000;
//    cerr << m_sFilename << " readFrameForTime " << timeWanted
//                << ", LastFrameTime= " << m_LastVideoFrameTime << ", diff= " 
//                << timeWanted-m_LastVideoFrameTime << endl;
    if (timeWanted == -1) {
        readFrame(Frame, FrameTime);
    } else {
        double TimePerFrame = 1000/m_FPS;
        if (timeWanted-m_LastVideoFrameTime < 0.5*TimePerFrame) {
//            cerr << "   LastFrameTime = " << m_LastFrameTime << ", display again."
//                    <<  endl;
            // The last frame is still current. Display it again.
            return FA_USE_LAST_FRAME;
        } else {
            while (FrameTime-timeWanted < -0.5*TimePerFrame && !m_bVideoEOF) {
                readFrame(Frame, FrameTime);
//                cerr << "   readFrame returned time " << FrameTime << "." <<  endl;
            }
//            cerr << "  frame ok." << endl;
        }
    }
    return FA_NEW_FRAME;
}

static ProfilingZoneID DecodeProfilingZone("FFMpeg: decode");

void FFMpegDecoder::readFrame(AVFrame& Frame, long long& FrameTime)
{
    AVG_ASSERT(m_pDemuxer);
    ScopeTimer Timer(DecodeProfilingZone); 
    if (m_bEOFPending) {
        m_bVideoEOF = true;
        m_bEOFPending = false;
        return;
    }
#if LIBAVFORMAT_BUILD < ((49<<16)+(0<<8)+0)
    AVCodecContext *enc = &m_pVStream->codec;
#else
    AVCodecContext *enc = m_pVStream->codec;
#endif
    int bGotPicture = 0;
    AVPacket* pPacket = 0;
    while (!bGotPicture && !m_bVideoEOF) {
        pPacket = m_pDemuxer->getPacket(m_VStreamIndex);
        m_bFirstPacket = false;
        if (pPacket) {
    //        cerr << "decode, size=" << pPacket->size << endl;
            int Len1 = avcodec_decode_video(enc, &Frame, &bGotPicture, pPacket->data,
                    pPacket->size);
            if (Len1 > 0) {
                AVG_ASSERT(Len1 == pPacket->size);
            }
            if (bGotPicture) {
                FrameTime = getFrameTime(pPacket->dts);
            }
            av_free_packet(pPacket);
            delete pPacket;
        } else {
            // No more packets -> EOF. Decode the last data we got.
            avcodec_decode_video(enc, &Frame, &bGotPicture, 0, 0);
            if (bGotPicture) {
                m_bEOFPending = true;
            } else {
                m_bVideoEOF = true;
            }
            // We don't have a timestamp for the last frame, so we'll
            // calculate it based on the frame before.
            FrameTime = m_LastVideoFrameTime+(long long)(1000.0/m_FPS);
            m_LastVideoFrameTime = FrameTime;
        }
    }
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

long long FFMpegDecoder::getFrameTime(long long dts)
{
    if (m_VideoStartTimestamp == -1) {
        m_VideoStartTimestamp = (long long)((1000*dts)/m_TimeUnitsPerSecond);
    }
    long long FrameTime;
    if (m_bUseStreamFPS) {
        FrameTime = (long long)((1000*dts)/m_TimeUnitsPerSecond)-m_VideoStartTimestamp;
    } else {
        if (m_LastVideoFrameTime == -1000) {
            FrameTime = 0;
        } else {
            FrameTime = m_LastVideoFrameTime + (long long)(1000.0/m_FPS);
        }
    }
    m_LastVideoFrameTime = FrameTime;
    return FrameTime;
}

long long FFMpegDecoder::getStartTime()
{
    if (m_pVStream) {
        return m_VideoStartTimestamp;
    } else {
        return m_AudioStartTimestamp;
    }
}

double FFMpegDecoder::calcStreamFPS() const
{
#if LIBAVFORMAT_BUILD < ((49<<16)+(0<<8)+0)
    return m_pVStream->r_frame_rate;
#else
    return (m_pVStream->r_frame_rate.num/m_pVStream->r_frame_rate.den);
#endif 
}

string FFMpegDecoder::getStreamPF() const
{
#if LIBAVFORMAT_BUILD < ((49<<16)+(0<<8)+0)
    AVCodecContext *pCodec = &m_pVStream->codec;
#else
    AVCodecContext *pCodec = m_pVStream->codec;
#endif
    ::PixelFormat pf = pCodec->pix_fmt;
    return avcodec_get_pix_fmt_name(pf);
}

// TODO: this should be logarithmic...
void FFMpegDecoder::volumize(AudioBufferPtr pBuffer)
{
    double curVol = m_Volume;
    double volDiff = m_LastVolume - curVol;
    
    if(curVol == 1.0 && volDiff == 0.0)
        return;
   
    short * pData = pBuffer->getData();
    for (int i = 0; i < pBuffer->getNumFrames()*pBuffer->getNumChannels(); i++) {
        double fadeVol = 0;
        if (volDiff != 0 && i < VOLUME_FADE_SAMPLES) {
            fadeVol = volDiff * (VOLUME_FADE_SAMPLES - i) / VOLUME_FADE_SAMPLES;
        }
        
        int s = int(pData[i] * (curVol + fadeVol));
        
        if (s < -32768)
            s = -32768;
        if (s >  32767)
            s = 32767;
        
        pData[i] = s;
    }
    m_LastVolume = curVol;
}

}

