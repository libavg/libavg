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

// In Audio frames.
#define AUDIO_BUFFER_SIZE 256

using namespace std;

namespace avg {

AudioDecoderThread::AudioDecoderThread(CQueue& cmdQ, AudioMsgQueue& msgQ, 
        FFMpegDecoderPtr pDecoder, const AudioParams& ap)
    : WorkerThread<AudioDecoderThread>(string("AudioDecoderThread"), cmdQ),
      m_MsgQ(msgQ),
      m_pDecoder(pDecoder),
      m_AP(ap)
{
}

AudioDecoderThread::~AudioDecoderThread()
{
}

bool AudioDecoderThread::work() 
{
    if (m_pDecoder->isEOF(SS_AUDIO)) {
        // replace this with waitForMessage()
        msleep(10);
    } else {
        AudioBufferPtr pBuffer = m_pDecoder->getAudioBuffer();
        if (m_pDecoder->isAudioSeekDone()) {
            VideoMsgPtr pMsg(new VideoMsg());
            float time = m_pDecoder->getCurTime(SS_AUDIO);
            pMsg->setSeekDone(-1, time);
            m_MsgQ.push(pMsg);
        }
        VideoMsgPtr pVMsg = VideoMsgPtr(new VideoMsg());
        if (pBuffer) {
            float curTime = m_pDecoder->getCurTime(SS_AUDIO);
            pVMsg->setAudio(pBuffer, curTime);
        } else {
            AVG_ASSERT(m_pDecoder->isEOF(SS_AUDIO));
            pVMsg->setEOF();
        }
        m_MsgQ.push(pVMsg);
        ThreadProfiler::get()->reset();
    }
    return true;
}

void AudioDecoderThread::seek(float destTime)
{
    m_MsgQ.clear();
    m_pDecoder->seek(destTime);
}

void AudioDecoderThread::setVolume(float volume)
{
    m_pDecoder->setVolume(volume);
}

}
