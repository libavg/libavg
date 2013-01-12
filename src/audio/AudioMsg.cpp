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

#include "AudioMsg.h"

#include "../base/ObjectCounter.h"
#include "../base/Exception.h"

#include <iostream>

using namespace std;

namespace avg {

AudioMsg::AudioMsg()
    : m_MsgType(NONE)
{
    ObjectCounter::get()->incRef(&typeid(*this));
}

AudioMsg::~AudioMsg()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void AudioMsg::setAudio(AudioBufferPtr pAudioBuffer, float audioTime)
{
    AVG_ASSERT(pAudioBuffer);
    setType(AUDIO);
    m_pAudioBuffer = pAudioBuffer;
    m_AudioTime = audioTime;
}

void AudioMsg::setAudioTime(float audioTime)
{
    setType(AUDIO_TIME);
    m_AudioTime = audioTime;
}

void AudioMsg::setEOF()
{
    setType(END_OF_FILE);
}

void AudioMsg::setError(const Exception& ex)
{
    setType(ERROR);
    m_pEx = new Exception(ex);
}

void AudioMsg::setSeekDone(float seekTime)
{
    setType(SEEK_DONE);
    m_SeekTime = seekTime;
}

AudioMsg::MsgType AudioMsg::getType()
{
    return m_MsgType;
}

AudioBufferPtr AudioMsg::getAudioBuffer() const
{
    AVG_ASSERT(m_MsgType == AUDIO);
    return m_pAudioBuffer;
}

float AudioMsg::getAudioTime() const
{
    AVG_ASSERT(m_MsgType == AUDIO_TIME || m_MsgType == AUDIO);
    return m_AudioTime;
}

const Exception& AudioMsg::getException() const
{
    AVG_ASSERT(m_MsgType == ERROR);
    return *m_pEx;
}

float AudioMsg::getSeekTime()
{
    AVG_ASSERT(m_MsgType == SEEK_DONE);
    return m_SeekTime;
}
    
void AudioMsg::dump()
{
    switch (m_MsgType) {
        case NONE:
            cerr << "NONE" << endl;
            break;
        case AUDIO:
            cerr << "AUDIO" << endl;
            break;
        case AUDIO_TIME:
            cerr << "AUDIO_TIME" << endl;
            break;
        case END_OF_FILE:
            cerr << "END_OF_FILE" << endl;
            break;
        case ERROR:
            cerr << "ERROR" << endl;
            break;
        case FRAME:
            cerr << "FRAME" << endl;
            break;
        case VDPAU_FRAME:
            cerr << "VDPAU_FRAME" << endl;
            break;
        case SEEK_DONE:
            cerr << "SEEK_DONE" << endl;
            break;
        case PACKET:
            cerr << "PACKET" << endl;
            break;
        default:
            AVG_ASSERT(false);
            break;
    }
}
    
void AudioMsg::setType(MsgType msgType)
{
    AVG_ASSERT(m_MsgType == NONE);
    m_MsgType = msgType;
}



}

