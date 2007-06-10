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

#ifndef _PacketVideoMsg_H_
#define _PacketVideoMsg_H_

#include "../base/Queue.h"
#include "../graphics/Bitmap.h"

#ifdef _WIN32
#define EMULATE_INTTYPES
#else
// This is probably GCC-specific.
#define INT64_C(c)    c ## L
#endif
#include <ffmpeg/avformat.h>

#include <vector>

namespace avg {

class PacketVideoMsg {
    public:
        PacketVideoMsg(AVPacket * pPacket, bool bSeekDone);
        virtual ~PacketVideoMsg();

        void freePacket();
        AVPacket * getPacket();
        bool isSeekDone();

    private:
        AVPacket * m_pPacket;
        bool m_bSeekDone;
};

typedef boost::shared_ptr<PacketVideoMsg> PacketVideoMsgPtr;
typedef Queue<PacketVideoMsgPtr> VideoPacketQueue;
typedef boost::shared_ptr<VideoPacketQueue> VideoPacketQueuePtr;

}
#endif 

