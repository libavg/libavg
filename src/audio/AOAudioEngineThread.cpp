#include "AOAudioEngineThread.h"

#include "../base/Exception.h"
#include "Dynamics.h"

using namespace std;

namespace avg{

AOAudioEngineThread::AOAudioEngineThread(CQueue& CmdQ, AudioParams ap, float volume):
        WorkerThread<AOAudioEngineThread>("AOAudioThread", CmdQ),
        m_device(0),
        m_default_driver(0),
        m_play(false),
        m_volume(volume),
        m_AP(ap),
        m_buffer(0),
        m_bufferLen(0),
        m_pCurBufferLen(new int(0))
{
    Dynamics<float, 2>* limiter = new Dynamics<float, 2>(float(m_AP.m_SampleRate));
    limiter->setThreshold(0.f); // in dB
    limiter->setAttackTime(0.f); // in seconds
    limiter->setReleaseTime(0.05f); // in seconds
    limiter->setRmsTime(0.f); // in seconds
    limiter->setRatio(std::numeric_limits<float>::infinity());
    limiter->setMakeupGain(0.f); // in dB
    m_limiter = limiter;


    m_default_driver = ao_default_driver_id();
    memset(&m_format, 0, sizeof(m_format));
    m_format.bits = 16;
    m_format.channels = ap.m_Channels;
    m_format.rate = 44100;
    m_format.byte_format = AO_FMT_LITTLE;

    m_bufferLen = m_format.bits/8 * m_format.channels * m_format.rate;
    m_buffer = (char*)calloc(m_bufferLen, sizeof(char));

    m_device = ao_open_live(m_default_driver, &m_format, NULL /* no options */);
    if (m_device == NULL) {
        AVG_TRACE(Logger::ERROR, "Can't open AO audio device.");
    }
}

AOAudioEngineThread::~AOAudioEngineThread()
{
    m_audioSources.clear();
/*    if (m_limiter) {
        delete m_limiter;
        m_limiter = 0;
    }
    if (m_device) {
        ao_close(m_device);
    }*/
}

bool AOAudioEngineThread::work()
{
    if (m_play) {
        mixAudio(m_buffer, m_bufferLen, m_pCurBufferLen);
        ao_play(m_device, m_buffer, *m_pCurBufferLen*4);
    }
    return true;
}

void AOAudioEngineThread::playAudio(bool startStop)
{
    m_play = startStop;
}

void AOAudioEngineThread::updateVolume(float volume)
{
    m_volume = volume;
}

void AOAudioEngineThread::addSource(IAudioSource* source)
{
    m_audioSources.push_back(source);
}

void AOAudioEngineThread::removeSource(IAudioSource* source)
{
    AudioSourceList::iterator it;
    for(it = m_audioSources.begin(); it != m_audioSources.end(); it++)
    {
        if (*it == source) {
            m_audioSources.erase(it);
            break;
        }
    }
}

void AOAudioEngineThread::mixAudio(char *destBuffer, int destBufferLen, int* m_pCurBufferLen)
{
    int numFrames = destBufferLen/(2*m_format.channels); // 16 bit samples.

    if (m_audioSources.size() == 0) {
        return;
    }
    if (!m_pTempBuffer || m_pTempBuffer->getNumFrames() < numFrames) {
        if (m_pTempBuffer) {
            delete[] m_pMixBuffer;
        }
        m_pTempBuffer = AudioBufferPtr(new AudioBuffer(numFrames, m_AP));
        m_pMixBuffer = new float[m_format.channels*numFrames];
    }

    for (int i = 0; i <m_format.channels*numFrames; ++i) {
        m_pMixBuffer[i]=0;
    }
    {
        AudioSourceList::iterator it;
        for(it = m_audioSources.begin(); it != m_audioSources.end(); it++) {
            m_pTempBuffer->clear();
            *m_pCurBufferLen = (*it)->fillAudioBuffer(m_pTempBuffer);
            addBuffers(m_pMixBuffer, m_pTempBuffer);
        }
    }
    calcVolume(m_pMixBuffer, numFrames*m_format.channels);
    for (int i = 0; i < numFrames; ++i) {
//        m_limiter->process(m_pMixBuffer+i*m_format.channels);
        for (int j = 0; j < m_format.channels; ++j) {
            ((short*)destBuffer)[i*2+j] = int(m_pMixBuffer[i*2+j]*32768);
        }
    }
}

void AOAudioEngineThread::addBuffers(float *dest, AudioBufferPtr pSrc)
{
    int numFrames = pSrc->getNumFrames();
    short* pData = pSrc->getData();
    for(int i = 0; i < numFrames*m_format.channels; ++i) {
        dest[i] += pData[i]/32768.0f;
    }
}

void AOAudioEngineThread::calcVolume(float *buffer, int numSamples)
{
    // TODO: We need a VolumeFader class that keeps state.
    for(int i = 0; i < numSamples; ++i) {
        buffer[i] *= m_volume;
    }
}

}
