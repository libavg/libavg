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

#include "../base/ObjectCounter.h"
#include "../base/Exception.h"
#include "../base/ScopeTimer.h"
#include "FFMpegDecoder.h"

#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

#include <math.h>
#include <iostream>

using namespace boost;
using namespace std;

namespace avg {

AsyncVideoDecoder::AsyncVideoDecoder(VideoDecoderPtr pSyncDecoder, int queueLength)
    : m_State(CLOSED),
      m_pSyncDecoder(pSyncDecoder),
      m_QueueLength(queueLength),
      m_pVDecoderThread(0),
      m_pADecoderThread(0),
      m_PF(NO_PIXELFORMAT),
      m_bAudioEOF(false),
      m_bVideoEOF(false),
      m_bSeekPending(false),
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
        bool bUseHardwareAccelleration)
{
    m_bAudioEOF = false;
    m_bVideoEOF = false;
    m_bSeekPending = false;
    m_sFilename = sFilename;

    m_pSyncDecoder->open(m_sFilename, bThreadedDemuxer, bUseHardwareAccelleration);
    m_VideoInfo = m_pSyncDecoder->getVideoInfo();
    // Temporary pf - always assumes shaders will be available.
    m_PF = m_pSyncDecoder->getPixelFormat();
    m_State = OPENED;
}

