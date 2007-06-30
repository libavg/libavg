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

#ifndef _AsyncVideoDecoder_H_
#define _AsyncVideoDecoder_H_

#include "IVideoDecoder.h"
#include "VideoDecoderThread.h"
#include "FrameVideoMsg.h"

#include "../graphics/Bitmap.h"

#include <string>

namespace avg {

class AsyncVideoDecoder: public IVideoDecoder
{
    public:
        AsyncVideoDecoder(VideoDecoderPtr pSyncDecoder);
        virtual ~AsyncVideoDecoder();
        virtual void open(const std::string& sFilename, YCbCrMode ycbcrMode,
                bool bSyncDemuxer);
        virtual void close();
        virtual void seek(int DestFrame);
        virtual IntPoint getSize();
        virtual int getNumFrames();
        virtual double getFPS();
        virtual PixelFormat getPixelFormat();

        virtual FrameAvailableCode renderToBmp(BitmapPtr pBmp, long long TimeWanted);
        virtual FrameAvailableCode renderToYCbCr420p(BitmapPtr pBmpY, BitmapPtr pBmpCb, 
                BitmapPtr pBmpCr, long long TimeWanted);
        virtual long long getCurFrameTime();
        virtual bool isEOF();

    private:
        void getInfoMsg();
        FrameVideoMsgPtr getBmpsForTime(long long TimeWanted, 
                FrameAvailableCode& FrameAvailable);
        FrameVideoMsgPtr getNextBmps(bool bWait);
        void waitForSeekDone();

        VideoDecoderPtr m_pSyncDecoder;
        std::string m_sFilename;

        boost::thread* m_pDecoderThread;

        VideoDecoderThread::CmdQueuePtr m_pCmdQ;
        VideoMsgQueuePtr m_pMsgQ;

        IntPoint m_Size;
        int m_NumFrames;
        double m_FPS;
        PixelFormat m_PF;
        bool m_bEOF;
        bool m_bSeekPending;

        long long m_LastFrameTime;
        long long m_TimePerFrame;
};

}
#endif 

