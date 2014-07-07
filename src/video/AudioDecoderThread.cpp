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
//  Original author of this file is Nick Hebner (hebnern@gmail.com).
//

#include "AudioDecoderThread.h"

#include "../base/Logger.h"
#include "../base/TimeSource.h"
#include "../base/ScopeTimer.h"

#if AVUTIL_VERSION_INT > AV_VERSION_INT(52, 0, 0)
#include <libavutil/samplefmt.h>
#endif

#ifndef AVCODEC_MAX_AUDIO_FRAME_SIZE
    #define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000
#endif

using namespace std;

namespace avg {

AudioDecoderThread::AudioDecoderThread(CQueue& cmdQ, AudioMsgQueue& msgQ, 
        VideoMsgQueue& packetQ, AVStream* pStream, const AudioParams& ap)
    : WorkerThread<AudioDecoderThread>(string("AudioDecoderThread"), cmdQ),
      m_MsgQ(msgQ),
      m_PacketQ(packetQ),
      m_AP(ap),
      m_pStream(pStream),
      m_pResampleContext(0),
      m_State(DECODING)
{
    m_LastFrameTime = 0;
    m_AudioStartTimestamp = 0;

    if (m_pStream->start_time != (long long)AV_NOPTS_VALUE) {
        m_AudioStartTimestamp = float(av_q2d(m_pStream->time_base)*m_pStream->start_time);
    }
    m_InputSampleRate = (int)(m_pStream->codec->sample_rate);
    m_InputSampleFormat = m_pStream->codec->sample_fmt;
}

AudioDecoderThread::~AudioDecoderThread()
{
    if (m_pResampleContext) {
#ifdef LIBAVRESAMPLE_VERSION
        avresample_close(m_pResampleContext);
        avresample_free(&m_pResampleContext);
#else
        audio_resample_close(m_pResampleContext);
#endif
        m_pResampleContext = 0;
    }
}

static ProfilingZoneID DecoderProfilingZone("Audio Decoder Thread", true);
static ProfilingZoneID PacketWaitProfilingZone("Audio Wait for packet", true);

bool AudioDecoderThread::work() 
{
    ScopeTimer timer(DecoderProfilingZone);
    VideoMsgPtr pMsg;
    {
        ScopeTimer timer(PacketWaitProfilingZone);
        pMsg = m_PacketQ.pop(true);
    }
    switch (pMsg->getType()) {
        case VideoMsg::PACKET: {
            AVPacket* pPacket = pMsg->getPacket();
            switch(m_State) {
                case DECODING:
                    decodePacket(pPacket);
                    break;
                case SEEK_DONE:
                    handleSeekDone(pPacket);
                    break;
                case DISCARDING:
                    discardPacket(pPacket);
                    break;
                default:
                    AVG_ASSERT(false);
            }
            av_free_packet(pPacket);
            delete pPacket;
            break;
        }
        case VideoMsg::SEEK_DONE:
            m_State = SEEK_DONE;
            m_SeekSeqNum = pMsg->getSeekSeqNum();
            m_SeekTime = pMsg->getSeekTime();
            break;
        case VideoMsg::END_OF_FILE:
            pushEOF();
            break;
        case VideoMsg::CLOSED:
            m_MsgQ.clear();
            stop();
            break;
        default:
            pMsg->dump();
            AVG_ASSERT(false);
    }
    ThreadProfiler::get()->reset();
    return true;
}

void AudioDecoderThread::decodePacket(AVPacket* pPacket)
{
    char* pDecodedData;
    AVPacket* pTempPacket = new AVPacket;
    av_init_packet(pTempPacket);
    pTempPacket->data = pPacket->data;
    pTempPacket->size = pPacket->size;
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 25, 0)
    AVFrame* pDecodedFrame;
    pDecodedFrame = avcodec_alloc_frame();
#else
    pDecodedData = (char*)av_malloc(AVCODEC_MAX_AUDIO_FRAME_SIZE +
            FF_INPUT_BUFFER_PADDING_SIZE);
#endif
    while (pTempPacket->size > 0) {
        int gotFrame = 0;
        int bytesDecoded = AVCODEC_MAX_AUDIO_FRAME_SIZE;
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 25, 0)
        int bytesConsumed = avcodec_decode_audio4(m_pStream->codec, pDecodedFrame,
                &gotFrame, pTempPacket);
        if (gotFrame) {
            bytesDecoded = av_samples_get_buffer_size(0, m_pStream->codec->channels,
                    pDecodedFrame->nb_samples, m_pStream->codec->sample_fmt, 1);
            pDecodedData = (char*)(pDecodedFrame->data[0]);
        } else {
            bytesDecoded = 0;
        }
#else
        int bytesConsumed = avcodec_decode_audio3(m_pStream->codec, (short*)pDecodedData,
                &bytesDecoded, pTempPacket);
#endif
//        This is triggered for some strange/broken videos.
//        AVG_ASSERT(bytesConsumed != 0);
        if (bytesConsumed < 0) {
            // Error decoding -> throw away current packet.
            bytesDecoded = 0;
            pTempPacket->size = 0;
        } else {
            pTempPacket->data += bytesConsumed;
            pTempPacket->size -= bytesConsumed;
        }
        if (bytesDecoded > 0) {
            int framesDecoded = bytesDecoded/(m_pStream->codec->channels*
                    getBytesPerSample(m_InputSampleFormat));
            AudioBufferPtr pBuffer;
            bool bNeedsResample = (m_InputSampleRate != m_AP.m_SampleRate ||
                    m_InputSampleFormat != SAMPLE_FMT_S16 ||
                    m_pStream->codec->channels != m_AP.m_Channels);
            bool bIsPlanar = false;
#if LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(51, 27, 0)
            bIsPlanar = av_sample_fmt_is_planar((SampleFormat)m_InputSampleFormat);
            if (bIsPlanar) {
                char* pPackedData = (char*)av_malloc(AVCODEC_MAX_AUDIO_FRAME_SIZE +
                        FF_INPUT_BUFFER_PADDING_SIZE);
                planarToInterleaved(pPackedData, pDecodedData, m_pStream->codec->channels,
                        m_pStream->codec->frame_size);
                pBuffer = resampleAudio(pPackedData, framesDecoded,
                        av_get_packed_sample_fmt((SampleFormat)m_InputSampleFormat));
                av_free(pPackedData);
                bNeedsResample = false;
            }
#endif
            if (bNeedsResample) {
                pBuffer = resampleAudio(pDecodedData, framesDecoded,
                        m_InputSampleFormat);
            } else if (!bIsPlanar) {
                pBuffer = AudioBufferPtr(new AudioBuffer(framesDecoded, m_AP));
                memcpy(pBuffer->getData(), pDecodedData, bytesDecoded);
            }
            m_LastFrameTime += float(pBuffer->getNumFrames())/m_AP.m_SampleRate;
            pushAudioMsg(pBuffer, m_LastFrameTime);
        }
    }
