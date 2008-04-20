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


namespace avg {

SDLAudioEngine::SDLAudioEngine()
    : m_TempFrame(0),
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
    if (m_TempFrame) {
        delete m_TempFrame;
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

void SDLAudioEngine::init(const AudioParams& AP) 
{
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
    SDL_PauseAudio(1);
    SDL_CloseAudio();

    SDL_LockAudio();
    getSources().clear();
    SDL_UnlockAudio();
    if (m_pLimiter) {
        delete m_pLimiter;
        m_pLimiter = 0;
    }
}

void SDLAudioEngine::setAudioEnabled(bool bEnabled)
{
    SDL_LockAudio();
    AudioEngine::setAudioEnabled(bEnabled);
    SDL_UnlockAudio();
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
    SDL_LockAudio();
    AudioEngine::addSource(pSource);
    SDL_UnlockAudio();
}

void SDLAudioEngine::removeSource(IAudioSource* pSource)
{
    SDL_LockAudio();
    AudioEngine::removeSource(pSource);
    SDL_UnlockAudio();
}

void SDLAudioEngine::mixAudio(Uint8 *pDestBuffer, int destBufferLen)
{
    int numFrames = destBufferLen/(2*getChannels()); // 16 bit samples.

    if (getSources().size() == 0) {
        return;
    }
    if (!m_TempFrame || m_TempFrame->getSize() < destBufferLen) {
        if (m_TempFrame) {
            delete m_TempFrame;
            delete m_pMixBuffer;
        }
        m_TempFrame = new AudioFrame(destBufferLen, m_AP);
        m_pMixBuffer = new double[getChannels()*numFrames];
    }

    for (int i=0; i<getChannels()*numFrames; ++i) {
        m_pMixBuffer[i]=0;
    }
    AudioSourceList::iterator it;
    for(it = getSources().begin(); it != getSources().end(); it++) {
        m_TempFrame->clear();
        (*it)->fillAudioFrame(m_TempFrame);
        addBuffers(m_pMixBuffer, (short*)m_TempFrame->getBuffer(), numFrames);
    }
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

void SDLAudioEngine::addBuffers(double *pDest, short *pSrc, int numFrames)
{
    for(int i = 0; i < numFrames*getChannels(); ++i) {
        pDest[i] += pSrc[i]/32768.0;
    }
}

}
