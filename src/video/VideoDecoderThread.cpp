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

#include "VideoDecoderThread.h"

#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/ScopeTimer.h"
#include "../base/TimeSource.h"

using namespace std;

namespace avg {

VideoDecoderThread::VideoDecoderThread(CQueue& cmdQ, VideoMsgQueue& msgQ, 
        VideoDecoderPtr pDecoder)
    : WorkerThread<VideoDecoderThread>(string("Video Decoder"), cmdQ, 
            Logger::PROFILE_VIDEO),
      m_MsgQ(msgQ),
      m_pDecoder(pDecoder),
      m_pBmpQ(new BitmapQueue()),
      m_pHalfBmpQ(new BitmapQueue())
{
}

VideoDecoderThread::~VideoDecoderThread()
{
}

static ProfilingZoneID DecoderProfilingZone("DecoderThread");
static ProfilingZoneID PushMsgProfilingZone("DecoderThread: push message");

bool VideoDecoderThread::work() 
{
    if (m_pDecoder->isEOF(SS_VIDEO)) {
        if (!m_pDecoder->getVideoInfo().m_bHasAudio) {
            m_pDecoder->seek(0);
        } else {
            // TODO: Replace this with waitForMessage()
            msleep(10);
        }
    } else {
        ScopeTimer timer(DecoderProfilingZone);
        vector<BitmapPtr> pBmps;
        IntPoint size = m_pDecoder->getSize();
        IntPoint halfSize(size.x/2, size.y/2);
        PixelFormat pf = m_pDecoder->getPixelFormat();
        FrameAvailableCode frameAvailable;
        if (pixelFormatIsPlanar(pf)) {
            pBmps.push_back(getBmp(m_pBmpQ, size, I8));
            pBmps.push_back(getBmp(m_pHalfBmpQ, halfSize, I8));
            pBmps.push_back(getBmp(m_pHalfBmpQ, halfSize, I8));
            if (pf == YCbCrA420p) {
                pBmps.push_back(getBmp(m_pBmpQ, size, I8));
            }
        } else {
            pBmps.push_back(getBmp(m_pBmpQ, size, pf));
        }
        frameAvailable = m_pDecoder->renderToBmps(pBmps, -1);

        if (m_pDecoder->isEOF(SS_VIDEO)) {
            VideoMsgPtr pMsg(new VideoMsg());
            pMsg->setEOF();
            m_MsgQ.push(pMsg);
        } else {
            ScopeTimer timer(PushMsgProfilingZone);
            
            AVG_ASSERT(frameAvailable == FA_NEW_FRAME);
            VideoMsgPtr pMsg(new VideoMsg());
            pMsg->setFrame(pBmps, m_pDecoder->getCurTime(SS_VIDEO));
            m_MsgQ.push(pMsg);
            msleep(0);
        }
    }
    return true;
}

void VideoDecoderThread::seek(double destTime)
{
    while (!m_MsgQ.empty()) {
        m_MsgQ.pop(false);
    }

    double VideoFrameTime = -1;
    double AudioFrameTime = -1;
    m_pDecoder->seek(destTime);
    if (m_pDecoder->getVideoInfo().m_bHasVideo) {
        VideoFrameTime = m_pDecoder->getCurTime(SS_VIDEO);
    }
    if (m_pDecoder->getVideoInfo().m_bHasAudio) {
        AudioFrameTime = m_pDecoder->getCurTime(SS_AUDIO);
    }
    
    VideoMsgPtr pMsg(new VideoMsg());
    pMsg->setSeekDone(VideoFrameTime, AudioFrameTime);
    m_MsgQ.push(pMsg);
}

void VideoDecoderThread::setFPS(double fps)
{
    m_pDecoder->setFPS(fps);
}

void VideoDecoderThread::returnFrame(VideoMsgPtr pMsg)
{
    m_pBmpQ->push(pMsg->getFrameBitmap(0));
    PixelFormat pf = m_pDecoder->getPixelFormat();
    if (pixelFormatIsPlanar(pf)) {
        m_pHalfBmpQ->push(pMsg->getFrameBitmap(1));
        m_pHalfBmpQ->push(pMsg->getFrameBitmap(2));
        if (pf == YCbCrA420p) {
            m_pBmpQ->push(pMsg->getFrameBitmap(3));
        }
    }
}

BitmapPtr VideoDecoderThread::getBmp(BitmapQueuePtr pBmpQ, const IntPoint& size, 
        PixelFormat pf)
{
    BitmapPtr pBmp = pBmpQ->pop(false);
    if (pBmp) {
        AVG_ASSERT (pBmp->getSize() == size && pBmp->getPixelFormat() == pf);
        return pBmp;
    } else {
        return BitmapPtr(new Bitmap(size, pf)); 
    }
}

}
