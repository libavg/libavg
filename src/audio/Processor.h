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
//  Original author of this file is Andreas Beisler.
//

#pragma once
#ifndef __PROCESSOR_H__
#define __PROCESSOR_H__

#include <assert.h>

template<typename T, int IN_CHANNELS, int OUT_CHANNELS>
class Processor
{
    public:
        Processor(T fs);
        virtual ~Processor();
        void Process(T* inSamples, T* outSamples);

    protected:
        T m_fs;

    private:
        Processor();
};

template<typename T, int IN_CHANNELS, int OUT_CHANNELS>
Processor<T, IN_CHANNELS, OUT_CHANNELS>::Processor(T fs)
    : m_fs(fs)
{
}

template<typename T, int IN_CHANNELS, int OUT_CHANNELS>
Processor<T, IN_CHANNELS, OUT_CHANNELS>::~Processor()
{
}

template<typename T, int IN_CHANNELS, int OUT_CHANNELS>
void Processor<T, IN_CHANNELS, OUT_CHANNELS>::Process(T* inSamples, T* outSamples)
{
    assert(IN_CHANNELS == OUT_CHANNELS);
    memcpy(outSamples, inSamples, sizeof(T)*IN_CHANNELS);
}

#endif // __PROCESSOR_H__

