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

#include "SDLAudioEngine.h"
#include "Dynamics.h"

#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/ScopeTimer.h"
#include "../base/Profiler.h"

#include <iostream>

namespace avg {

using namespace std;
using namespace boost;

SDLAudioEngine::SDLAudioEngine()
    : m_pTempBuffer(),
      m_pMixBuffer(0),
      m_pLimiter(0)
{
    if (SDL_InitSubSystem(SDL_INIT_AUDIO)==-1) {
        AVG_TRACE(Logger::ERROR, "Can't init SDL audio subsystem.");
        exit(-1);
    }
}

SDLAudioEngine::~SDLAudioEngine()
{
    if (m_pMixBuffer) {
        delete[] m_pMixBuffer;
    }
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

int SDLAudioEngine::getChannels()
{
    return m_AP.m_Channels;
}

int SDLAudioEngine::getSampleRate()
{
    return m_AP.m_SampleRate;
}

const AudioParams & SDLAudioEngine::getParams()
{
    return m_AP;
}

void SDLAudioEngine::init(const AudioParams& AP, double volume) 
{
    AudioEngine::init(AP, volume);
    m_AP = AP;
    Dynamics<double, 2>* pLimiter = new Dynamics<double, 2>(m_AP.m_SampleRate);
    pLimiter->setThreshold(0.); // in dB
    pLimiter->setAttackTime(0.); // in seconds
    pLimiter->setReleaseTime(0.05); // in seconds
    pLimiter->setRmsTime(0.); // in seconds
    pLimiter->setRatio(std::numeric_limits<double>::infinity());
    pLimiter->setMakeupGain(0.); // in dB
    m_pLimiter = pLimiter;
    
    SDL_AudioSpec desired;
    desired.freq = m_AP.m_SampleRate;
    desired.format = AUDIO_S16SYS;
    desired.channels = m_AP.m_Channels;
    desired.silence = 0;
    desired.samples = m_AP.m_OutputBufferSamples;
    desired.callback = audioCallback;
    desired.userdata = this;

    if (SDL_OpenAudio(&desired, 0) < 0) {
      //throw new Exception("Cannot open audio device");
    }
}

void SDLAudioEngine::teardown()
{
    mutex::scoped_lock Lock(m_Mutex);
    SDL_PauseAudio(1);
    SDL_CloseAudio();

    getSources().clear();
    if (m_pLimiter) {
        delete m_pLimiter;
        m_pLimiter = 0;
    }
}

void SDLAudioEngine::setAudioEnabled(bool bEnabled)
{
    mutex::scoped_lock Lock(m_Mutex);
    AudioEngine::setAudioEnabled(bEnabled);
}

void SDLAudioEngine::play()
{
    SDL_PauseAudio(0);
}

void SDLAudioEngine::pause()
{
    SDL_PauseAudio(1);
}

void SDLAudioEngine::addSource(IAudioSource* pSource)
{
    mutex::scoped_lock Lock(m_Mutex);
    AudioEngine::addSource(pSource);
}

void SDLAudioEngine::removeSource(IAudioSource* pSource)
{
    mutex::scoped_lock Lock(m_Mutex);
    AudioEngine::removeSource(pSource);
}

void SDLAudioEngine::setVolume(double volume)
{
    mutex::scoped_lock Lock(m_Mutex);
    AudioEngine::setVolume(volume);
}

void SDLAudioEngine::mixAudio(Uint8 *pDestBuffer, int destBufferLen)
{
    mutex::scoped_lock Lock(m_Mutex);
    int numFrames = destBufferLen/(2*getChannels()); // 16 bit samples.

    if (getSources().size() == 0) {
        return;
    }
    if (!m_pTempBuffer || m_pTempBuffer->getNumFrames() < numFrames) {
        if (m_pTempBuffer) {
            delete[] m_pMixBuffer;
        }
        m_pTempBuffer = AudioBufferPtr(new AudioBuffer(numFrames, m_AP));
        m_pMixBuffer = new double[getChannels()*numFrames];
    }

    for (int i=0; i<getChannels()*numFrames; ++i) {
        m_pMixBuffer[i]=0;
    }
    AudioSourceList::iterator it;
    for(it = getSources().begin(); it != getSources().end(); it++) {
        m_pTempBuffer->clear();
        (*it)->fillAudioBuffer(m_pTempBuffer);
        addBuffers(m_pMixBuffer, m_pTempBuffer);
    }
    calcVolume(m_pMixBuffer, numFrames*getChannels(), getVolume());
    for (int i=0; i<numFrames; ++i) {
        m_pLimiter->process(m_pMixBuffer+i*getChannels());
        for (int j=0; j<getChannels(); ++j) {
            ((short*)pDestBuffer)[i*2+j]=short(m_pMixBuffer[i*2+j]*32768);
        }
    }
}

void SDLAudioEngine::audioCallback(void *userData, Uint8 *audioBuffer, int audioBufferLen)
{
    SDLAudioEngine *pThis = (SDLAudioEngine*)userData;
    pThis->mixAudio(audioBuffer, audioBufferLen);
}

void SDLAudioEngine::addBuffers(double *pDest, AudioBufferPtr pSrc)
{
    int numFrames = pSrc->getNumFrames();
    short * pData = pSrc->getData();
    for(int i = 0; i < numFrames*getChannels(); ++i) {
        pDest[i] += pData[i]/32768.0;
    }
}

void SDLAudioEngine::calcVolume(double *pBuffer, int numSamples, double volume)
{
    // TODO: We need a VolumeFader class that keeps state.
    for(int i = 0; i < numSamples; ++i) {
        pBuffer[i] *= volume;
    }
}

}