#if LIBAVCODEC_VERSION_MAJOR > 53
    avcodec_free_frame(&pDecodedFrame);
#elif LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 25, 0)
    delete pDecodedFrame;
#else
    av_free(pDecodedData);
#endif
    delete pTempPacket;
}

void AudioDecoderThread::handleSeekDone(AVPacket* pPacket)
{
    m_MsgQ.clear();
    m_LastFrameTime = float(pPacket->dts*av_q2d(m_pStream->time_base))
            - m_AudioStartTimestamp;

   if (fabs(m_LastFrameTime - m_SeekTime) < 0.01) {
        pushSeekDone(m_LastFrameTime, m_SeekSeqNum);
        decodePacket(pPacket);
        m_State = DECODING;
    } else {
        if (m_LastFrameTime-0.01f < m_SeekTime) {
            // Received frame that's earlier than the destination, so throw away frames
            // until the time is correct.
            m_State = DISCARDING;
        } else {
            // Received frame that's too late, so insert a buffer of silence to 
            // compensate.
            insertSilence(m_LastFrameTime - m_SeekTime);
            m_LastFrameTime = m_SeekTime;
            pushSeekDone(m_LastFrameTime, m_SeekSeqNum);
            decodePacket(pPacket);
            m_State = DECODING;
        }
    }
}

void AudioDecoderThread::discardPacket(AVPacket* pPacket)
{
    m_LastFrameTime = float(pPacket->dts*av_q2d(m_pStream->time_base))
            - m_AudioStartTimestamp;
    if (m_LastFrameTime-0.01f > m_SeekTime) {
        pushSeekDone(m_LastFrameTime, m_SeekSeqNum);
        m_State = DECODING;
    }
}

