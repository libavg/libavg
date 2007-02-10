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

#include "VideoDecoderThread.h"
#include "FrameVideoMsg.h"
#include "InfoVideoMsg.h"
#include "EOFVideoMsg.h"

#include "../base/Logger.h"

using namespace std;

namespace avg {

VideoDecoderThread::VideoDecoderThread(VideoMsgQueue& MsgQ, CmdQueue& CmdQ, 
        VideoDecoderPtr pDecoder, const std::string& sFilename, 
        DisplayEngine::YCbCrMode ycbcrMode)
    : WorkerThread<VideoDecoderThread>(CmdQ),
      m_MsgQ(MsgQ),
      m_pDecoder(pDecoder),
      m_sFilename(sFilename),
      m_YCbCrMode(ycbcrMode)
{
}

VideoDecoderThread::~VideoDecoderThread()
{
}

bool VideoDecoderThread::init()
{
    try {
        m_pDecoder->open(m_sFilename, m_YCbCrMode);
        PixelFormat PF = m_pDecoder->getPixelFormat();
        VideoMsgPtr pInfoMsg(new InfoVideoMsg(m_pDecoder->getSize(), m_pDecoder->getNumFrames(),
                m_pDecoder->getFPS(), PF));
        m_MsgQ.push(pInfoMsg);
        return true;
    } catch (Exception& ex) {
//        push exception on MsgQ.
        AVG_TRACE(Logger::ERROR, ex.GetStr());
        return false;
    }
};

bool VideoDecoderThread::work() 
{
    vector<BitmapPtr> pBmps;
    bool bEOF;
    IntPoint Size = m_pDecoder->getSize();
    PixelFormat PF = m_pDecoder->getPixelFormat();
    if (PF == YCbCr420p || PF ==YCbCrJ420p) {
        BitmapPtr pBmpY = BitmapPtr(new Bitmap(Size, I8));
        IntPoint HalfSize(Size.x/2, Size.y/2);
        BitmapPtr pBmpU = BitmapPtr(new Bitmap(HalfSize, I8));
        BitmapPtr pBmpV = BitmapPtr(new Bitmap(HalfSize, I8));
        bEOF=m_pDecoder->renderToYCbCr420p(pBmpY, pBmpU, pBmpV);
        pBmps.push_back(pBmpY);
        pBmps.push_back(pBmpU);
        pBmps.push_back(pBmpV);
    } else {
        BitmapPtr pBmp = BitmapPtr(new Bitmap(Size, PF));
        bEOF=m_pDecoder->renderToBmp(pBmp);
        pBmps.push_back(pBmp);
    }
    if (bEOF) {
        m_MsgQ.push(VideoMsgPtr(new EOFVideoMsg()));
        return false;
    } else {
        m_MsgQ.push(VideoMsgPtr(new FrameVideoMsg(pBmps)));
        return true;
    }
}

void VideoDecoderThread::deinit()
{
    m_pDecoder->close();
}

void VideoDecoderThread::seek(int DestFrame)
{
    // TODO: Implement
}

}
