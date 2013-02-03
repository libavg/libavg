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
//  Original author of this file is Nick Hebner (hebnern@gmail.com).
//

#include "AudioDecoderThread.h"

#include "../base/Logger.h"
#include "../base/TimeSource.h"
#include "../base/ScopeTimer.h"

using namespace std;

namespace avg {

AudioDecoderThread::AudioDecoderThread(CQueue& cmdQ, AudioMsgQueue& msgQ, 
        VideoMsgQueue& packetQ, AVStream* pStream, const AudioParams& ap)
    : WorkerThread<AudioDecoderThread>(string("AudioDecoderThread"), cmdQ),
      m_MsgQ(msgQ),
      m_PacketQ(packetQ),
      m_AP(ap),
      m_pAStream(pStream),
      m_pAudioResampleContext(0),
      m_State(DECODING)
{
    m_LastFrameTime = 0;
    m_AudioStartTimestamp = 0;

    if (m_pAStream->start_time != (long long)AV_NOPTS_VALUE) {
        m_AudioStartTimestamp = float(av_q2d(m_pAStream->time_base)*m_pAStream->start_time);
    }
    m_InputSampleRate = (int)(m_pAStream->codec->sample_rate);
}

AudioDecoderThread::~AudioDecoderThread()
{
    if (m_pAudioResampleContext) {
        audio_resample_close(m_pAudioResampleContext);
        m_pAudioResampleContext = 0;
    }

    m_LastFrameTime = 0;
    m_AudioStartTimestamp = 0;
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
            close();
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
    short* pDecodedData = (short*)av_malloc(AVCODEC_MAX_AUDIO_FRAME_SIZE +
            FF_INPUT_BUFFER_PADDING_SIZE);
    AVPacket* pTempPacket = new AVPacket;
    av_init_packet(pTempPacket);
    pTempPacket->data = pPacket->data;
    pTempPacket->size = pPacket->size;
    while (pTempPacket->size > 0) {
        int bytesDecoded = AVCODEC_MAX_AUDIO_FRAME_SIZE;
#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(52, 31, 0)
        int bytesConsumed = avcodec_decode_audio3(m_pAStream->codec, pDecodedData,
                &bytesDecoded, pTempPacket);
#else
        int bytesConsumed = avcodec_decode_audio2(m_pAStream->codec, pDecodedData,
                &bytesDecoded, pTempPacket->data, pTempPacket->size);
#endif
        AVG_ASSERT(bytesConsumed != 0);
        if (bytesConsumed < 0) {
            // Error decoding -> throw away current packet.
            bytesDecoded = 0;
            pTempPacket->size = 0;
        } else {
            pTempPacket->data += bytesConsumed;
            pTempPacket->size -= bytesConsumed;
        }
        if (bytesDecoded > 0) {
            int framesDecoded = bytesDecoded/(m_pAStream->codec->channels*sizeof(short));
            AudioBufferPtr pBuffer;
            bool bNeedsResample = (m_InputSampleRate != m_AP.m_SampleRate || 
                    m_pAStream->codec->channels != m_AP.m_Channels);
            if (bNeedsResample) {
                pBuffer = resampleAudio(pDecodedData, framesDecoded);
            } else {
                pBuffer = AudioBufferPtr(new AudioBuffer(framesDecoded, m_AP));
                memcpy(pBuffer->getData(), pDecodedData, bytesDecoded);
            }
            m_LastFrameTime += float(pBuffer->getNumFrames())/m_AP.m_SampleRate;
            pushAudioMsg(pBuffer, m_LastFrameTime);
        }
    }
    av_free(pDecodedData);
    delete pTempPacket;
}

void AudioDecoderThread::handleSeekDone(AVPacket* pPacket)
{
    m_MsgQ.clear();
    m_LastFrameTime = float(pPacket->dts*av_q2d(m_pAStream->time_base))
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
    m_LastFrameTime = float(pPacket->dts*av_q2d(m_pAStream->time_base))
            - m_AudioStartTimestamp;
    if (m_LastFrameTime-0.01f > m_SeekTime) {
        pushSeekDone(m_LastFrameTime, m_SeekSeqNum);
        m_State = DECODING;
    }
}

void AudioDecoderThread::close()
{
    m_MsgQ.clear();
    stop();
}

AudioBufferPtr AudioDecoderThread::resampleAudio(short* pDecodedData, int framesDecoded)
{
    if (!m_pAudioResampleContext) {
#if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(52, 24, 0)
        m_pAudioResampleContext = av_audio_resample_init(m_AP.m_Channels, 
                m_pAStream->codec->channels, m_AP.m_SampleRate, m_InputSampleRate,
                SAMPLE_FMT_S16, SAMPLE_FMT_S16, 16, 10, 0, 0.8);
#else
        m_pAudioResampleContext = audio_resample_init(m_AP.m_Channels, 
                m_pAStream->codec->channels, m_AP.m_SampleRate, m_InputSampleRate);
#endif        
    }

    short pResampledData[AVCODEC_MAX_AUDIO_FRAME_SIZE/2];
    int framesResampled = audio_resample(m_pAudioResampleContext, pResampledData,
            pDecodedData, framesDecoded);
    AudioBufferPtr pBuffer(new AudioBuffer(framesResampled, m_AP));
    memcpy(pBuffer->getData(), pResampledData, 
            framesResampled*m_AP.m_Channels*sizeof(short));
    return pBuffer;
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

}