AudioBufferPtr AudioDecoderThread::resampleAudio(char* pDecodedData, int framesDecoded,
        int currentSampleFormat)
{
    if (!m_pResampleContext) {
#ifdef LIBAVRESAMPLE_VERSION
        m_pResampleContext = avresample_alloc_context();
        av_opt_set_int(m_pResampleContext, "in_channel_layout",
                av_get_default_channel_layout(m_pStream->codec->channels), 0);
        av_opt_set_int(m_pResampleContext, "out_channel_layout", AV_CH_LAYOUT_STEREO, 0);
        av_opt_set_int(m_pResampleContext, "in_sample_rate", m_InputSampleRate, 0);
        av_opt_set_int(m_pResampleContext, "out_sample_rate", m_AP.m_SampleRate, 0);
        av_opt_set_int(m_pResampleContext, "in_sample_fmt",
                (SampleFormat)currentSampleFormat, 0);
        av_opt_set_int(m_pResampleContext, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);
        int err = avresample_open(m_pResampleContext);
        AVG_ASSERT(err >= 0);
#else
        m_pResampleContext = av_audio_resample_init(m_AP.m_Channels, 
                m_pStream->codec->channels, m_AP.m_SampleRate, m_InputSampleRate,
                SAMPLE_FMT_S16, (SampleFormat)currentSampleFormat, 16, 10, 0, 0.8);
#endif
        AVG_ASSERT(m_pResampleContext);
    }
#ifdef LIBAVRESAMPLE_VERSION
    uint8_t *pResampledData;
    int leftoverSamples = avresample_available(m_pResampleContext);
    int framesAvailable = leftoverSamples +
            av_rescale_rnd(avresample_get_delay(m_pResampleContext) +
                    framesDecoded, m_AP.m_SampleRate, m_InputSampleRate, AV_ROUND_UP);
    av_samples_alloc(&pResampledData, 0, 2, framesAvailable,
            AV_SAMPLE_FMT_S16, 0);
    int framesResampled = avresample_convert(m_pResampleContext, &pResampledData, 0, 
            framesAvailable, (uint8_t**)&pDecodedData, 0, framesDecoded);
    AudioBufferPtr pBuffer(new AudioBuffer(framesResampled, m_AP));
    memcpy(pBuffer->getData(), pResampledData, 
            framesResampled*m_AP.m_Channels*sizeof(short));
    av_freep(&pResampledData);
#else
    short pResampledData[AVCODEC_MAX_AUDIO_FRAME_SIZE/2];
    int framesResampled = audio_resample(m_pResampleContext, pResampledData,
            (short*)pDecodedData, framesDecoded);
    AudioBufferPtr pBuffer(new AudioBuffer(framesResampled, m_AP));
    memcpy(pBuffer->getData(), pResampledData, 
            framesResampled*m_AP.m_Channels*sizeof(short));
#endif
    return pBuffer;
}

void AudioDecoderThread::planarToInterleaved(char* pOutput, char* pInput, int numChannels,
        int numSamples)
{
    AVG_ASSERT(numChannels <= 8);
    if (numSamples == 0) {
        // Fishy, some ogg files have no proper frame_size set. But outputBufferSamples
        // worked for sample ogg file.
        numSamples = m_AP.m_OutputBufferSamples;
    }
    int i, j;
    int bytesPerSample = getBytesPerSample(m_InputSampleFormat);
    char * pPlanes[8] = {};
    for (i=0; i<numChannels; i++) {
        pPlanes[i] = pInput + i*(numSamples*bytesPerSample);
    }
    for (i=0; i<numSamples; i++) {
        for (j=0; j<numChannels; j++) {
            memcpy(pOutput, pPlanes[j], bytesPerSample);
            pOutput += bytesPerSample;
            pPlanes[j] += bytesPerSample;
        }
    }
}

void AudioDecoderThread::insertSilence(float duration)
{
    int numDelaySamples = int(duration*m_AP.m_SampleRate);
    AudioBufferPtr pBuffer(new AudioBuffer(numDelaySamples, m_AP));
    pBuffer->clear();
    pushAudioMsg(pBuffer, m_LastFrameTime);
}

void AudioDecoderThread::pushAudioMsg(AudioBufferPtr pBuffer, float time)
{
    VideoMsgPtr pMsg(new VideoMsg());
    pMsg->setAudio(pBuffer, time);
    m_MsgQ.push(pMsg);
}

void AudioDecoderThread::pushSeekDone(float time, int seqNum)
{
    VideoMsgPtr pMsg(new VideoMsg());
    pMsg->setSeekDone(seqNum, time);
    m_MsgQ.push(pMsg);
}

void AudioDecoderThread::pushEOF()
{
    VideoMsgPtr pMsg(new VideoMsg());
    pMsg->setEOF();
    m_MsgQ.push(pMsg);
}

int AudioDecoderThread::getBytesPerSample(int sampleFormat)
{
    switch (sampleFormat) {
        case SAMPLE_FMT_U8:
            return 1;
        case SAMPLE_FMT_S16:
            return 2;
        case SAMPLE_FMT_S32:
            return 4;
        case SAMPLE_FMT_FLT:
            return 4;
        case SAMPLE_FMT_DBL:
            return 8;
#if LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(52, 3, 0)
        case SAMPLE_FMT_S16P:
            return 2;
        case SAMPLE_FMT_FLTP:
            return 4;
#endif
        default:
            AVG_LOG_ERROR("Unknown SampleFormat: " << sampleFormat << "\n");
            AVG_ASSERT(false);
            return 0;
    }
}

}
