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
#include "ErrorVideoMsg.h"
#include "EOFVideoMsg.h"

#include "../base/Logger.h"

using namespace std;

namespace avg {

VideoDecoderThread::VideoDecoderThread(VideoMsgQueue& MsgQ, CmdQueue& CmdQ, 
        VideoDecoderPtr pDecoder, const std::string& sFilename, 
        YCbCrMode ycbcrMode, bool bSyncDemuxer)
    : WorkerThread<VideoDecoderThread>("VideoDecoder", CmdQ),
      m_MsgQ(MsgQ),
      m_pDecoder(pDecoder),
      m_sFilename(sFilename),
      m_YCbCrMode(ycbcrMode),
      m_bSyncDemuxer(bSyncDemuxer)
{
}

VideoDecoderThread::~VideoDecoderThread()
{
}

bool VideoDecoderThread::init()
{
    try {
        m_pDecoder->open(m_sFilename, m_YCbCrMode, m_bSyncDemuxer);
        PixelFormat PF = m_pDecoder->getPixelFormat();
        VideoMsgPtr pInfoMsg(new InfoVideoMsg(m_pDecoder->getSize(), 
                m_pDecoder->getNumFrames(), m_pDecoder->getFPS(), PF));
        m_MsgQ.push(pInfoMsg);
        return true;
    } catch (Exception& ex) {
        AVG_TRACE(Logger::ERROR, ex.GetStr());
        ErrorVideoMsgPtr pErrorMsg(new ErrorVideoMsg(ex));
        m_MsgQ.push(pErrorMsg);
        return false;
    }
};

bool VideoDecoderThread::work() 
{
    if (m_pDecoder->isEOF()) {
        // replace this with waitForMessage()
        TimeSource::get()->msleep(10);
    } else {
        vector<BitmapPtr> pBmps;
        IntPoint Size = m_pDecoder->getSize();
        PixelFormat PF = m_pDecoder->getPixelFormat();
        if (PF == YCbCr420p || PF ==YCbCrJ420p) {
            BitmapPtr pBmpY = BitmapPtr(new Bitmap(Size, I8));
            IntPoint HalfSize(Size.x/2, Size.y/2);
            BitmapPtr pBmpU = BitmapPtr(new Bitmap(HalfSize, I8));
            BitmapPtr pBmpV = BitmapPtr(new Bitmap(HalfSize, I8));
            bool bFrame=m_pDecoder->renderToYCbCr420p(pBmpY, pBmpU, pBmpV);
            if (bFrame) {
                pBmps.push_back(pBmpY);
                pBmps.push_back(pBmpU);
                pBmps.push_back(pBmpV);
            }
        } else {
            BitmapPtr pBmp = BitmapPtr(new Bitmap(Size, PF));
            bool bFrame=m_pDecoder->renderToBmp(pBmp);
            if (bFrame) {
                pBmps.push_back(pBmp);
            }
        }
        if (m_pDecoder->isEOF()) {
            m_MsgQ.push(VideoMsgPtr(new EOFVideoMsg()));
        } else {
            m_MsgQ.push(VideoMsgPtr(new FrameVideoMsg(pBmps)));
        }
    }
    return true;
}

void VideoDecoderThread::deinit()
{
    m_pDecoder->close();
}

void VideoDecoderThread::seek(int DestFrame)
{
    while (!m_MsgQ.empty()) {
        m_MsgQ.pop(true);
    }
    m_pDecoder->seek(DestFrame);
}

}
