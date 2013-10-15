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
        VideoMsgQueue& packetQ, AVStream* pStream, const IntPoint& size, PixelFormat pf, 
        bool bUseVDPAU)
    : WorkerThread<VideoDecoderThread>(string("Video Decoder"), cmdQ, 
            Logger::category::PROFILE_VIDEO),
      m_MsgQ(msgQ),
      m_PacketQ(packetQ),
      m_pBmpQ(new BitmapQueue()),
      m_pHalfBmpQ(new BitmapQueue()),
      m_Size(size),
      m_PF(pf),
      m_bUseVDPAU(bUseVDPAU),
      m_bSeekDone(false),
      m_bProcessingLastFrames(false)
{
    m_pFrameDecoder = FFMpegFrameDecoderPtr(new FFMpegFrameDecoder(pStream));
}

VideoDecoderThread::~VideoDecoderThread()
{
}

bool VideoDecoderThread::init()
{
#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(54, 28, 0) 
    m_pFrame = avcodec_alloc_frame();
#else
    m_pFrame = new AVFrame;
#endif
    return true;
}
        
void VideoDecoderThread::deinit()
{
#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(54, 28, 0) 
    avcodec_free_frame(&m_pFrame);
#else
    delete m_pFrame;
#endif
}

static ProfilingZoneID DecoderProfilingZone("Video Decoder Thread", true);
static ProfilingZoneID PacketWaitProfilingZone("Video wait for packet", true);

bool VideoDecoderThread::work() 
{
    ScopeTimer timer(DecoderProfilingZone);
    if (m_bProcessingLastFrames) {
        // EOF received, but last frames still need to be decoded.
        handleEOF();
    } else {
        // Standard decoding.
        VideoMsgPtr pMsg;
        {
            ScopeTimer timer(PacketWaitProfilingZone);
            pMsg = m_PacketQ.pop(true);
        }
        switch (pMsg->getType()) {
            case VideoMsg::PACKET:
                decodePacket(pMsg->getPacket());
                break;
            case VideoMsg::END_OF_FILE:
                handleEOF();
                m_bProcessingLastFrames = true;
                break;
            case VideoMsg::SEEK_DONE:
                handleSeekDone(pMsg);
                break;
            case VideoMsg::CLOSED:
                close();
                break;
            default:
                pMsg->dump();
                AVG_ASSERT(false);
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

void VideoDecoderThread::decodePacket(AVPacket* pPacket)
{
    bool bGotPicture = m_pFrameDecoder->decodePacket(pPacket, m_pFrame, m_bSeekDone);
    if (bGotPicture) {
        m_bSeekDone = false;
        sendFrame(m_pFrame);
    }
}

void VideoDecoderThread::handleEOF()
{
    bool bGotPicture = m_pFrameDecoder->decodeLastFrame(m_pFrame);
    if (bGotPicture) {
        sendFrame(m_pFrame);
    } else {
        m_bProcessingLastFrames = false;
        VideoMsgPtr pMsg(new VideoMsg());
        pMsg->setEOF();
        pushMsg(pMsg);
    }
}

void VideoDecoderThread::handleSeekDone(VideoMsgPtr pMsg)
{
    m_pFrameDecoder->handleSeek();
    m_bSeekDone = true;
    m_MsgQ.clear();
    pushMsg(pMsg);
}

static ProfilingZoneID CopyImageProfilingZone("Copy image", true);

void VideoDecoderThread::sendFrame(AVFrame* pFrame)
{
    VideoMsgPtr pMsg(new VideoMsg());
    if (m_bUseVDPAU) {
        vdpau_render_state *pRenderState = (vdpau_render_state *)pFrame->data[0];
        pMsg->setVDPAUFrame(pRenderState, m_pFrameDecoder->getCurTime());
    } else {
        vector<BitmapPtr> pBmps;
        if (pixelFormatIsPlanar(m_PF)) {
            ScopeTimer timer(CopyImageProfilingZone);
            IntPoint halfSize(m_Size.x/2, m_Size.y/2);
            pBmps.push_back(getBmp(m_pBmpQ, m_Size, I8));
            pBmps.push_back(getBmp(m_pHalfBmpQ, halfSize, I8));
            pBmps.push_back(getBmp(m_pHalfBmpQ, halfSize, I8));
            if (m_PF == YCbCrA420p) {
                pBmps.push_back(getBmp(m_pBmpQ, m_Size, I8));
            }
            for (unsigned i = 0; i < pBmps.size(); ++i) {
                m_pFrameDecoder->copyPlaneToBmp(pBmps[i], pFrame->data[i], 
                        pFrame->linesize[i]);
            }
        } else {
            pBmps.push_back(getBmp(m_pBmpQ, m_Size, m_PF));
            m_pFrameDecoder->convertFrameToBmp(pFrame, pBmps[0]);
        }
        pMsg->setFrame(pBmps, m_pFrameDecoder->getCurTime());
    }
    pushMsg(pMsg);
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

static ProfilingZoneID PushMsgProfilingZone("Push message", true);

void VideoDecoderThread::pushMsg(VideoMsgPtr pMsg)
{
    ScopeTimer timer(PushMsgProfilingZone);
    m_MsgQ.push(pMsg);
}

}
