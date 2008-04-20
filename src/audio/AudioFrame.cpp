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

#include "AudioFrame.h"

#include <string>

namespace avg {

AudioFrame::AudioFrame(int size, AudioParams AP)
    : m_Size(size),
    m_AP(AP)
{
    m_Buffer = new unsigned char[size];
}

AudioFrame::~AudioFrame()
{
    delete m_Buffer;
}

unsigned char* AudioFrame::getBuffer()
{
    return m_Buffer;
}

int AudioFrame::getSize()
{
    return m_Size;
}

int AudioFrame::getChannels()
{
    return m_AP.m_Channels;
}

int AudioFrame::getRate()
{
    return m_AP.m_SampleRate;
}

void AudioFrame::clear()
{
    memset(m_Buffer, 0, m_Size);
}

}
