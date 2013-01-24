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

#include "VideoDecoderThread.h"

#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/ScopeTimer.h"
#include "../base/TimeSource.h"
#include "../avgconfigwrapper.h"

struct vdpau_render_state;

using namespace std;

namespace avg {

VideoDecoderThread::VideoDecoderThread(CQueue& cmdQ, VideoMsgQueue& msgQ, 
        FFMpegDecoderPtr pDecoder, const IntPoint& size, PixelFormat pf, bool bUseVDPAU)
    : WorkerThread<VideoDecoderThread>(string("Video Decoder"), cmdQ, 
            Logger::PROFILE_VIDEO),
      m_MsgQ(msgQ),
      m_pDecoder(pDecoder),
      m_pBmpQ(new BitmapQueue()),
      m_pHalfBmpQ(new BitmapQueue()),
      m_Size(size),
      m_PF(pf),
      m_bUseVDPAU(bUseVDPAU)
{
}

VideoDecoderThread::~VideoDecoderThread()
{
}

static ProfilingZoneID DecoderProfilingZone("DecoderThread", true);
static ProfilingZoneID PushMsgProfilingZone("DecoderThread: push message", true);

bool VideoDecoderThread::work() 
{
    ScopeTimer timer(DecoderProfilingZone);
    vdpau_render_state* pRenderState = 0;
    FrameAvailableCode frameAvailable;
    vector<BitmapPtr> pBmps;
    if (m_bUseVDPAU) {
#ifdef AVG_ENABLE_VDPAU
        frameAvailable = m_pDecoder->renderToVDPAU(&pRenderState);
#else
        frameAvailable = FA_NEW_FRAME; // Never executed - silences compiler warning.
#endif
    } else {
        IntPoint halfSize(m_Size.x/2, m_Size.y/2);
        if (pixelFormatIsPlanar(m_PF)) {
            pBmps.push_back(getBmp(m_pBmpQ, m_Size, I8));
            pBmps.push_back(getBmp(m_pHalfBmpQ, halfSize, I8));
            pBmps.push_back(getBmp(m_pHalfBmpQ, halfSize, I8));
            if (m_PF == YCbCrA420p) {
                pBmps.push_back(getBmp(m_pBmpQ, m_Size, I8));
            }
        } else {
            pBmps.push_back(getBmp(m_pBmpQ, m_Size, m_PF));
        }
        frameAvailable = m_pDecoder->renderToBmps(pBmps);
    }
    if (frameAvailable == FA_CLOSED) {
        close();
    } else {
        if (m_pDecoder->isVideoSeekDone()) {
            m_MsgQ.clear();
            VideoMsgPtr pMsg(new VideoMsg());
            float videoFrameTime = m_pDecoder->getCurTime();
            pMsg->setSeekDone(m_pDecoder->getSeekSeqNum(), videoFrameTime);
            m_MsgQ.push(pMsg);
        }
        if (m_pDecoder->isEOF(SS_VIDEO)) {
            VideoMsgPtr pMsg(new VideoMsg());
            pMsg->setEOF();
            m_MsgQ.push(pMsg);
        } else {
            ScopeTimer timer(PushMsgProfilingZone);
            AVG_ASSERT(frameAvailable == FA_NEW_FRAME);
            VideoMsgPtr pMsg(new VideoMsg());
            if (m_bUseVDPAU) {
                pMsg->setVDPAUFrame(pRenderState, m_pDecoder->getCurTime());
            } else {
                pMsg->setFrame(pBmps, m_pDecoder->getCurTime());
            }
            m_MsgQ.push(pMsg);
            msleep(0);
        }
    }
    ThreadProfiler::get()->reset();
    return true;
}

void VideoDecoderThread::setFPS(float fps)
{
    m_pDecoder->setFPS(fps);
}

void VideoDecoderThread::returnFrame(VideoMsgPtr pMsg)
{
    m_pBmpQ->push(pMsg->getFrameBitmap(0));
    if (pixelFormatIsPlanar(m_PF)) {
        m_pHalfBmpQ->push(pMsg->getFrameBitmap(1));
        m_pHalfBmpQ->push(pMsg->getFrameBitmap(2));
        if (m_PF == YCbCrA420p) {
            m_pBmpQ->push(pMsg->getFrameBitmap(3));
        }
    }
}
        
void VideoDecoderThread::close()
{
    m_MsgQ.clear();
    stop();
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
