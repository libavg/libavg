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
    AOAudioEngineThread(CQueue& CmdQ, AudioParams ap, float volume);
    ~AOAudioEngineThread();

    bool work();
    void playAudio(bool startStop);
    void updateVolume(float volume);
    void addSource(IAudioSource* source);
    void removeSource(IAudioSource* source);

private:
    void mixAudio(char *destBuffer, int destBufferLen, int* m_curBufferLen);
    void addBuffers(float *dest, AudioBufferPtr pSrc);
    void calcVolume(float *buffer, int numSamples);


    ao_device* m_device;
    ao_sample_format m_format;
    int m_default_driver;

    bool m_play;

    float m_volume;
    AudioParams m_AP;
    AudioBufferPtr m_pTempBuffer;
    float* m_pMixBuffer;
    char* m_buffer;
    int m_bufferLen;
    int* m_pCurBufferLen;
    IProcessor<float>* m_limiter;
    AudioSourceList m_audioSources;
};

}
#endif