void AsyncVideoDecoder::startDecoding(bool bDeliverYCbCr, const AudioParams* pAP)
{
    AVG_ASSERT(m_State == OPENED);
    m_pSyncDecoder->setVolume(m_Volume);
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
        m_pAMsgQ = VideoMsgQueuePtr(new VideoMsgQueue(8));
        m_pADecoderThread = new boost::thread(
                 AudioDecoderThread(*m_pACmdQ, *m_pAMsgQ, m_pSyncDecoder, *pAP));
        m_AudioMsgData = 0;
        m_AudioMsgSize = 0;
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
        scoped_lock lock1(m_AudioMutex);
        if (m_pADecoderThread) {
            m_pACmdQ->pushCmd(boost::bind(&AudioDecoderThread::stop, _1));
            m_pAMsgQ->pop(false);
            m_pAMsgQ->pop(false);
            m_pADecoderThread->join();
            delete m_pADecoderThread;
            m_pADecoderThread = 0;
            m_pAMsgQ = VideoMsgQueuePtr();
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
    AVG_ASSERT(m_State == DECODING);
    waitForSeekDone();
    scoped_lock lock1(m_AudioMutex);
    scoped_lock Lock2(m_SeekMutex);
    m_bAudioEOF = false;
    m_bVideoEOF = false;
    m_bSeekPending = false;
    m_bSeekPending = true;
    if (m_pVCmdQ) {
        m_pVCmdQ->pushCmd(boost::bind(&VideoDecoderThread::seek, _1, destTime));
    } else {
        m_pACmdQ->pushCmd(boost::bind(&AudioDecoderThread::seek, _1, destTime));
    }
    bool bDone = false;
    while (!bDone && m_bSeekPending) {
        VideoMsgPtr pMsg;
        if (m_pVCmdQ) {
            pMsg = m_pVMsgQ->pop(false);
        } else {
            pMsg = m_pAMsgQ->pop(false);
        }
        if (pMsg) {
            switch (pMsg->getType()) {
                case VideoMsg::SEEK_DONE:
                    m_bSeekPending = false;
                    m_LastVideoFrameTime = pMsg->getSeekVideoFrameTime();
                    m_LastAudioFrameTime = pMsg->getSeekAudioFrameTime();
                    break;
                case VideoMsg::FRAME:
                    returnFrame(pMsg);
                    break;
                default:
                    break;
            }
        } else {
            bDone = true;
        }
    }
}

void AsyncVideoDecoder::loop()
{
    m_LastVideoFrameTime = -1;
    m_bAudioEOF = false;
    m_bVideoEOF = false;
    if (getVideoInfo().m_bHasAudio) {
        seek(0);
    }
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
    VideoMsgPtr pFrameMsg = getBmpsForTime(timeWanted, frameAvailable);
    if (frameAvailable == FA_NEW_FRAME) {
        AVG_ASSERT(pFrameMsg);
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

int AsyncVideoDecoder::fillAudioBuffer(AudioBufferPtr pBuffer)
{
    AVG_ASSERT(m_State == DECODING);
    AVG_ASSERT (m_pADecoderThread);
    if (m_bAudioEOF) {
        return 0;
    }
    scoped_lock lock(m_AudioMutex);
    waitForSeekDone();

    unsigned char* pDest = (unsigned char *)(pBuffer->getData());
    int bufferLeftToFill = pBuffer->getNumBytes();
    VideoMsgPtr pMsg;
    while (bufferLeftToFill > 0) {
        while (m_AudioMsgSize > 0 && bufferLeftToFill > 0) {
            int copyBytes = min(bufferLeftToFill, m_AudioMsgSize);
            memcpy(pDest, m_AudioMsgData, copyBytes);
            m_AudioMsgSize -= copyBytes;
            m_AudioMsgData += copyBytes;
            bufferLeftToFill -= copyBytes;
            pDest += copyBytes;

            m_LastAudioFrameTime += copyBytes / 
                    (pBuffer->getFrameSize() * pBuffer->getRate());
        }
        if (bufferLeftToFill != 0) {
            pMsg = m_pAMsgQ->pop(false);
            if (pMsg) {
                if (pMsg->getType() == VideoMsg::END_OF_FILE) {
                    m_bAudioEOF = true;
                    return pBuffer->getNumFrames()-bufferLeftToFill/
                        pBuffer->getFrameSize();
                }
                AVG_ASSERT(pMsg->getType() == VideoMsg::AUDIO);

                m_AudioMsgSize = pMsg->getAudioBuffer()->getNumFrames()
                    *pBuffer->getFrameSize();
                m_AudioMsgData = (unsigned char *)(pMsg->getAudioBuffer()->getData());
                m_LastAudioFrameTime = pMsg->getAudioTime();
            } else {
                return pBuffer->getNumFrames()-bufferLeftToFill/pBuffer->getFrameSize();
            }
        }
    }
    return pBuffer->getNumFrames();
}
        
VideoMsgPtr AsyncVideoDecoder::getBmpsForTime(float timeWanted, 
        FrameAvailableCode& frameAvailable)
{
    if (timeWanted < 0 && timeWanted != -1) {
        cerr << "Illegal timeWanted: " << timeWanted << endl;
        AVG_ASSERT(false);
    }
    // XXX: This code is sort-of duplicated in FFMpegDecoder::readFrameForTime()
    float frameTime = -1;
    VideoMsgPtr pFrameMsg;
    if (timeWanted == -1) {
        pFrameMsg = getNextBmps(true);
        frameAvailable = FA_NEW_FRAME;
    } else {
        float timePerFrame = 1.0f/getFPS();
        if (fabs(float(timeWanted-m_LastVideoFrameTime)) < 0.5*timePerFrame || 
                m_LastVideoFrameTime > timeWanted+timePerFrame) {
            // The last frame is still current. Display it again.
            frameAvailable = FA_USE_LAST_FRAME;
            return VideoMsgPtr();
        } else {
            if (m_bVideoEOF) {
                frameAvailable = FA_USE_LAST_FRAME;
                return VideoMsgPtr();
            }
            while (frameTime-timeWanted < -0.5*timePerFrame && !m_bVideoEOF) {
                if (pFrameMsg) {
                    if (pFrameMsg->getType() == VideoMsg::FRAME) {
                        returnFrame(pFrameMsg);
                    } else {
#if AVG_ENABLE_VDPAU
                        vdpau_render_state* pRenderState = pFrameMsg->getRenderState();
                        VDPAU::unlockSurface(pRenderState);
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
    }
    if (pFrameMsg) {
        m_LastVideoFrameTime = pFrameMsg->getFrameTime();
    }
    return pFrameMsg;
}

VideoMsgPtr AsyncVideoDecoder::getNextBmps(bool bWait)
{
    waitForSeekDone();
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
    scoped_lock lock(m_SeekMutex);
    if (m_bSeekPending) {
        do {
            VideoMsgPtr pMsg;
            if (m_pVCmdQ) {
                pMsg = m_pVMsgQ->pop(true);
            } else {
                pMsg = m_pAMsgQ->pop(true);
            }
            switch (pMsg->getType()) {
                case VideoMsg::SEEK_DONE:
                    m_bSeekPending = false;
                    m_LastVideoFrameTime = pMsg->getSeekVideoFrameTime();
                    m_LastAudioFrameTime = pMsg->getSeekAudioFrameTime();
                    break;
                case VideoMsg::FRAME:
                    returnFrame(pMsg);
                    break;
                case VideoMsg::VDPAU_FRAME:
                    break;
                default:
                    // TODO: Handle ERROR messages here.
                    break;
            }
        } while (m_bSeekPending);
    }
}

void AsyncVideoDecoder::returnFrame(VideoMsgPtr& pFrameMsg)
{
    if (pFrameMsg) {
        m_pVCmdQ->pushCmd(boost::bind(&VideoDecoderThread::returnFrame, _1, pFrameMsg));
    }
}

}
