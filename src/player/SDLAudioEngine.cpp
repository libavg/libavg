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
//  Original author of this file is Nick Hebner (hebern@gmail.com).
//

#include "SDLAudioEngine.h"

#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/ScopeTimer.h"
#include "../base/Profiler.h"


namespace avg {

SDLAudioEngine::SDLAudioEngine()
    : m_MixFrame(NULL)
{
    if (SDL_InitSubSystem(SDL_INIT_AUDIO)==-1) {
        AVG_TRACE(Logger::ERROR, "Can't init SDL audio subsystem.");
        exit(-1);
    }
}

SDLAudioEngine::~SDLAudioEngine()
{
    if(m_MixFrame)
        delete m_MixFrame;
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void SDLAudioEngine::init(const AudioParams& AP) 
{
    SDL_AudioSpec desired;
    SDL_AudioSpec acquired;
    desired.freq = AP.m_SampleRate;
    desired.format = AUDIO_S16SYS;
    desired.channels = AP.m_Channels;
    desired.silence = 0;
    desired.samples = SDL_AUDIO_BUFFER_SIZE;
    desired.callback = audioCallback;
    desired.userdata = this;

    if(SDL_OpenAudio(&desired, &acquired) < 0) {
      //throw new Exception("Cannot open audio device");
    }
    
    m_AP.m_Channels = acquired.channels;
    m_AP.m_SampleRate = acquired.freq;
}

void SDLAudioEngine::teardown()
{
    SDL_PauseAudio(1);
    SDL_CloseAudio();
    
    SDL_LockAudio();
    m_AudioSources.clear();
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

void SDLAudioEngine::addSource(AudioSourcePtr pSource)
{
    SDL_LockAudio();
    AudioEngine::addSource(pSource);
    SDL_UnlockAudio();
}

void SDLAudioEngine::mixAudio(Uint8 *audioBuffer, int audioBufferLen)
{
    if(m_MixFrame == NULL || m_MixFrame->getSize() < audioBufferLen)
    {
        if(m_MixFrame)
            delete m_MixFrame;
        m_MixFrame = new AudioFrame(audioBufferLen, m_AP);
    }
    
    m_MixFrame->clear();
    
    std::vector<AudioSourcePtr>::iterator it;
    for(it = m_AudioSources.begin(); it != m_AudioSources.end(); it++)
    {
        AudioSourcePtr src = *it;
        src->fillAudioFrame(m_MixFrame);
        SDL_MixAudio(audioBuffer, m_MixFrame->getBuffer(), audioBufferLen, 
            (int)(src->getVolume()*SDL_MIX_MAXVOLUME));
    }
}

void SDLAudioEngine::audioCallback(void *userData, Uint8 *audioBuffer, int audioBufferLen)
{
    SDLAudioEngine *pThis = (SDLAudioEngine*)userData;
    pThis->mixAudio(audioBuffer, audioBufferLen);
}

}
