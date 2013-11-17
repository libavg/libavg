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

#include "AudioEngine.h"

#include "Dynamics.h"

#include "../base/Exception.h"
#include "../base/Logger.h"

#include <iostream>

using namespace std;
using namespace boost;

namespace avg {

AudioEngine* AudioEngine::s_pInstance = 0;

AudioEngine* AudioEngine::get()
{
    return s_pInstance;
}

AudioEngine::AudioEngine()
    : m_pTempBuffer(),
      m_pMixBuffer(0),
      m_pLimiter(0),
      m_bEnabled(true),
      m_Volume(1)
{
    AVG_ASSERT(s_pInstance == 0);
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) == -1) {
        AVG_LOG_ERROR("Can't init SDL audio subsystem.");
        exit(-1);
    }
    s_pInstance = this;
}

AudioEngine::~AudioEngine()
{
    if (m_pMixBuffer) {
        delete[] m_pMixBuffer;
    }
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    m_AudioSources.clear();
}

int AudioEngine::getChannels()
{
    return m_AP.m_Channels;
}

int AudioEngine::getSampleRate()
{
    return m_AP.m_SampleRate;
}

const AudioParams * AudioEngine::getParams()
{
    if (isEnabled()) {
        return &m_AP;
    } else {
        return 0;
    }
}

void AudioEngine::init(const AudioParams& ap, float volume) 
{
    m_Volume = volume;
    m_AP = ap;
    Dynamics<float, 2>* pLimiter = new Dynamics<float, 2>(float(m_AP.m_SampleRate));
    pLimiter->setThreshold(0.f); // in dB
    pLimiter->setAttackTime(0.f); // in seconds
    pLimiter->setReleaseTime(0.05f); // in seconds
    pLimiter->setRmsTime(0.f); // in seconds
    pLimiter->setRatio(std::numeric_limits<float>::infinity());
    pLimiter->setMakeupGain(0.f); // in dB
    m_pLimiter = pLimiter;
    
    SDL_AudioSpec desired;
    desired.freq = m_AP.m_SampleRate;
    desired.format = AUDIO_S16SYS;
    desired.channels = m_AP.m_Channels;
    desired.silence = 0;
    desired.samples = m_AP.m_OutputBufferSamples;
    desired.callback = audioCallback;
    desired.userdata = this;

    int err = SDL_OpenAudio(&desired, 0);
    if (err < 0) {
        static bool bWarned = false;
        if (!bWarned) {
            AVG_TRACE(Logger::category::CONFIG, Logger::severity::WARNING,
                    "Can't open audio: " << SDL_GetError());
            bWarned = true;
        }
    }
}

void AudioEngine::teardown()
{
    {
        lock_guard lock(m_Mutex);
        SDL_PauseAudio(1);
    }
    // Optimized away - takes too long.
//    SDL_CloseAudio();

    m_AudioSources.clear();
    if (m_pLimiter) {
        delete m_pLimiter;
        m_pLimiter = 0;
    }
}

void AudioEngine::setAudioEnabled(bool bEnabled)
{
    SDL_LockAudio();
    lock_guard lock(m_Mutex);
    AVG_ASSERT(m_AudioSources.empty());
    m_bEnabled = bEnabled;
    if (m_bEnabled) {
        play();
    } else {
        pause();
    }
    SDL_UnlockAudio();
}

void AudioEngine::play()
{
    SDL_PauseAudio(0);
}

void AudioEngine::pause()
{
    SDL_PauseAudio(1);
}

int AudioEngine::addSource(AudioMsgQueue& dataQ, AudioMsgQueue& statusQ)
{
    SDL_LockAudio();
    lock_guard lock(m_Mutex);
    static int nextID = -1;
    nextID++;
    AudioSourcePtr pSrc(new AudioSource(dataQ, statusQ, m_AP.m_SampleRate));
    m_AudioSources[nextID] = pSrc;
    SDL_UnlockAudio();
    return nextID;
}

void AudioEngine::removeSource(int id)
{
    SDL_LockAudio();
    lock_guard lock(m_Mutex);
    int numErased = m_AudioSources.erase(id);
    AVG_ASSERT(numErased == 1);
    SDL_UnlockAudio();
}

