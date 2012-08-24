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
//  Original author of this file is Benjamin Granzow (kontakt@bgranzow.de).
//

#include "AOAudioEngineThread.h"

#include "../base/Exception.h"
#include "Dynamics.h"

using namespace std;

namespace avg{

AOAudioEngineThread::AOAudioEngineThread(CQueue& cmdQ, AudioParams ap, float volume):
        WorkerThread<AOAudioEngineThread>("AOAudioThread", cmdQ),
        m_pDevice(0),
        m_bPlaying(false),
        m_Volume(volume),
        m_AP(ap)
{
    Dynamics<float, 2>* pLimiter = new Dynamics<float, 2>(float(m_AP.m_SampleRate));
    pLimiter->setThreshold(0.f); // in dB
    pLimiter->setAttackTime(0.f); // in seconds
    pLimiter->setReleaseTime(0.05f); // in seconds
    pLimiter->setRmsTime(0.f); // in seconds
    pLimiter->setRatio(std::numeric_limits<float>::infinity());
    pLimiter->setMakeupGain(0.f); // in dB
    m_pLimiter = pLimiter;

    memset(&m_Format, 0, sizeof(m_Format));
    m_Format.bits = 16;
    m_Format.channels = ap.m_Channels;
    m_Format.rate = 44100;
    m_Format.byte_format = AO_FMT_LITTLE;

    m_pOutputBuffer = AudioBufferPtr(new AudioBuffer(m_AP.m_OutputBufferSamples, m_AP));
    m_pTempBuffer = AudioBufferPtr(new AudioBuffer(m_AP.m_OutputBufferSamples, m_AP));
    m_pMixBuffer = new float[m_AP.m_Channels*m_AP.m_OutputBufferSamples];

    int driverID = ao_default_driver_id();
    m_pDevice = ao_open_live(driverID, &m_Format, NULL /* no options */);
    if (m_pDevice == NULL) {
        AVG_TRACE(Logger::ERROR, "Can't open AO audio device.");
    }
}

AOAudioEngineThread::~AOAudioEngineThread()
{
    m_AudioSources.clear();
/*    if (m_pLimiter) {
        delete m_pLimiter;
        m_pLimiter = 0;
    }
    if (m_pDevice) {
        ao_close(m_pDevice);
    }*/
}

bool AOAudioEngineThread::work()
{
    if (m_bPlaying) {
        mixAudio(m_pOutputBuffer);
        ao_play(m_pDevice, (char*)(m_pOutputBuffer->getData()), 
                m_pOutputBuffer->getNumBytes());
    }
    return true;
}

void AOAudioEngineThread::playAudio(bool bPlay)
{
    m_bPlaying = bPlay;
}

void AOAudioEngineThread::updateVolume(float volume)
{
    m_Volume = volume;
}

void AOAudioEngineThread::addSource(IAudioSource* pSource)
{
    m_AudioSources.push_back(pSource);
}

void AOAudioEngineThread::removeSource(IAudioSource* pSource)
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

void AOAudioEngineThread::mixAudio(AudioBufferPtr pDestBuffer)
{
    int numFrames = pDestBuffer->getNumFrames();
    for (int i = 0; i <m_Format.channels*numFrames; ++i) {
        m_pMixBuffer[i]=0;
    }
    {
        AudioSourceList::iterator it;
        for(it = m_AudioSources.begin(); it != m_AudioSources.end(); it++) {
            m_pTempBuffer->clear();
            (*it)->fillAudioBuffer(m_pTempBuffer);
            addBuffers(m_pMixBuffer, m_pTempBuffer);
        }
    }
    calcVolume(m_pMixBuffer, numFrames*m_Format.channels);
    short* pDestData = pDestBuffer->getData();
    for (int i = 0; i < numFrames; ++i) {
//        m_pLimiter->process(m_pMixBuffer+i*m_Format.channels);
        for (int j = 0; j < m_Format.channels; ++j) {
            pDestData[i*2+j] = int(m_pMixBuffer[i*2+j]*32768);
        }
    }
}

void AOAudioEngineThread::addBuffers(float* pDest, AudioBufferPtr pSrc)
{
    int numFrames = pSrc->getNumFrames();
    short* pData = pSrc->getData();
    for (int i = 0; i < numFrames*m_Format.channels; ++i) {
        pDest[i] += pData[i]/32768.0f;
    }
}

void AOAudioEngineThread::calcVolume(float* pBuffer, int numSamples)
{
    for (int i = 0; i < numSamples; ++i) {
        pBuffer[i] *= m_Volume;
    }
}

}
