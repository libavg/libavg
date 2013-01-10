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

namespace avg {

AsyncVideoDecoder::AsyncVideoDecoder(FFMpegDecoderPtr pSyncDecoder, int queueLength)
    : m_State(CLOSED),
      m_pSyncDecoder(pSyncDecoder),
      m_QueueLength(queueLength),
      m_pVDecoderThread(0),
      m_pADecoderThread(0),
      m_PF(NO_PIXELFORMAT),
      m_bAudioEOF(false),
      m_bVideoEOF(false),
      m_bASeekPending(false),
      m_bVSeekPending(false),
      m_Volume(1.0),
      m_LastVideoFrameTime(-1),
      m_LastAudioFrameTime(-1)
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

void AsyncVideoDecoder::open(const std::string& sFilename, bool bThreadedDemuxer,
        bool bUseHardwareAcceleration, bool bEnableSound)
{
    m_bAudioEOF = false;
    m_bVideoEOF = false;
    m_bASeekPending = false;
    m_bVSeekPending = false;
    m_sFilename = sFilename;

    m_pSyncDecoder->open(m_sFilename, bThreadedDemuxer, bUseHardwareAcceleration, 
            bEnableSound);
    m_VideoInfo = m_pSyncDecoder->getVideoInfo();
    // Temporary pf - always assumes shaders will be available.
    m_PF = m_pSyncDecoder->getPixelFormat();
    m_State = OPENED;
}

void AsyncVideoDecoder::startDecoding(bool bDeliverYCbCr, const AudioParams* pAP)
{
    AVG_ASSERT(m_State == OPENED);
    m_pSyncDecoder->startDecoding(bDeliverYCbCr, pAP);
    m_VideoInfo = m_pSyncDecoder->getVideoInfo();
    if (m_VideoInfo.m_bHasVideo) {
        m_LastVideoFrameTime = -1;
        m_PF = m_pSyncDecoder->getPixelFormat();
        m_pVCmdQ = VideoDecoderThread::CQueuePtr(new VideoDecoderThread::CQueue);
        m_pVMsgQ = VideoMsgQueuePtr(new VideoMsgQueue(m_QueueLength));
        m_pVDecoderThread = new boost::thread(
                 VideoDecoderThread(*m_pVCmdQ, *m_pVMsgQ, m_pSyncDecoder));
    }
    
    if (m_VideoInfo.m_bHasAudio) {
        m_pACmdQ = AudioDecoderThread::CQueuePtr(new AudioDecoderThread::CQueue);
        m_pAMsgQ = AudioMsgQueuePtr(new AudioMsgQueue(100));
        m_pAStatusQ = AudioMsgQueuePtr(new AudioMsgQueue(100));
        m_pADecoderThread = new boost::thread(
                 AudioDecoderThread(*m_pACmdQ, *m_pAMsgQ, m_pSyncDecoder, *pAP));
        m_pACmdQ->pushCmd(boost::bind(&AudioDecoderThread::setVolume, _1, m_Volume));
        m_LastAudioFrameTime = 0;
    }
    m_State = DECODING;
}

void AsyncVideoDecoder::close()
{
    AVG_ASSERT(m_State != CLOSED);
    if (m_pVDecoderThread) {
        m_pVCmdQ->pushCmd(boost::bind(&VideoDecoderThread::stop, _1));
        getNextBmps(false); // If the Queue is full, this breaks the lock in the thread.
        m_pVDecoderThread->join();
        delete m_pVDecoderThread;
        m_pVDecoderThread = 0;
        m_pVMsgQ = VideoMsgQueuePtr();
    }
    {
        if (m_pADecoderThread) {
            m_pACmdQ->pushCmd(boost::bind(&AudioDecoderThread::stop, _1));
            m_pAMsgQ->clear();
            m_pAStatusQ->clear();
            m_pADecoderThread->join();
            delete m_pADecoderThread;
            m_pADecoderThread = 0;
            m_pAStatusQ = AudioMsgQueuePtr();
            m_pAMsgQ = AudioMsgQueuePtr();
        }
        m_pSyncDecoder->close();
    }        
}

VideoDecoder::DecoderState AsyncVideoDecoder::getState() const
{
    return m_State;
}

VideoInfo AsyncVideoDecoder::getVideoInfo() const
{
    AVG_ASSERT(m_State != CLOSED);
    return m_VideoInfo;
}

void AsyncVideoDecoder::seek(float destTime)
{
//    cerr << "AsyncVideoDecoder::seek " << destTime << endl;
    AVG_ASSERT(m_State == DECODING);
    waitForSeekDone();
    scoped_lock lock(m_SeekMutex);
    m_bAudioEOF = false;
    m_bVideoEOF = false;
    if (m_pACmdQ) {
        m_pACmdQ->pushCmd(boost::bind(&AudioDecoderThread::seek, _1, destTime,
                !(bool(m_pVCmdQ))));
        m_bASeekPending = true;
    }
    if (m_pVCmdQ) {
        m_pVCmdQ->pushCmd(boost::bind(&VideoDecoderThread::seek, _1, destTime));
        m_bVSeekPending = true;
    }
//    cerr << "AsyncVideoDecoder::seek destTime=" << destTime << endl;
    checkSeekDone();
}

void AsyncVideoDecoder::loop()
{
    m_LastVideoFrameTime = -1;
    m_bAudioEOF = false;
    m_bVideoEOF = false;
    seek(0);
}

IntPoint AsyncVideoDecoder::getSize() const
{
    AVG_ASSERT(m_State != CLOSED);
    return m_VideoInfo.m_Size;
}

int AsyncVideoDecoder::getCurFrame() const
{
    AVG_ASSERT(m_State != CLOSED);
    return int(getCurTime(SS_VIDEO)*m_VideoInfo.m_StreamFPS+0.5);
}

int AsyncVideoDecoder::getNumFramesQueued() const
{
    AVG_ASSERT(m_State == DECODING);
    return m_pVMsgQ->size();
}

float AsyncVideoDecoder::getCurTime(StreamSelect stream) const
{
    AVG_ASSERT(m_State != CLOSED);
    switch (stream) {
        case SS_DEFAULT:
        case SS_VIDEO:
            AVG_ASSERT(m_VideoInfo.m_bHasVideo);
            return m_LastVideoFrameTime;
            break;
        case SS_AUDIO:
            AVG_ASSERT(m_VideoInfo.m_bHasAudio);
            return m_LastAudioFrameTime;
            break;
        default:
            AVG_ASSERT(false);
    }
    return -1;
}

float AsyncVideoDecoder::getNominalFPS() const
{
    AVG_ASSERT(m_State != CLOSED);
    return m_VideoInfo.m_StreamFPS;
}

float AsyncVideoDecoder::getFPS() const
{
    AVG_ASSERT(m_State != CLOSED);
    return m_VideoInfo.m_FPS;
}

void AsyncVideoDecoder::setFPS(float fps)
{
    AVG_ASSERT(!m_pADecoderThread);
    m_pVCmdQ->pushCmd(boost::bind(&VideoDecoderThread::setFPS, _1, fps));
    if (fps != 0) {
        m_VideoInfo.m_FPS = fps;
    }
}

float AsyncVideoDecoder::getVolume() const
{
    AVG_ASSERT(m_State != CLOSED);
    return m_Volume;
}

void AsyncVideoDecoder::setVolume(float volume)
{
    m_Volume = volume;
    if (m_State != CLOSED && m_VideoInfo.m_bHasAudio && m_pACmdQ) {
        m_pACmdQ->pushCmd(boost::bind(&AudioDecoderThread::setVolume, _1, volume));
    }
}

PixelFormat AsyncVideoDecoder::getPixelFormat() const
{
    AVG_ASSERT(m_State != CLOSED);
    return m_PF;
}

static ProfilingZoneID VDPAUDecodeProfilingZone("AsyncVideoDecoder: VDPAU", true);

FrameAvailableCode AsyncVideoDecoder::renderToBmps(vector<BitmapPtr>& pBmps,
        float timeWanted)
{
    AVG_ASSERT(m_State == DECODING);
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
        if (pFrameMsg->getType() == VideoMsg::VDPAU_FRAME) {
#ifdef AVG_ENABLE_VDPAU
            ScopeTimer timer(VDPAUDecodeProfilingZone);
            vdpau_render_state* pRenderState = pFrameMsg->getRenderState();
            if (pixelFormatIsPlanar(m_PF)) {
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

bool AsyncVideoDecoder::isEOF(StreamSelect stream) const
{
    AVG_ASSERT(m_State == DECODING);
    switch(stream) {
        case SS_AUDIO:
            return (!m_VideoInfo.m_bHasAudio || m_bAudioEOF);
        case SS_VIDEO:
            return (!m_VideoInfo.m_bHasVideo || m_bVideoEOF);
        case SS_ALL:
            return isEOF(SS_VIDEO) && isEOF(SS_AUDIO);
        default:
            return false;
    }
}

void AsyncVideoDecoder::throwAwayFrame(float timeWanted)
{
    AVG_ASSERT(m_State == DECODING);
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

VideoMsgPtr AsyncVideoDecoder::getBmpsForTime(float timeWanted, 
        FrameAvailableCode& frameAvailable)
{
    if (timeWanted < 0) {
        cerr << "Illegal timeWanted: " << timeWanted << endl;
        AVG_ASSERT(false);
    }
    // XXX: This code is sort-of duplicated in FFMpegDecoder::readFrameForTime()
    float frameTime = -1;
    VideoMsgPtr pFrameMsg;
    float timePerFrame = 1.0f/getFPS();
    if (!m_bVSeekPending && 
            (fabs(float(timeWanted-m_LastVideoFrameTime)) < 0.5*timePerFrame || 
             m_LastVideoFrameTime > timeWanted+timePerFrame ||
             m_bVideoEOF)) 
    {
        // The last frame is still current. Display it again.
        frameAvailable = FA_USE_LAST_FRAME;
        return VideoMsgPtr();
    } else {
        while (frameTime-timeWanted < -0.5*timePerFrame && !m_bVideoEOF) {
            if (pFrameMsg) {
                if (pFrameMsg->getType() == VideoMsg::FRAME) {
                    returnFrame(pFrameMsg);
                } else {
#if AVG_ENABLE_VDPAU
                    vdpau_render_state* pRenderState = pFrameMsg->getRenderState();
                    pRenderState->state &= ~FF_VDPAU_STATE_USED_FOR_REFERENCE;
                    unlockVDPAUSurface(pRenderState);
#endif
                }
            }
            pFrameMsg = getNextBmps(false);
            if (pFrameMsg) {
                if (m_bVSeekPending) {
                    frameAvailable = FA_STILL_DECODING;
                    returnFrame(dynamic_pointer_cast<VideoMsg>(pFrameMsg));
                    return VideoMsgPtr();
                }
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
                m_bVideoEOF = true;
                return VideoMsgPtr();
            case VideoMsg::ERROR:
                m_bVideoEOF = true;
                return VideoMsgPtr();
            case AudioMsg::SEEK_DONE:
                m_bVSeekPending = false;
                m_LastVideoFrameTime = pMsg->getSeekTime();
//                cerr << "Video SEEK_DONE Audio: " << m_LastAudioFrameTime << ", Video: " 
//                        << m_LastVideoFrameTime << ", diff: " << 
//                        m_LastAudioFrameTime - m_LastVideoFrameTime << endl;
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

void AsyncVideoDecoder::checkSeekDone()
{
    if (m_pVMsgQ) {
        VideoMsgPtr pMsg = m_pVMsgQ->pop(false);
        while (pMsg && m_bVSeekPending) {
            m_bVSeekPending = handleVSeekMsg(pMsg);
            if (m_bVSeekPending) {
                pMsg = m_pVMsgQ->pop(false);
            }
        }
    }
}

void AsyncVideoDecoder::waitForSeekDone()
{
//    cerr << "AsyncVideoDecoder::waitForSeekDone" << endl;
    scoped_lock lock(m_SeekMutex);
    if (m_bVSeekPending) {
//        cerr << "AsyncVideoDecoder::waitForSeekDone: V pending" << endl;
        do {
            VideoMsgPtr pMsg = m_pVMsgQ->pop(true);
            m_bVSeekPending = handleVSeekMsg(pMsg);
        } while (m_bVSeekPending);
    }
//    cerr << "AsyncVideoDecoder::waitForSeekDone 1" << endl;
    if (m_bASeekPending) {
//        cerr << "AsyncVideoDecoder::waitForSeekDone: A pending" << endl;
        do {
            AudioMsgPtr pMsg = m_pAStatusQ->pop(true);
            handleAudioMsg(pMsg);
        } while (m_bASeekPending);
    }
//    cerr << "AsyncVideoDecoder::waitForSeekDone end" << endl;
}

bool AsyncVideoDecoder::handleVSeekMsg(VideoMsgPtr pMsg)
{
    switch (pMsg->getType()) {
        case AudioMsg::SEEK_DONE:
            m_LastVideoFrameTime = pMsg->getSeekTime();
//            cerr << "Video SEEK_DONE Audio: " << m_LastAudioFrameTime << ", Video: " 
//                    << m_LastVideoFrameTime << ", diff: " << 
//                    m_LastAudioFrameTime - m_LastVideoFrameTime << endl;
            return false;
        case VideoMsg::FRAME:
            returnFrame(dynamic_pointer_cast<VideoMsg>(pMsg));
            return true;
        case VideoMsg::VDPAU_FRAME:
        case VideoMsg::END_OF_FILE:
            return true;
        default:
            // TODO: Handle ERROR messages here.
            AVG_ASSERT(false);
            return true;
    }
}

void AsyncVideoDecoder::handleAudioMsg(AudioMsgPtr pMsg)
{
//    pMsg->dump();
    switch (pMsg->getType()) {
        case AudioMsg::END_OF_FILE:
        case AudioMsg::ERROR:
            m_bAudioEOF = true;
            break;
        case AudioMsg::SEEK_DONE:
            m_bASeekPending = false;
            m_bAudioEOF = false;
            m_LastAudioFrameTime = pMsg->getSeekTime();
//            cerr << "Audio SEEK_DONE Audio: " << m_LastAudioFrameTime << ", Video: " 
//                    << m_LastVideoFrameTime << ", diff: " << 
//                    m_LastAudioFrameTime - m_LastVideoFrameTime << endl;
            break;
        case AudioMsg::AUDIO_TIME:
            m_LastAudioFrameTime = pMsg->getAudioTime();
//          cerr << "Audio: " << m_LastAudioFrameTime << ", Video: " 
//                  << m_LastVideoFrameTime << ", diff: " << 
//                  m_LastAudioFrameTime - m_LastVideoFrameTime << endl;
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
        m_pVCmdQ->pushCmd(boost::bind(&VideoDecoderThread::returnFrame, _1, pFrameMsg));
    }
}

}
