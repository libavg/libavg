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
//  Original author of this file is Nick Hebner (hebnern@gmail.com).
//

#include "AudioDecoderThread.h"
#include "AudioVideoMsg.h"
#include "SeekDoneVideoMsg.h"
#include "EOFVideoMsg.h"

#include "../base/Logger.h"

#define AUDIO_BUFFER_SIZE 1024

using namespace std;

namespace avg {

AudioDecoderThread::AudioDecoderThread(CmdQueue& CmdQ, VideoMsgQueue& MsgQ, 
        VideoDecoderPtr pDecoder)
    : WorkerThread<AudioDecoderThread>(string("AudioDecoderThread"), CmdQ),
      m_BufferSize(AUDIO_BUFFER_SIZE),
      m_MsgQ(MsgQ),
      m_pDecoder(pDecoder)
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
        AudioVideoMsgPtr pVMsg = AudioVideoMsgPtr(new AudioVideoMsg(m_BufferSize, 
                    m_pDecoder->getCurTime(SS_AUDIO)));
        int BytesWritten = m_pDecoder->fillAudioFrame(pVMsg->getBuffer(), pVMsg->getSize());
        if (BytesWritten != pVMsg->getSize()) {
            AudioVideoMsgPtr pOldMsg = pVMsg;
            pVMsg = AudioVideoMsgPtr(new AudioVideoMsg(BytesWritten, 
                        m_pDecoder->getCurTime(SS_AUDIO)));
            memcpy(pVMsg->getBuffer(), pOldMsg->getBuffer(), BytesWritten);
            m_MsgQ.push(pVMsg);
        } else {
            m_MsgQ.push(pVMsg);
        }
        if (m_pDecoder->isEOF(SS_AUDIO)) {
            m_MsgQ.push(VideoMsgPtr(new EOFVideoMsg())); 
        }
    }
    return true;
}

void AudioDecoderThread::seek(long long DestTime)
{
    try {
        while (!m_MsgQ.empty()) {
            m_MsgQ.pop(false);
        }
    } catch (Exception&) {
    }
    
    bool performSeek = !m_pDecoder->hasVideo();
    long long VideoFrameTime = -1;
    long long AudioFrameTime = -1;
    if (performSeek) {
        m_pDecoder->seek(DestTime);
        AudioFrameTime = m_pDecoder->getCurTime(SS_AUDIO);
        if (m_pDecoder->hasVideo()) {
            VideoFrameTime = m_pDecoder->getCurTime(SS_VIDEO);
        }
    }
    
    m_MsgQ.push(VideoMsgPtr(new SeekDoneVideoMsg(performSeek, 
                VideoFrameTime, AudioFrameTime)));
}

}
