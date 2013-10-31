//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2011 Ulrich von Zadow
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
#include "VideoMsg.h"

#include "../base/WorkerThread.h"
#include "../base/Command.h"
#include "../base/Queue.h"

#include "WrapFFMpeg.h"

#include <boost/thread.hpp>

namespace avg {

typedef Queue<Bitmap> BitmapQueue;
typedef boost::shared_ptr<BitmapQueue> BitmapQueuePtr;

class FFMpegFrameDecoder;
typedef boost::shared_ptr<FFMpegFrameDecoder> FFMpegFrameDecoderPtr;

class AVG_API VideoDecoderThread: public WorkerThread<VideoDecoderThread> {
    public:
        VideoDecoderThread(CQueue& cmdQ, VideoMsgQueue& msgQ, VideoMsgQueue& packetQ, 
                AVStream* pStream, const IntPoint& size, PixelFormat pf, bool bUseVDPAU);
        virtual ~VideoDecoderThread();
        virtual bool init();
        virtual void deinit();
        
        bool work();
        void setFPS(float fps);
        void returnFrame(VideoMsgPtr pMsg);

    private:
        void decodePacket(AVPacket* pPacket);
        void handleEOF();
        void handleSeekDone(VideoMsgPtr pMsg);
        void sendFrame(AVFrame* pFrame);
        void close();
        BitmapPtr getBmp(BitmapQueuePtr pBmpQ, const IntPoint& size, PixelFormat pf);
        void pushMsg(VideoMsgPtr pMsg);

        VideoMsgQueue& m_MsgQ;
        FFMpegFrameDecoderPtr m_pFrameDecoder;
        VideoMsgQueue& m_PacketQ;

        BitmapQueuePtr m_pBmpQ;
        BitmapQueuePtr m_pHalfBmpQ;
        
        IntPoint m_Size;
        PixelFormat m_PF;
        bool m_bUseVDPAU;

        bool m_bSeekDone;
        bool m_bProcessingLastFrames;
        AVFrame* m_pFrame;
};

}
#endif 

