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
#include "AsyncDemuxer.h"
#include "FFMpegFrameDecoder.h"

#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/ScopeTimer.h"
#include "../base/TimeSource.h"
#include "../avgconfigwrapper.h"

struct vdpau_render_state;

using namespace std;

namespace avg {

VideoDecoderThread::VideoDecoderThread(CQueue& cmdQ, VideoMsgQueue& msgQ, 
        AsyncDemuxer* pDemuxer, AVStream* pStream, int streamIndex,
        const IntPoint& size, PixelFormat pf, bool bUseVDPAU)
    : WorkerThread<VideoDecoderThread>(string("Video Decoder"), cmdQ, 
            Logger::category::PROFILE_VIDEO),
      m_MsgQ(msgQ),
      m_pDemuxer(pDemuxer),
      m_pStream(pStream),
      m_StreamIndex(streamIndex),
      m_pBmpQ(new BitmapQueue()),
      m_pHalfBmpQ(new BitmapQueue()),
      m_Size(size),
      m_PF(pf),
      m_bUseVDPAU(bUseVDPAU),
      m_bSeekDone(false),
      m_bEOFPending(false)
{
    m_pFrameDecoder = FFMpegFrameDecoderPtr(new FFMpegFrameDecoder(pStream));
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
        frameAvailable = renderToVDPAU(&pRenderState);
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
        frameAvailable = renderToBmps(pBmps);
    }
    if (frameAvailable == FA_CLOSED) {
        close();
    } else {
        if (isSeekDone()) {
            m_MsgQ.clear();
            VideoMsgPtr pMsg(new VideoMsg());
            float videoFrameTime = m_pFrameDecoder->getCurTime();
            pMsg->setSeekDone(m_SeekSeqNum, videoFrameTime);
            m_MsgQ.push(pMsg);
        }
        if (isEOF()) {
            VideoMsgPtr pMsg(new VideoMsg());
            pMsg->setEOF();
            m_MsgQ.push(pMsg);
        } else {
            ScopeTimer timer(PushMsgProfilingZone);
            AVG_ASSERT(frameAvailable == FA_NEW_FRAME);
            VideoMsgPtr pMsg(new VideoMsg());
            if (m_bUseVDPAU) {
                pMsg->setVDPAUFrame(pRenderState, m_pFrameDecoder->getCurTime());
            } else {
                pMsg->setFrame(pBmps, m_pFrameDecoder->getCurTime());
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
    m_pFrameDecoder->setFPS(fps);
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

static ProfilingZoneID RenderToBmpProfilingZone("FFMpeg: renderToBmp", true);
static ProfilingZoneID CopyImageProfilingZone("FFMpeg: copy image", true);
static ProfilingZoneID VDPAUCopyProfilingZone("FFMpeg: VDPAU copy", true);

FrameAvailableCode VideoDecoderThread::renderToBmps(vector<BitmapPtr>& pBmps)
{
    ScopeTimer timer(RenderToBmpProfilingZone);
    AVFrame frame;
    readFrame(frame);

    if (m_pDemuxer->isClosed(m_StreamIndex)) {
        return FA_CLOSED;
    } else {
        if (isEOF()) {
            return FA_USE_LAST_FRAME;
        } else {
            if (pixelFormatIsPlanar(m_PF)) {
                ScopeTimer timer(CopyImageProfilingZone);
                for (unsigned i = 0; i < pBmps.size(); ++i) {
                    m_pFrameDecoder->copyPlaneToBmp(pBmps[i],
                            frame.data[i], frame.linesize[i]);
                }
            } else {
                m_pFrameDecoder->convertFrameToBmp(frame, pBmps[0]);
            }
            return FA_NEW_FRAME;
        }
    }
}

#ifdef AVG_ENABLE_VDPAU
FrameAvailableCode VideoDecoderThread::renderToVDPAU(vdpau_render_state** ppRenderState)
{
    AVG_ASSERT(m_bUseVDPAU);
    ScopeTimer timer(RenderToBmpProfilingZone);
    AVFrame frame;
    readFrame(frame);
    if (m_pDemuxer->isClosed(m_StreamIndex)) {
        return FA_CLOSED;
    } else {
        if (isEOF()) {
            return FA_USE_LAST_FRAME;
        } else {
            ScopeTimer timer(VDPAUCopyProfilingZone);
            vdpau_render_state *pRenderState = (vdpau_render_state *)frame.data[0];
            *ppRenderState = pRenderState;
            return FA_NEW_FRAME;
        }
    }
}
#endif

bool VideoDecoderThread::isSeekDone()
{
    bool bSeekDone = m_bSeekDone;
    m_bSeekDone = false;
    return bSeekDone;
}

bool VideoDecoderThread::isEOF() const
{
    return m_pFrameDecoder->isEOF() && !m_bEOFPending;
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

static ProfilingZoneID DecodeProfilingZone("FFMpeg: decode", true);

void VideoDecoderThread::readFrame(AVFrame& frame)
{
    ScopeTimer timer(DecodeProfilingZone); 

    if (m_bEOFPending) {
        m_bEOFPending = false;
        return;
    }
    bool bDone = false;
    while (!bDone) {
        int seqNum;
        float seekTime = m_pDemuxer->isSeekDone(m_StreamIndex, seqNum);
        if (seekTime != -1) {
            m_pFrameDecoder->handleSeek();
            m_SeekSeqNum = seqNum;
            m_bSeekDone = true;
        }
        AVPacket* pPacket = m_pDemuxer->getPacket(m_StreamIndex);
        bool bGotPicture = m_pFrameDecoder->decodePacket(pPacket, frame, m_bSeekDone);
        if (bGotPicture && m_pFrameDecoder->isEOF()) {
            m_bEOFPending = true;
        }
        if (bGotPicture || m_pFrameDecoder->isEOF()) {
            bDone = true;
        }
    }
}

}
