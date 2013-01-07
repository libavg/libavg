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
      m_Volume(1.0),
      m_LastVolume(1.0),
      m_bEOF(false)
{
//    cerr << "        AudioDecoderThread" << endl;
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
//    cerr << "        ~AudioDecoderThread" << endl;
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
        // replace this with waitForMessage()
        msleep(10);
    } else {
        bool bSeekDone;
        AudioBufferPtr pBuffer = getAudioBuffer(bSeekDone);
        if (bSeekDone) {
            VideoMsgPtr pMsg(new VideoMsg());
            pMsg->setSeekDone(m_LastFrameTime);
            m_MsgQ.push(pMsg);
        }
        VideoMsgPtr pVMsg = VideoMsgPtr(new VideoMsg());
        if (pBuffer) {
            pVMsg->setAudio(pBuffer, m_LastFrameTime);
        } else {
            AVG_ASSERT(m_bEOF);
            pVMsg->setEOF();
        }
        m_MsgQ.push(pVMsg);
        ThreadProfiler::get()->reset();
    }
    return true;
}

void AudioDecoderThread::seek(float destTime, bool bSeekDemuxer)
{
    m_MsgQ.clear();
    if (bSeekDemuxer) {
        m_pDemuxer->seek(destTime + m_AudioStartTimestamp);
    }
    m_bEOF = false;
}

void AudioDecoderThread::setVolume(float volume)
{
    m_Volume = volume;
}

AudioBufferPtr AudioDecoderThread::getAudioBuffer(bool& bSeekDone)
{
    bSeekDone = false;
//    cerr << "          AudioDecoderThread::getAudioBuffer" << endl;
    short pDecodedData[AVCODEC_MAX_AUDIO_FRAME_SIZE/2];

    int bytesConsumed = 0;
    int bytesDecoded = 0;
    while (bytesDecoded == 0) {
        if (!m_pCurAudioPacket) {
//            cerr << "                  get new packet" << endl;
            bool bNewSeekDone;
            m_pCurAudioPacket = m_pDemuxer->getPacket(m_AStreamIndex, bNewSeekDone);
            if (bNewSeekDone) {
                bSeekDone = true;
                avcodec_flush_buffers(m_pAStream->codec);
                m_pCurAudioPacket = m_pDemuxer->getPacket(m_AStreamIndex, bNewSeekDone);
                m_LastFrameTime =
                        float(m_pCurAudioPacket->dts*av_q2d(m_pAStream->time_base))
                        - m_AudioStartTimestamp;
            }
            if (!m_pCurAudioPacket) {
//                cerr << "                  eof" << endl;
                m_bEOF = true;
                return AudioBufferPtr();
            }
//            cerr << "                  packet size: " << m_pCurAudioPacket->size << endl;
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
//        cerr << "                  avcodec_decode_audio: bytesConsumed=" <<
//                  bytesConsumed << ", bytesDecoded=" << bytesDecoded << endl;
        if (bytesConsumed < 0) {
            // Error decoding -> throw away current packet.
            bytesDecoded = 0;
            deleteCurAudioPacket();
//            cerr << "                  error decoding" << endl;
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
//    cerr << "                  Decoder time: " << m_LastFrameTime << endl;
    pBuffer->volumize(m_LastVolume, m_Volume);
    m_LastVolume = m_Volume;
//    cerr << "                FFMpegDecoder::getAudioBuffer() end" << endl;
    return pBuffer;
}

AudioBufferPtr AudioDecoderThread::resampleAudio(short* pDecodedData, int framesDecoded)
{
    if (!m_pAudioResampleContext) {
//        cerr << "init resample" << endl;
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
//    cerr << "Resample: " << framesDecoded << "->" << framesResampled << endl;
    return pBuffer;
}

void AudioDecoderThread::deleteCurAudioPacket()
{
    if (m_pCurAudioPacket) {
        av_free_packet(m_pCurAudioPacket);
        delete m_pCurAudioPacket;
        m_pCurAudioPacket = 0;
        delete m_pTempAudioPacket;
        m_pTempAudioPacket = 0;
    }
}

}
