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

#include "AudioSource.h"
#include "AudioEngine.h"

#include <string>
#include <algorithm>

using namespace std;

namespace avg {

AudioSource::AudioSource(AudioMsgQueuePtr pMsgQ, AudioMsgQueuePtr pStatusQ, 
        int sampleRate)
    : m_pMsgQ(pMsgQ),
      m_pStatusQ(pStatusQ),
      m_SampleRate(sampleRate)
{
}

AudioSource::~AudioSource()
{
}

void AudioSource::fillAudioBuffer(AudioBufferPtr pBuffer)
{
    unsigned char* pDest = (unsigned char *)(pBuffer->getData());
    int framesLeftToFill = pBuffer->getNumFrames();
    AudioMsgPtr pMsg;
    while (framesLeftToFill > 0) {
        int framesLeftInBuffer = 0;
        if (m_pInputAudioBuffer) {
            framesLeftInBuffer = m_pInputAudioBuffer->getNumFrames() - m_CurInputAudioPos;
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

            m_LastAudioFrameTime += float(framesToCopy);
//            cerr << "  " << m_LastAudioFrameTime << endl;
        }
        if (framesLeftToFill != 0) {
            bool bContinue = processNextMsg();
            if (!bContinue) {
                framesLeftToFill = 0;
            }
        }
    }
    AudioMsgPtr pStatusMsg(new AudioMsg);
    pStatusMsg->setAudioTime(m_LastAudioFrameTime);
    m_pStatusQ->push(pStatusMsg);
}
    
bool AudioSource::processNextMsg()
{
    AudioMsgPtr pMsg = m_pMsgQ->pop(false);
    if (pMsg) {
        switch (pMsg->getType()) {
            case AudioMsg::AUDIO:
                m_pInputAudioBuffer = pMsg->getAudioBuffer();
                m_CurInputAudioPos = 0;
                m_LastAudioFrameTime = pMsg->getAudioTime();
//                cerr << "  New buffer: " << m_pInputAudioBuffer->getNumFrames() << endl;
                return true;
            case AudioMsg::END_OF_FILE: {
                AudioMsgPtr pStatusMsg(new AudioMsg);
                pStatusMsg->setEOF();
                m_pStatusQ->push(pStatusMsg);
                return false;
            }
            case AudioMsg::SEEK_DONE:
                m_LastAudioFrameTime = pMsg->getSeekAudioFrameTime()*m_SampleRate;
                return true;
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
