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

#include "AsyncDemuxer.h"

#include "../base/Logger.h"
#include "../base/TimeSource.h"

// In Audio frames.
#define AUDIO_BUFFER_SIZE 256

using namespace std;

namespace avg {

AudioDecoderThread::AudioDecoderThread(CQueue& cmdQ, AudioMsgQueue& msgQ, 
        FFMpegDecoderPtr pDecoder, const AudioParams& ap)
    : WorkerThread<AudioDecoderThread>(string("AudioDecoderThread"), cmdQ),
      m_MsgQ(msgQ),
      m_AP(ap),
      m_pAudioResampleContext(0),
      m_bEOF(false)
{
    m_pCurAudioPacket = 0;
    m_pTempAudioPacket = 0;
    m_LastFrameTime = 0;
    m_AudioStartTimestamp = 0;

    m_pAStream = pDecoder->getAudioStream(); 
    m_AStreamIndex = pDecoder->getAStreamIndex();
    m_pDemuxer = dynamic_cast<AsyncDemuxer*>(pDecoder->getDemuxer());
    if (m_pAStream->start_time != (long long)AV_NOPTS_VALUE) {
        m_AudioStartTimestamp = float(av_q2d(m_pAStream->time_base)*m_pAStream->start_time);
    }
    m_EffectiveSampleRate = (int)(m_pAStream->codec->sample_rate);
}

AudioDecoderThread::~AudioDecoderThread()
{
    deleteCurAudioPacket();

    if (m_pAudioResampleContext) {
        audio_resample_close(m_pAudioResampleContext);
        m_pAudioResampleContext = 0;
    }

    m_LastFrameTime = 0;
    m_AudioStartTimestamp = 0;
}

bool AudioDecoderThread::work() 
{
    if (m_bEOF) {
        msleep(10);
        int seqNum;
        float seekTime = m_pDemuxer->isSeekDone(m_AStreamIndex, seqNum, false);
        if (seekTime != -1) {
            m_bEOF = false;
            handleSeekDone(seqNum, seekTime);
        }
        if (m_pDemuxer->isClosed(m_AStreamIndex)) {
            close();
        }
    } else {
        AudioBufferPtr pBuffer = getAudioBuffer();
        if (pBuffer) {
            pushAudioMsg(pBuffer, m_LastFrameTime);
        } else {
            if (m_SeekTime != -1) {
                handleSeekDone(-1, m_SeekTime);
                m_SeekTime = -1;
            }
            if (m_pDemuxer->isClosed(m_AStreamIndex)) {
                close();
            }
        }
    }
    ThreadProfiler::get()->reset();
    return true;
}

void AudioDecoderThread::close()
{
    m_MsgQ.clear();
    stop();
}

AudioBufferPtr AudioDecoderThread::getAudioBuffer()
{
    short* pDecodedData = (short*)av_malloc(AVCODEC_MAX_AUDIO_FRAME_SIZE +
            FF_INPUT_BUFFER_PADDING_SIZE);

    int bytesConsumed = 0;
    int bytesDecoded = 0;
    while (bytesDecoded == 0) {
        if (!m_pCurAudioPacket) {
            int seqNum;
            m_SeekTime = m_pDemuxer->isSeekDone(m_AStreamIndex, seqNum);
            if (m_SeekTime != -1) {
                av_free(pDecodedData);
                return AudioBufferPtr();
            }
            m_pCurAudioPacket = m_pDemuxer->getPacket(m_AStreamIndex);
            if (!m_pCurAudioPacket) {
                handleNoPacketMsg();
                av_free(pDecodedData);
                return AudioBufferPtr();
            }
            m_pTempAudioPacket = new AVPacket;
            av_init_packet(m_pTempAudioPacket);
            m_pTempAudioPacket->data = m_pCurAudioPacket->data;
            m_pTempAudioPacket->size = m_pCurAudioPacket->size;
        }
        bytesDecoded = AVCODEC_MAX_AUDIO_FRAME_SIZE;
#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(52, 31, 0)
        bytesConsumed = avcodec_decode_audio3(m_pAStream->codec, pDecodedData,
                &bytesDecoded, m_pTempAudioPacket);
#else
        bytesConsumed = avcodec_decode_audio2(m_pAStream->codec, pDecodedData,
                &bytesDecoded, m_pTempAudioPacket->data, m_pTempAudioPacket->size);
#endif
        if (bytesConsumed < 0) {
            // Error decoding -> throw away current packet.
            bytesDecoded = 0;
            deleteCurAudioPacket();
        } else {
            m_pTempAudioPacket->data += bytesConsumed;
            m_pTempAudioPacket->size -= bytesConsumed;

            if (m_pTempAudioPacket->size <= 0) {
                deleteCurAudioPacket();
            }
        }
    }
    int framesDecoded = bytesDecoded/(m_pAStream->codec->channels*sizeof(short));
    AudioBufferPtr pBuffer;
    bool bNeedsResample = (m_EffectiveSampleRate != m_AP.m_SampleRate || 
            m_pAStream->codec->channels != m_AP.m_Channels);
    if (bNeedsResample) {
        pBuffer = resampleAudio(pDecodedData, framesDecoded);
    } else {
        pBuffer = AudioBufferPtr(new AudioBuffer(framesDecoded, m_AP));
        memcpy(pBuffer->getData(), pDecodedData, bytesDecoded);
    }
    m_LastFrameTime += float(pBuffer->getNumFrames())/m_AP.m_SampleRate;
    av_free(pDecodedData);
    return pBuffer;
}

AudioBufferPtr AudioDecoderThread::resampleAudio(short* pDecodedData, int framesDecoded)
{
    if (!m_pAudioResampleContext) {
#if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(52, 24, 0)
        m_pAudioResampleContext = av_audio_resample_init(m_AP.m_Channels, 
                m_pAStream->codec->channels, m_AP.m_SampleRate, m_EffectiveSampleRate,
                SAMPLE_FMT_S16, SAMPLE_FMT_S16, 16, 10, 0, 0.8);
#else
        m_pAudioResampleContext = audio_resample_init(m_AP.m_Channels, 
                m_pAStream->codec->channels, m_AP.m_SampleRate, m_EffectiveSampleRate);
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

void AudioDecoderThread::handleSeekDone(int seqNum, float seekTime)
{
    avcodec_flush_buffers(m_pAStream->codec);
    m_MsgQ.clear();

    // Get the next packet so we know at what time we actually arrived.
    deleteCurAudioPacket();
    m_pCurAudioPacket = m_pDemuxer->getPacket(m_AStreamIndex);
    if (!m_pCurAudioPacket) {
        handleNoPacketMsg();
    } else {
        m_LastFrameTime = float(m_pCurAudioPacket->dts*av_q2d(m_pAStream->time_base))
                - m_AudioStartTimestamp;

        if (fabs(m_LastFrameTime - seekTime) < 0.05) {
            pushSeekDone(m_LastFrameTime, seqNum);
        } else { 
            if (m_LastFrameTime-0.05f < seekTime) {
                // Received frame that's earlier than the destination, so throw away frames
                // until the time is correct.
                while (m_LastFrameTime-0.05f < seekTime) {
                    deleteCurAudioPacket();
                    float seekTime = m_pDemuxer->isSeekDone(m_AStreamIndex, seqNum);
                    AVG_ASSERT(seekTime == -1);
                    m_pCurAudioPacket = m_pDemuxer->getPacket(m_AStreamIndex);
                    if (!m_pCurAudioPacket) {
                        handleNoPacketMsg();
                        return;
                    }
                    AVG_ASSERT(m_pCurAudioPacket);
                    m_LastFrameTime = 
                            float(m_pCurAudioPacket->dts*av_q2d(m_pAStream->time_base))
                            - m_AudioStartTimestamp;
                }
        
                pushSeekDone(m_LastFrameTime, seqNum);
            } else {
                // Received frame that's too late, so insert a buffer of silence to compensate.
                pushSeekDone(m_LastFrameTime, seqNum);

                // send empty buffer 
                int numDelaySamples = int((m_LastFrameTime - seekTime)*m_AP.m_SampleRate);
                AudioBufferPtr pBuffer(new AudioBuffer(numDelaySamples, m_AP));
                pBuffer->clear();
                m_LastFrameTime = seekTime;
                pushAudioMsg(pBuffer, m_LastFrameTime);
            }
        }

        AVG_ASSERT(m_pCurAudioPacket);
        m_pTempAudioPacket = new AVPacket;
        av_init_packet(m_pTempAudioPacket);
        m_pTempAudioPacket->data = m_pCurAudioPacket->data;
        m_pTempAudioPacket->size = m_pCurAudioPacket->size;
    }
}

void AudioDecoderThread::handleNoPacketMsg()
{
    // Either close or eof
    if (!m_pDemuxer->isClosed(m_AStreamIndex)) {
        // Early out: EOF during seek
        m_bEOF = true;
        pushEOF();
    }
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

void AudioDecoderThread::deleteCurAudioPacket()
{
    if (m_pCurAudioPacket) {
        av_free_packet(m_pCurAudioPacket);
        delete m_pCurAudioPacket;
        m_pCurAudioPacket = 0;
    }
    if (m_pTempAudioPacket) {
        delete m_pTempAudioPacket;
        m_pTempAudioPacket = 0;
    }
}

}