void AudioEngine::pauseSource(int id)
{
    lock_guard lock(m_Mutex);
    AudioSourceMap::iterator itSource = m_AudioSources.find(id);
    AVG_ASSERT(itSource != m_AudioSources.end());
    AudioSourcePtr pSource = itSource->second;
    pSource->pause();
}

void AudioEngine::playSource(int id)
{
    lock_guard lock(m_Mutex);
    AudioSourceMap::iterator itSource = m_AudioSources.find(id);
    AVG_ASSERT(itSource != m_AudioSources.end());
    AudioSourcePtr pSource = itSource->second;
    pSource->play();
}

void AudioEngine::notifySeek(int id)
{
    lock_guard lock(m_Mutex);
    AudioSourceMap::iterator itSource = m_AudioSources.find(id);
    AVG_ASSERT(itSource != m_AudioSources.end());
    AudioSourcePtr pSource = itSource->second;
    pSource->notifySeek();
}

void AudioEngine::setSourceVolume(int id, float volume)
{
    lock_guard lock(m_Mutex);
    AudioSourceMap::iterator itSource = m_AudioSources.find(id);
    AVG_ASSERT(itSource != m_AudioSources.end());
    AudioSourcePtr pSource = itSource->second;
    pSource->setVolume(volume);
}

void AudioEngine::setVolume(float volume)
{
    SDL_LockAudio();
    lock_guard lock(m_Mutex);
    m_Volume = volume;
    SDL_UnlockAudio();
}

float AudioEngine::getVolume() const
{
    return m_Volume;
}

bool AudioEngine::isEnabled() const
{
    return m_bEnabled;
}
        
void AudioEngine::mixAudio(Uint8 *pDestBuffer, int destBufferLen)
{
    int numFrames = destBufferLen/(2*getChannels()); // 16 bit samples.

    if (m_AudioSources.size() == 0) {
        return;
    }
    if (!m_pTempBuffer || m_pTempBuffer->getNumFrames() < numFrames) {
        if (m_pTempBuffer) {
            delete[] m_pMixBuffer;
        }
        m_pTempBuffer = AudioBufferPtr(new AudioBuffer(numFrames, m_AP));
        m_pMixBuffer = new float[getChannels()*numFrames];
    }

    for (int i = 0; i < getChannels()*numFrames; ++i) {
        m_pMixBuffer[i]=0;
    }
    {
        lock_guard lock(m_Mutex);
        AudioSourceMap::iterator it;
        for (it = m_AudioSources.begin(); it != m_AudioSources.end(); it++) {
            m_pTempBuffer->clear();
            it->second->fillAudioBuffer(m_pTempBuffer);
            addBuffers(m_pMixBuffer, m_pTempBuffer);
        }
    }
    calcVolume(m_pMixBuffer, numFrames*getChannels(), getVolume());
    for (int i = 0; i < numFrames; ++i) {
        m_pLimiter->process(m_pMixBuffer+i*getChannels());
        for (int j = 0; j < getChannels(); ++j) {
            ((short*)pDestBuffer)[i*2+j]=short(m_pMixBuffer[i*2+j]*32768);
        }
    }
}

void AudioEngine::audioCallback(void *userData, Uint8 *audioBuffer, int audioBufferLen)
{
    AudioEngine *pThis = (AudioEngine*)userData;
    pThis->mixAudio(audioBuffer, audioBufferLen);
}

void AudioEngine::addBuffers(float *pDest, AudioBufferPtr pSrc)
{
    int numFrames = pSrc->getNumFrames();
    short * pData = pSrc->getData();
    for(int i = 0; i < numFrames*getChannels(); ++i) {
        pDest[i] += pData[i]/32768.0f;
    }
}

void AudioEngine::calcVolume(float *pBuffer, int numSamples, float volume)
{
    // TODO: We need a VolumeFader class that keeps state.
    for(int i = 0; i < numSamples; ++i) {
        pBuffer[i] *= volume;
    }
}

}
