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

#ifndef _AOAudioEngineThread_H_
#define _AOAudioEngineThread_H_

#include <base/WorkerThread.h>
#include <ao/ao.h>

#include "AudioParams.h"
#include "AudioBuffer.h"
#include "IAudioSource.h"
#include "IProcessor.h"

namespace avg {

typedef std::vector<IAudioSource*> AudioSourceList;

class AOAudioEngineThread : public WorkerThread<AOAudioEngineThread>
{
public:
    AOAudioEngineThread(CQueue& cmdQ, AudioParams ap, float volume);
    ~AOAudioEngineThread();
    bool init();
    void deinit();
    bool haveDevice();

    bool work();
    void playAudio(bool bPlay);
    void updateVolume(float volume);
    void addSource(IAudioSource* pSource);
    void removeSource(IAudioSource* pSource);

private:
    void mixAudio(AudioBufferPtr pDestBuffer);
    void addBuffers(float* pDest, AudioBufferPtr pSrc);
    void calcVolume(float* pBuffer, int numSamples);

    ao_device* m_pDevice;
    ao_sample_format m_Format;

    bool m_bUsingNullDevice;
    bool m_bPlaying;

    float m_Volume;
    AudioParams m_AP;
    AudioBufferPtr m_pTempBuffer;
    float* m_pMixBuffer;
    AudioBufferPtr m_pOutputBuffer;
    
    IProcessor<float>* m_pLimiter;
    AudioSourceList m_AudioSources;
};

}
#endif
