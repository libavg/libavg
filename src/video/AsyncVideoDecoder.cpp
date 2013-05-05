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

#include "AsyncVideoDecoder.h"

#ifdef AVG_ENABLE_VDPAU
#include "VDPAUDecoder.h"
#include "VDPAUHelper.h"
#endif

#include "../base/ObjectCounter.h"
#include "../base/Exception.h"
#include "../base/ScopeTimer.h"

#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

#include <math.h>
#include <iostream>

using namespace boost;
using namespace std;

#define AUDIO_MSG_QUEUE_LENGTH  50
#define AUDIO_STATUS_QUEUE_LENGTH -1
#define PACKET_QUEUE_LENGTH 50

namespace avg {

AsyncVideoDecoder::AsyncVideoDecoder(int queueLength)
    : m_QueueLength(queueLength),
      m_pDemuxThread(0),
      m_pVDecoderThread(0),
      m_pADecoderThread(0),
      m_bUseStreamFPS(true),
      m_FPS(0)
{
    ObjectCounter::get()->incRef(&typeid(*this));
}

AsyncVideoDecoder::~AsyncVideoDecoder()
{
    if (m_pVDecoderThread || m_pADecoderThread) {
        close();
    }
    ObjectCounter::get()->decRef(&typeid(*this));
}

void AsyncVideoDecoder::open(const std::string& sFilename, bool bUseHardwareAcceleration, 
        bool bEnableSound)
{
    m_NumSeeksSent = 0;
    m_NumVSeeksDone = 0;
    m_NumASeeksDone = 0;
    m_bAudioEOF = false;
    m_bVideoEOF = false;
    m_bWasVSeeking = false;
    m_bWasSeeking = false;
    m_CurVideoFrameTime = -1;
    
    VideoDecoder::open(sFilename, bUseHardwareAcceleration, bEnableSound);

    if (getVideoInfo().m_bHasVideo && m_bUseStreamFPS) {
        m_FPS = getStreamFPS();
    }
}

void AsyncVideoDecoder::startDecoding(bool bDeliverYCbCr, const AudioParams* pAP)
{
    VideoDecoder::startDecoding(bDeliverYCbCr, pAP);

    AVG_ASSERT(!m_pDemuxThread);
    vector<int> streamIndexes;
    if (getVStreamIndex() >= 0) {
        streamIndexes.push_back(getVStreamIndex());
    }
    if (getAStreamIndex() >= 0) {
        streamIndexes.push_back(getAStreamIndex());
    }
    setupDemuxer(streamIndexes);

    if (getVideoInfo().m_bHasVideo) {
        m_LastVideoFrameTime = -1;
        m_CurVideoFrameTime = -1;
        if (m_bUseStreamFPS) {
            m_FPS = getStreamFPS();
        }
        m_pVCmdQ = VideoDecoderThread::CQueuePtr(new VideoDecoderThread::CQueue);
        m_pVMsgQ = VideoMsgQueuePtr(new VideoMsgQueue(m_QueueLength));
        VideoMsgQueue& packetQ = *m_PacketQs[getVStreamIndex()];

        m_pVDecoderThread = new boost::thread(VideoDecoderThread(
                *m_pVCmdQ, *m_pVMsgQ, packetQ, getVideoStream(), 
                getSize(), getPixelFormat(), usesVDPAU()));
    }
    
    if (getVideoInfo().m_bHasAudio) {
        m_pACmdQ = AudioDecoderThread::CQueuePtr(new AudioDecoderThread::CQueue);
        m_pAMsgQ = AudioMsgQueuePtr(new AudioMsgQueue(AUDIO_MSG_QUEUE_LENGTH));
        m_pAStatusQ = AudioMsgQueuePtr(new AudioMsgQueue(AUDIO_STATUS_QUEUE_LENGTH));
        VideoMsgQueue& packetQ = *m_PacketQs[getAStreamIndex()];
        m_pADecoderThread = new boost::thread(
                AudioDecoderThread(*m_pACmdQ, *m_pAMsgQ, packetQ, getAudioStream(), *pAP));
        m_LastAudioFrameTime = 0;
    }
}

void AsyncVideoDecoder::close()
{
    AVG_ASSERT(getState() != CLOSED);

    if (m_pDemuxThread) {
        m_pDemuxCmdQ->pushCmd(boost::bind(&VideoDemuxerThread::close, _1));
        m_pDemuxThread->join();
    }

    if (m_pVDecoderThread) {
        m_pVMsgQ->clear();
        m_pVDecoderThread->join();
        delete m_pVDecoderThread;
        m_pVDecoderThread = 0;
        m_pVMsgQ = VideoMsgQueuePtr();
    }
    if (m_pADecoderThread) {
        m_pAMsgQ->clear();
        m_pAStatusQ->clear();
        m_pADecoderThread->join();
        delete m_pADecoderThread;
        m_pADecoderThread = 0;
        m_pAStatusQ = AudioMsgQueuePtr();
        m_pAMsgQ = AudioMsgQueuePtr();
    }
    VideoDecoder::close();
    if (m_pDemuxThread) {
        deleteDemuxer();
    }
}

void AsyncVideoDecoder::seek(float destTime)
{
    AVG_ASSERT(getState() == DECODING);
    m_bAudioEOF = false;
    m_bVideoEOF = false;
    m_NumSeeksSent++;
    m_pDemuxCmdQ->pushCmd(boost::bind(&VideoDemuxerThread::seek, _1, m_NumSeeksSent,
            destTime));
}

void AsyncVideoDecoder::loop()
{
    m_LastVideoFrameTime = -1;
    m_bAudioEOF = false;
    m_bVideoEOF = false;
    seek(0);
}

int AsyncVideoDecoder::getCurFrame() const
{
    AVG_ASSERT(getState() != CLOSED);
    return int(getCurTime()*getVideoInfo().m_StreamFPS+0.5);
}

int AsyncVideoDecoder::getNumFramesQueued() const
{
    AVG_ASSERT(getState() == DECODING);
    return m_pVMsgQ->size();
}

float AsyncVideoDecoder::getCurTime() const
{
    AVG_ASSERT(getState() != CLOSED);
    if (getVideoInfo().m_bHasVideo) {
        return m_CurVideoFrameTime;
    } else {
        return m_LastAudioFrameTime;
    }
}

float AsyncVideoDecoder::getFPS() const
{
    AVG_ASSERT(getState() != CLOSED);
    return m_FPS;
}

void AsyncVideoDecoder::setFPS(float fps)
{
    AVG_ASSERT(!m_pADecoderThread);
    m_pVCmdQ->pushCmd(boost::bind(&VideoDecoderThread::setFPS, _1, fps));
    m_bUseStreamFPS = (fps == 0);
    if (m_bUseStreamFPS) {
        m_FPS = getVideoInfo().m_StreamFPS;
    } else {
        m_FPS = fps;
    }
}

static ProfilingZoneID VDPAUDecodeProfilingZone("AsyncVideoDecoder: VDPAU", true);

FrameAvailableCode AsyncVideoDecoder::renderToBmps(vector<BitmapPtr>& pBmps,
        float timeWanted)
{
    AVG_ASSERT(getState() == DECODING);
    FrameAvailableCode frameAvailable;
    VideoMsgPtr pFrameMsg;
    if (timeWanted == -1) {
        waitForSeekDone();
        pFrameMsg = getNextBmps(true);
        frameAvailable = FA_NEW_FRAME;
    } else {
        pFrameMsg = getBmpsForTime(timeWanted, frameAvailable);
    }
    if (frameAvailable == FA_NEW_FRAME) {
        AVG_ASSERT(pFrameMsg);
        m_LastVideoFrameTime = pFrameMsg->getFrameTime();
        m_CurVideoFrameTime = m_LastVideoFrameTime;
        if (pFrameMsg->getType() == VideoMsg::VDPAU_FRAME) {
#ifdef AVG_ENABLE_VDPAU
            ScopeTimer timer(VDPAUDecodeProfilingZone);
            vdpau_render_state* pRenderState = pFrameMsg->getRenderState();
            if (pixelFormatIsPlanar(getPixelFormat())) {
                getPlanesFromVDPAU(pRenderState, pBmps[0], pBmps[1], pBmps[2]);
            } else {
                getBitmapFromVDPAU(pRenderState, pBmps[0]);
            }
#endif
        } else {
            for (unsigned i = 0; i < pBmps.size(); ++i) {
                pBmps[i]->copyPixels(*(pFrameMsg->getFrameBitmap(i)));
            }
            returnFrame(pFrameMsg);
        }
    }
    return frameAvailable;
}

void AsyncVideoDecoder::updateAudioStatus()
{
    if (m_pAStatusQ) {
        AudioMsgPtr pMsg = m_pAStatusQ->pop(false);
        while (pMsg) {
            handleAudioMsg(pMsg);
            pMsg = m_pAStatusQ->pop(false);
        }
    }
}

bool AsyncVideoDecoder::isEOF() const
{
    AVG_ASSERT(getState() == DECODING);
    bool bEOF = true;
    if (getVideoInfo().m_bHasAudio && !m_bAudioEOF) {
        bEOF = false;
    }
    if (getVideoInfo().m_bHasVideo && !m_bVideoEOF) {
        bEOF = false;
    }
    return bEOF;
}

void AsyncVideoDecoder::throwAwayFrame(float timeWanted)
{
    AVG_ASSERT(getState() == DECODING);
    FrameAvailableCode frameAvailable;
    VideoMsgPtr pFrameMsg = getBmpsForTime(timeWanted, frameAvailable);
}

AudioMsgQueuePtr AsyncVideoDecoder::getAudioMsgQ()
{
    return m_pAMsgQ;
}

AudioMsgQueuePtr AsyncVideoDecoder::getAudioStatusQ() const
{
    return m_pAStatusQ;
}

void AsyncVideoDecoder::setupDemuxer(vector<int> streamIndexes)
{
    m_pDemuxCmdQ = VideoDemuxerThread::CQueuePtr(new VideoDemuxerThread::CQueue());    
    for (unsigned i = 0; i < streamIndexes.size(); ++i) {
        VideoMsgQueuePtr pPacketQ(new VideoMsgQueue(PACKET_QUEUE_LENGTH));
        m_PacketQs[streamIndexes[i]] = pPacketQ;
    }
    m_pDemuxThread = new boost::thread(VideoDemuxerThread(*m_pDemuxCmdQ,
            getFormatContext(), m_PacketQs));
}

void AsyncVideoDecoder::deleteDemuxer()
{
    delete m_pDemuxThread;
    m_pDemuxThread = 0;
    map<int, VideoMsgQueuePtr>::iterator it;
    for (it = m_PacketQs.begin(); it != m_PacketQs.end(); it++) {
        VideoMsgQueuePtr pPacketQ = it->second;
        VideoMsgPtr pPacketMsg;
        pPacketMsg = pPacketQ->pop(false);
        while (pPacketMsg) {
            pPacketMsg->freePacket();
            pPacketMsg = pPacketQ->pop(false);
        }
    }
}

VideoMsgPtr AsyncVideoDecoder::getBmpsForTime(float timeWanted, 
        FrameAvailableCode& frameAvailable)
{
    if (timeWanted < 0) {
        cerr << "Illegal timeWanted: " << timeWanted << endl;
        AVG_ASSERT(false);
    }
    VideoMsgPtr pFrameMsg;
    float timePerFrame = 1.0f/getFPS();

    checkForSeekDone();
    bool bVSeekDone = (!isVSeeking() && m_bWasVSeeking);
    m_bWasVSeeking = isVSeeking();
    
    if (!isSeeking() && m_bWasSeeking) {
//        cerr << "timeWanted: " << timeWanted << ", audio: " << m_LastAudioFrameTime
//                << ", diff: " << timeWanted-m_LastAudioFrameTime << endl;
    }
    m_bWasSeeking = isSeeking();
    if ((!bVSeekDone &&
            (isVSeeking() ||
             fabs(float(timeWanted-m_LastVideoFrameTime)) < 0.5*timePerFrame || 
             m_LastVideoFrameTime > timeWanted+timePerFrame)) ||
         m_bVideoEOF) 
    {
        // The last frame is still current. Display it again.
        frameAvailable = FA_USE_LAST_FRAME;
        return VideoMsgPtr();
    } else {
        float frameTime = -1;
        while (frameTime-timeWanted < -0.5*timePerFrame && !m_bVideoEOF) {
            if (pFrameMsg) {
                if (pFrameMsg->getType() == VideoMsg::FRAME) {
                    returnFrame(pFrameMsg);
                } else {
#if AVG_ENABLE_VDPAU
                    vdpau_render_state* pRenderState = pFrameMsg->getRenderState();
                    unlockVDPAUSurface(pRenderState);
#endif
                }
            }
            pFrameMsg = getNextBmps(false);
            if (pFrameMsg) {
                frameTime = pFrameMsg->getFrameTime();
            } else {
                frameAvailable = FA_STILL_DECODING;
                return VideoMsgPtr();
            }
        }
        if (!pFrameMsg) {
            cerr << "frameTime=" << frameTime << ", timeWanted=" << timeWanted 
                    << ", timePerFrame=" << timePerFrame << ", m_bVideoEOF=" 
                    << m_bVideoEOF << endl;
            AVG_ASSERT(false);
        }
        frameAvailable = FA_NEW_FRAME;
    }
    return pFrameMsg;
}

VideoMsgPtr AsyncVideoDecoder::getNextBmps(bool bWait)
{
    VideoMsgPtr pMsg = m_pVMsgQ->pop(bWait);
    if (pMsg) {
        switch (pMsg->getType()) {
            case VideoMsg::FRAME:
            case VideoMsg::VDPAU_FRAME:
                return pMsg;
            case VideoMsg::END_OF_FILE:
                m_NumVSeeksDone = m_NumSeeksSent;
                m_bVideoEOF = true;
                return VideoMsgPtr();
            case VideoMsg::ERROR:
                m_bVideoEOF = true;
                return VideoMsgPtr();
            case AudioMsg::SEEK_DONE:
                handleVSeekDone(pMsg);
                return getNextBmps(bWait);
            default:
                // Unhandled message type.
                AVG_ASSERT(false);
                return VideoMsgPtr();
        }
    } else {
        return pMsg;
    }
}
void AsyncVideoDecoder::waitForSeekDone()
{
    while (isVSeeking()) {
        VideoMsgPtr pMsg = m_pVMsgQ->pop(true);
        handleVSeekMsg(pMsg);
    }
}

void AsyncVideoDecoder::checkForSeekDone()
{
    if (isVSeeking()) {
        VideoMsgPtr pMsg;
        do {
            pMsg = m_pVMsgQ->pop(false);
            if (pMsg) {
                handleVSeekMsg(pMsg);
            }
        } while (pMsg && isVSeeking());
    }
}

void AsyncVideoDecoder::handleVSeekMsg(VideoMsgPtr pMsg)
{
    switch (pMsg->getType()) {
        case AudioMsg::SEEK_DONE:
            handleVSeekDone(pMsg);
            break;
        case VideoMsg::FRAME:
            returnFrame(dynamic_pointer_cast<VideoMsg>(pMsg));
            break;
        case VideoMsg::VDPAU_FRAME:
#ifdef AVG_ENABLE_VDPAU
            unlockVDPAUSurface(pMsg->getRenderState());
#endif            
            break;
        case VideoMsg::END_OF_FILE:
            m_NumVSeeksDone = m_NumSeeksSent;
            m_bVideoEOF = true;
            break;
        default:
            // TODO: Handle ERROR messages here.
            AVG_ASSERT(false);
    }
}

void AsyncVideoDecoder::handleVSeekDone(AudioMsgPtr pMsg)
{
    m_LastVideoFrameTime = pMsg->getSeekTime() - 1/m_FPS;
    if (m_NumVSeeksDone < pMsg->getSeekSeqNum()) {
        m_NumVSeeksDone = pMsg->getSeekSeqNum();
    }
}

void AsyncVideoDecoder::handleAudioMsg(AudioMsgPtr pMsg)
{
    switch (pMsg->getType()) {
        case AudioMsg::END_OF_FILE:
        case AudioMsg::ERROR:
            m_bAudioEOF = true;
            break;
        case AudioMsg::SEEK_DONE:
//            pMsg->dump();
            m_bAudioEOF = false;
            m_LastAudioFrameTime = pMsg->getSeekTime();
            if (m_NumASeeksDone < pMsg->getSeekSeqNum()) {
                m_NumASeeksDone = pMsg->getSeekSeqNum();
            }
            break;
        case AudioMsg::AUDIO_TIME:
            m_LastAudioFrameTime = pMsg->getAudioTime();
            break;
        default:
            // Unhandled message type.
            pMsg->dump();
            AVG_ASSERT(false);
    }
}

void AsyncVideoDecoder::returnFrame(VideoMsgPtr pFrameMsg)
{
    if (pFrameMsg) {
        AVG_ASSERT(pFrameMsg->getType() == VideoMsg::FRAME);
        m_pVCmdQ->pushCmd(boost::bind(&VideoDecoderThread::returnFrame, _1, pFrameMsg));
    }
}

bool AsyncVideoDecoder::isSeeking() const
{
    return (m_NumSeeksSent > m_NumVSeeksDone || m_NumSeeksSent > m_NumASeeksDone);
}

bool AsyncVideoDecoder::isVSeeking() const
{
    return m_NumSeeksSent > m_NumVSeeksDone;
}

}
