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

#include "AsyncVideoDecoder.h"

#include "../base/ObjectCounter.h"
#include "../base/Exception.h"

#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

#include <math.h>
#include <iostream>

using namespace boost;
using namespace std;

namespace avg {

AsyncVideoDecoder::AsyncVideoDecoder(VideoDecoderPtr pSyncDecoder)
    : m_State(CLOSED),
      m_pSyncDecoder(pSyncDecoder),
      m_pVDecoderThread(0),
      m_pADecoderThread(0),
      m_PF(NO_PIXELFORMAT),
      m_bAudioEOF(false),
      m_bVideoEOF(false),
      m_bSeekPending(false),
      m_Volume(1.0),
      m_LastVideoFrameTime(-1000),
      m_LastAudioFrameTime(-1000)
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

void AsyncVideoDecoder::open(const std::string& sFilename, bool bThreadedDemuxer)
{
    m_bAudioEOF = false;
    m_bVideoEOF = false;
    m_bSeekPending = false;
    m_sFilename = sFilename;

    m_pSyncDecoder->open(m_sFilename, bThreadedDemuxer);
    m_VideoInfo = m_pSyncDecoder->getVideoInfo();
    m_State = OPENED;
}

void AsyncVideoDecoder::startDecoding(bool bDeliverYCbCr, const AudioParams* pAP)
{
    AVG_ASSERT(m_State == OPENED);
    m_pSyncDecoder->startDecoding(bDeliverYCbCr, pAP);
    m_VideoInfo = m_pSyncDecoder->getVideoInfo();
    if (m_VideoInfo.m_bHasVideo) {
        m_LastVideoFrameTime = -1000;
        m_PF = m_pSyncDecoder->getPixelFormat();
        m_pVCmdQ = VideoDecoderThread::CQueuePtr(new VideoDecoderThread::CQueue);
        m_pVMsgQ = VideoMsgQueuePtr(new VideoMsgQueue(8));
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
        setVolume(m_Volume);
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
    }
    {
        scoped_lock Lock1(m_AudioMutex);
        if (m_pADecoderThread) {
            m_pACmdQ->pushCmd(boost::bind(&AudioDecoderThread::stop, _1));
            m_pAMsgQ->pop(false);
            m_pAMsgQ->pop(false);
            m_pADecoderThread->join();
            delete m_pADecoderThread;
            m_pADecoderThread = 0;
        }
        m_pSyncDecoder->close();
    }        
}

IVideoDecoder::DecoderState AsyncVideoDecoder::getState() const
{
    return m_State;
}

VideoInfo AsyncVideoDecoder::getVideoInfo() const
{
    AVG_ASSERT(m_State != CLOSED);
    return m_VideoInfo;
}

void AsyncVideoDecoder::seek(long long DestTime)
{
    AVG_ASSERT(m_State == DECODING);
    waitForSeekDone();
    scoped_lock Lock1(m_AudioMutex);
    scoped_lock Lock2(m_SeekMutex);
    m_bAudioEOF = false;
    m_bVideoEOF = false;
    m_bSeekPending = false;
    m_LastVideoFrameTime = -1000;
    m_bSeekPending = true;
    if (m_pVCmdQ) {
        m_pVCmdQ->pushCmd(boost::bind(&VideoDecoderThread::seek, _1, DestTime));
    } else {
        m_pACmdQ->pushCmd(boost::bind(&AudioDecoderThread::seek, _1, DestTime));
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

IntPoint AsyncVideoDecoder::getSize() const
{
    AVG_ASSERT(m_State != CLOSED);
    return m_VideoInfo.m_Size;
}

int AsyncVideoDecoder::getCurFrame() const
{
    AVG_ASSERT(m_State == DECODING);
    return int(getCurTime(SS_VIDEO)*m_VideoInfo.m_StreamFPS/1000.0+0.5);
}

int AsyncVideoDecoder::getNumFramesQueued() const
{
    AVG_ASSERT(m_State == DECODING);
    return m_pVMsgQ->size();
}

long long AsyncVideoDecoder::getCurTime(StreamSelect Stream) const
{
    AVG_ASSERT(m_State == DECODING);
    switch(Stream) {
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

double AsyncVideoDecoder::getNominalFPS() const
{
    AVG_ASSERT(m_State != CLOSED);
    return m_VideoInfo.m_StreamFPS;
}

double AsyncVideoDecoder::getFPS() const
{
    AVG_ASSERT(m_State != CLOSED);
    return m_VideoInfo.m_FPS;
}

void AsyncVideoDecoder::setFPS(double FPS)
{
    AVG_ASSERT(!m_pADecoderThread);
    m_pVCmdQ->pushCmd(boost::bind(&VideoDecoderThread::setFPS, _1, FPS));
    if (FPS != 0) {
        m_VideoInfo.m_FPS = FPS;
    }
}

double AsyncVideoDecoder::getVolume() const
{
    AVG_ASSERT(m_State != CLOSED);
    return m_Volume;
}

void AsyncVideoDecoder::setVolume(double Volume)
{
    m_Volume = Volume;
    if (m_State != CLOSED && m_VideoInfo.m_bHasAudio && m_pACmdQ) {
        m_pACmdQ->pushCmd(boost::bind(&AudioDecoderThread::setVolume, _1, Volume));
    }
}

PixelFormat AsyncVideoDecoder::getPixelFormat() const
{
    AVG_ASSERT(m_State == DECODING);
    return m_PF;
}

FrameAvailableCode AsyncVideoDecoder::renderToBmp(BitmapPtr pBmp, long long timeWanted)
{
    AVG_ASSERT(pBmp);
    AVG_ASSERT(m_State == DECODING);
    FrameAvailableCode FrameAvailable;
    VideoMsgPtr pFrameMsg = getBmpsForTime(timeWanted, FrameAvailable);
    if (FrameAvailable == FA_NEW_FRAME) {
        AVG_ASSERT(pFrameMsg);
        pBmp->copyPixels(*(pFrameMsg->getFrameBitmap(0)));
        returnFrame(pFrameMsg);
    }
    return FrameAvailable;
}

FrameAvailableCode AsyncVideoDecoder::renderToYCbCr420p(BitmapPtr pBmpY, BitmapPtr pBmpCb,
       BitmapPtr pBmpCr, long long timeWanted)
{
    AVG_ASSERT(m_State == DECODING);
    FrameAvailableCode FrameAvailable;
    VideoMsgPtr pFrameMsg = getBmpsForTime(timeWanted, FrameAvailable);
    if (FrameAvailable == FA_NEW_FRAME) {
        pBmpY->copyPixels(*(pFrameMsg->getFrameBitmap(0)));
        pBmpCb->copyPixels(*(pFrameMsg->getFrameBitmap(1)));
        pBmpCr->copyPixels(*(pFrameMsg->getFrameBitmap(2)));
        returnFrame(pFrameMsg);
    }
    return FrameAvailable;
}

bool AsyncVideoDecoder::isEOF(StreamSelect Stream) const
{
    AVG_ASSERT(m_State == DECODING);
    switch(Stream) {
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

void AsyncVideoDecoder::throwAwayFrame(long long timeWanted)
{
    AVG_ASSERT(m_State == DECODING);
    FrameAvailableCode FrameAvailable;
    VideoMsgPtr pFrameMsg = getBmpsForTime(timeWanted, FrameAvailable);
}

int AsyncVideoDecoder::fillAudioBuffer(AudioBufferPtr pBuffer)
{
    AVG_ASSERT(m_State == DECODING);
    AVG_ASSERT (m_pADecoderThread);
    if (m_bAudioEOF) {
        return 0;
    }
    scoped_lock Lock(m_AudioMutex);
    waitForSeekDone();

    unsigned char* audioBuffer = (unsigned char *)(pBuffer->getData());
    int audioBufferSize = pBuffer->getNumBytes();

    int bufferLeftToFill = audioBufferSize;
    VideoMsgPtr pMsg;
    while (bufferLeftToFill > 0) {
        while (m_AudioMsgSize > 0 && bufferLeftToFill > 0) {
            int copyBytes = min(bufferLeftToFill, m_AudioMsgSize);
            memcpy(audioBuffer, m_AudioMsgData, copyBytes);
            m_AudioMsgSize -= copyBytes;
            m_AudioMsgData += copyBytes;
            bufferLeftToFill -= copyBytes;
            audioBuffer += copyBytes;

            m_LastAudioFrameTime += (long long)(1000.0 * copyBytes / 
                    (pBuffer->getFrameSize() * pBuffer->getRate()));
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
                return pBuffer->getNumFrames()-bufferLeftToFill/
                    pBuffer->getFrameSize();
            }
        }
    }
    return pBuffer->getNumFrames();
}
        
VideoMsgPtr AsyncVideoDecoder::getBmpsForTime(long long timeWanted, 
        FrameAvailableCode& FrameAvailable)
{
    if (timeWanted < 0 && timeWanted != -1) {
        cerr << "Illegal timeWanted: " << timeWanted << endl;
        AVG_ASSERT(false);
    }
    // XXX: This code is sort-of duplicated in FFMpegDecoder::readFrameForTime()
    long long FrameTime = -1000;
    VideoMsgPtr pFrameMsg;
    if (timeWanted == -1) {
        pFrameMsg = getNextBmps(true);
        FrameAvailable = FA_NEW_FRAME;
    } else {
        double TimePerFrame = 1000.0/getFPS();
        if (fabs(double(timeWanted-m_LastVideoFrameTime)) < 0.5*TimePerFrame || 
                m_LastVideoFrameTime > timeWanted+TimePerFrame) {
            // The last frame is still current. Display it again.
            FrameAvailable = FA_USE_LAST_FRAME;
            return VideoMsgPtr();
        } else {
            if (m_bVideoEOF) {
                FrameAvailable = FA_USE_LAST_FRAME;
                return VideoMsgPtr();
            }
            while (FrameTime-timeWanted < -0.5*TimePerFrame && !m_bVideoEOF) {
                returnFrame(pFrameMsg);
                pFrameMsg = getNextBmps(false);
                if (pFrameMsg) {
                    FrameTime = pFrameMsg->getFrameTime();
                } else {
                    FrameAvailable = FA_STILL_DECODING;
                    return VideoMsgPtr();
                }
            }
            if (!pFrameMsg) {
                cerr << "FrameTime=" << FrameTime << ", timeWanted=" << timeWanted 
                        << ", TimePerFrame=" << TimePerFrame << ", m_bVideoEOF=" 
                        << m_bVideoEOF << endl;
                AVG_ASSERT(false);
            }
            FrameAvailable = FA_NEW_FRAME;
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
    scoped_lock Lock(m_SeekMutex);
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
