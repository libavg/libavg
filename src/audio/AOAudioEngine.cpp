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

#include "AOAudioEngine.h"

#include "../base/Exception.h"
#include "../base/Logger.h"

#include <iostream>

namespace avg {

using namespace std;
using namespace boost;

AOAudioEngine* AOAudioEngine::s_pInstance = 0;

AOAudioEngine* AOAudioEngine::get()
{
    return s_pInstance;
}

AOAudioEngine::AOAudioEngine(const AudioParams& ap, float volume)
    : m_enabled(false),
      m_AP(ap),
      m_aoAudioThread(0)
{
    AVG_ASSERT(s_pInstance == 0);
    ao_initialize();

    m_cmdQueue = AOAudioEngineThread::CQueuePtr(new AOAudioEngineThread::CQueue);
    m_aoAudioThread = new boost::thread(AOAudioEngineThread(*m_cmdQueue, ap, volume));
    s_pInstance = this;
}

AOAudioEngine::~AOAudioEngine()
{
    if (m_aoAudioThread) {
        m_aoAudioThread->join();
        delete m_aoAudioThread;
        m_aoAudioThread = 0;
    }
    ao_shutdown();
}

int AOAudioEngine::getChannels()
{
    return m_AP.m_Channels;
}

int AOAudioEngine::getSampleRate()
{
    return m_AP.m_SampleRate;
}

const AudioParams * AOAudioEngine::getParams()
{
    if (m_enabled) {
        return &m_AP;
    } else {
        return 0;
    }
}

void AOAudioEngine::teardown()
{
    m_cmdQueue->pushCmd(boost::bind(&AOAudioEngineThread::playAudio, _1, false));
}

void AOAudioEngine::setAudioEnabled(bool bEnabled)
{
    m_enabled = bEnabled;
}

void AOAudioEngine::play()
{
    m_cmdQueue->pushCmd(boost::bind(&AOAudioEngineThread::playAudio, _1, true));
}

void AOAudioEngine::pause()
{
    m_cmdQueue->pushCmd(boost::bind(&AOAudioEngineThread::playAudio, _1, false));
}

void AOAudioEngine::addSource(IAudioSource* pSource)
{
    m_cmdQueue->pushCmd(boost::bind(&AOAudioEngineThread::addSource, _1, pSource));
}

void AOAudioEngine::removeSource(IAudioSource* pSource)
{
    m_cmdQueue->pushCmd(boost::bind(&AOAudioEngineThread::removeSource, _1, pSource));
}

void AOAudioEngine::setVolume(float volume)
{
    m_cmdQueue->pushCmd(boost::bind(&AOAudioEngineThread::updateVolume, _1, volume));
}

}
