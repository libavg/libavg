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

#ifndef _AudioVideoMsg_H_
#define _AudioVideoMsg_H_

#include "VideoMsg.h"

#ifdef _WIN32
#define EMULATE_INTTYPES
#else
// This is probably GCC-specific.
#if !defined INT64_C
#if defined __WORDSIZE && __WORDSIZE == 64
#define INT64_C(c) c ## L
#else
#define INT64_C(c) c ## LL
#endif
#endif
#endif

extern "C" {
#include <ffmpeg/avformat.h>
}

namespace avg {

class AudioVideoMsg : public VideoMsg
{
public:
	AudioVideoMsg(int Size, long long Time);
	virtual ~AudioVideoMsg();
	
	unsigned char* getBuffer();
	int getSize();
	long long getFrameTime();
	
private:
    unsigned char* m_pBuffer;
    int m_Size;
    long long m_FrameTime;
};

typedef boost::shared_ptr<AudioVideoMsg> AudioVideoMsgPtr;

}

#endif
