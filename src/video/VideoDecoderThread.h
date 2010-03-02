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

#ifndef _VideoDecoderThread_H_
#define _VideoDecoderThread_H_

#include "../api.h"
#include "IVideoDecoder.h"
#include "VideoMsg.h"

#include "../base/WorkerThread.h"
#include "../base/Command.h"
#include "../base/Queue.h"

#include <boost/thread.hpp>

#include <string>

namespace avg {

typedef Queue<BitmapPtr> BitmapQueue;
typedef boost::shared_ptr<BitmapQueue> BitmapQueuePtr;

class AVG_API VideoDecoderThread: public WorkerThread<VideoDecoderThread> {
    public:
        VideoDecoderThread(CmdQueue& CmdQ, VideoMsgQueue& MsgQ, 
                VideoDecoderPtr pDecoder);
        virtual ~VideoDecoderThread();
        
        bool work();
        void seek(long long DestTime);
        void setFPS(double FPS);
        void returnFrame(VideoMsgPtr pMsg);

    private:
        BitmapPtr getBmp(BitmapQueuePtr BmpQ, const IntPoint& Size, 
                PixelFormat pf);
        VideoMsgQueue& m_MsgQ;
        VideoDecoderPtr m_pDecoder;

        BitmapQueuePtr m_pBmpQ;
        BitmapQueuePtr m_pHalfBmpQ;
};

}
#endif 

