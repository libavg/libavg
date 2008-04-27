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

#ifndef _AudioBuffer_H_
#define _AudioBuffer_H_

#include "AudioParams.h"

#include <boost/shared_ptr.hpp>

namespace avg
{

class AudioBuffer
{
    public:
        AudioBuffer(int numFrames, AudioParams AP);
        virtual ~AudioBuffer();

        short* getData();
        int getNumFrames();
        int getNumChannels();
        int getRate();
        void clear();

    private:
        int m_NumFrames;
        short* m_pData;
        AudioParams m_AP;
};

typedef boost::shared_ptr<AudioBuffer> AudioBufferPtr;

}

#endif /*AUDIOFRAME_H_*/
