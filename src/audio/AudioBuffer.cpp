//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2008 Ulrich von Zadow
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

#include "AudioBuffer.h"

#include <string>
#include <cstring>

namespace avg {

AudioBuffer::AudioBuffer(int numFrames, AudioParams ap)
    : m_NumFrames(numFrames),
      m_AP(ap)
{
    m_pData = new short[numFrames*sizeof(short)*ap.m_Channels];
}

AudioBuffer::~AudioBuffer()
{
    delete[] m_pData;
}

short* AudioBuffer::getData()
{
    return m_pData;
}

int AudioBuffer::getNumFrames()
{
    return m_NumFrames;
}

int AudioBuffer::getNumBytes()
{
    return m_NumFrames*m_AP.m_Channels*sizeof(short);
}

int AudioBuffer::getFrameSize()
{
    return m_AP.m_Channels*sizeof(short);
}

int AudioBuffer::getNumChannels()
{
    return m_AP.m_Channels;
}

int AudioBuffer::getRate()
{
    return m_AP.m_SampleRate;
}

void AudioBuffer::clear()
{
    memset(m_pData, 0, m_NumFrames*sizeof(short)*m_AP.m_Channels);
}

}
