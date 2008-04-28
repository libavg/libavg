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

#include "AudioEngine.h"

#include "../base/ObjectCounter.h"

using namespace std;

namespace avg {

AudioEngine::AudioEngine()
    : m_bEnabled(true),
      m_Volume(1)
{
    ObjectCounter::get()->incRef(&typeid(*this));
}

AudioEngine::~AudioEngine()
{
    ObjectCounter::get()->decRef(&typeid(*this));
    m_AudioSources.clear();
}

void AudioEngine::setAudioEnabled(bool bEnabled)
{
    m_bEnabled = bEnabled;
    if (m_bEnabled) {
        play();
    } else {
        pause();
    }
    
    AudioSourceList::iterator it;
    for(it = m_AudioSources.begin(); it != m_AudioSources.end(); it++)
    {
        (*it)->setAudioEnabled(m_bEnabled);
    }
}

void AudioEngine::init(const AudioParams& AP, double volume)
{
    m_Volume = volume;
}

AudioSourceList& AudioEngine::getSources()
{
    return m_AudioSources;
}

void AudioEngine::addSource(IAudioSource* pSource)
{
    pSource->setAudioEnabled(m_bEnabled);
    m_AudioSources.push_back(pSource);
}

void AudioEngine::removeSource(IAudioSource* pSource)
{
    AudioSourceList::iterator it;
    for(it = m_AudioSources.begin(); it != m_AudioSources.end(); it++)
    {
        if (*it == pSource) {
            m_AudioSources.erase(it);
            break;
        }
    }
}

void AudioEngine::setVolume(double volume)
{
    m_Volume = volume;
}

double AudioEngine::getVolume() const
{
    return m_Volume;
}

}
