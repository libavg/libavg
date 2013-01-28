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

#include "AudioSource.h"
#include "AudioEngine.h"

#include <string>
#include <algorithm>

using namespace std;

namespace avg {

AudioSource::AudioSource(AudioMsgQueue& msgQ, AudioMsgQueue& statusQ, int sampleRate)
    : m_MsgQ(msgQ),
      m_StatusQ(statusQ),
      m_SampleRate(sampleRate),
      m_bPaused(false),
      m_bSeeking(false),
      m_Volume(1.0),
      m_LastVolume(1.0)
{
}

AudioSource::~AudioSource()
{
}

void AudioSource::pause()
{
    m_bPaused = true;
}

void AudioSource::play()
{
    m_bPaused = false;
}

void AudioSource::notifySeek()
{
    while (m_bSeeking) {
        processNextMsg(true);
    }
    m_bSeeking = true;
}
    
void AudioSource::setVolume(float volume)
{
    m_Volume = volume;
}

void AudioSource::fillAudioBuffer(AudioBufferPtr pBuffer)
{
    bool bContinue = true;
    while (bContinue && m_bSeeking) {
        bContinue = processNextMsg(false);
    }
    if (!m_bPaused) {
        unsigned char* pDest = (unsigned char *)(pBuffer->getData());
        int framesLeftToFill = pBuffer->getNumFrames();
        AudioMsgPtr pMsg;
        while (framesLeftToFill > 0) {
            int framesLeftInBuffer = 0;
            if (m_pInputAudioBuffer) {
                framesLeftInBuffer = m_pInputAudioBuffer->getNumFrames()
                        - m_CurInputAudioPos;
            }
            while (framesLeftInBuffer > 0 && framesLeftToFill > 0) {
                int framesToCopy = min(framesLeftToFill, framesLeftInBuffer);
    //            cerr << "framesToCopy: " << framesToCopy << endl;
                char * pInputPos = (char*)m_pInputAudioBuffer->getData() + 
                        m_CurInputAudioPos*pBuffer->getFrameSize();
                int bytesToCopy = framesToCopy*pBuffer->getFrameSize();
                memcpy(pDest, pInputPos, bytesToCopy);
                m_CurInputAudioPos += framesToCopy;
                framesLeftToFill -= framesToCopy;
                framesLeftInBuffer -= framesToCopy;
                pDest += bytesToCopy;

                m_LastTime += framesToCopy/m_SampleRate;
    //            cerr << "  " << m_LastTime << endl;
            }
            if (framesLeftToFill != 0) {
                bool bContinue = processNextMsg(false);
                if (!bContinue) {
                    framesLeftToFill = 0;
                }
            }
        }
        pBuffer->volumize(m_LastVolume, m_Volume);
        m_LastVolume = m_Volume;

        AudioMsgPtr pStatusMsg(new AudioMsg);
        pStatusMsg->setAudioTime(m_LastTime);
        m_StatusQ.push(pStatusMsg);
    }
}
    
bool AudioSource::processNextMsg(bool bWait)
{
    AudioMsgPtr pMsg = m_MsgQ.pop(bWait);
    if (pMsg) {
        switch (pMsg->getType()) {
            case AudioMsg::AUDIO:
                m_pInputAudioBuffer = pMsg->getAudioBuffer();
                m_CurInputAudioPos = 0;
                m_LastTime = pMsg->getAudioTime();
//                cerr << "  New buffer: " << m_LastTime << endl;
                return true;
            case AudioMsg::END_OF_FILE: {
//                cerr << "        AudioSource: EOF" << endl;
                m_bSeeking = false;
                AudioMsgPtr pStatusMsg(new AudioMsg);
                pStatusMsg->setEOF();
                m_StatusQ.push(pStatusMsg);
                return false;
            }
            case AudioMsg::SEEK_DONE: {
//                cerr << "        AudioSource: SEEK_DONE" << endl;
                m_bSeeking = false;
                m_pInputAudioBuffer = AudioBufferPtr();
                m_LastTime = pMsg->getSeekTime();
                AudioMsgPtr pStatusMsg(new AudioMsg);
                pStatusMsg->setSeekDone(pMsg->getSeekSeqNum(), m_LastTime);
                m_StatusQ.push(pStatusMsg);
                return true;
            }
            default:
                AVG_ASSERT(false);
                return false;
        }
    } else {
        //                cerr << "no pop" << endl;
        return false;
    }
}

}
