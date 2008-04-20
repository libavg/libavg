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

#ifndef _AudioFrame_H_
#define _AudioFrame_H_

#include "AudioParams.h"

namespace avg
{

class AudioFrame
{
    public:
        AudioFrame(int size, AudioParams AP);
        virtual ~AudioFrame();
	
        unsigned char* getBuffer();
        int getSize();
        int getChannels();
        int getRate();
        void clear();
        
    private:
        int m_Size;
        unsigned char* m_Buffer;
        AudioParams m_AP;
};

}

#endif /*AUDIOFRAME_H_*/
