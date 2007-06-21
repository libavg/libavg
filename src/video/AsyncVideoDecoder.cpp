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

#include "AsyncVideoDecoder.h"
#include "EOFVideoMsg.h"
#include "InfoVideoMsg.h"
#include "ErrorVideoMsg.h"

#include "../base/ObjectCounter.h"

#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

#include <iostream>

using namespace boost;
using namespace std;

namespace avg {

AsyncVideoDecoder::AsyncVideoDecoder(VideoDecoderPtr pSyncDecoder)
    : m_pSyncDecoder(pSyncDecoder),
      m_pDecoderThread(0),
      m_Size(0,0),
      m_bEOF(false),
      m_bSeekPending(false)
{
    ObjectCounter::get()->incRef(&typeid(*this));
}

AsyncVideoDecoder::~AsyncVideoDecoder()
{
    if (m_pDecoderThread) {
        close();
    }
    ObjectCounter::get()->decRef(&typeid(*this));
}

void AsyncVideoDecoder::open(const std::string& sFilename, YCbCrMode ycbcrMode,
        bool bThreadedDemuxer)
{
    m_bEOF = false;
    m_bSeekPending = false;
    m_sFilename = sFilename;
    m_pCmdQ = VideoDecoderThread::CmdQueuePtr(new VideoDecoderThread::CmdQueue);
    m_pMsgQ = VideoMsgQueuePtr(new VideoMsgQueue(8));
    m_pDecoderThread = new boost::thread(
            VideoDecoderThread(*m_pMsgQ, *m_pCmdQ, m_pSyncDecoder, sFilename, 
                    ycbcrMode, bThreadedDemuxer));
    getInfoMsg();
}

void AsyncVideoDecoder::close()
{
    if (m_pDecoderThread) {
        m_pCmdQ->push(Command<VideoDecoderThread>(boost::bind(
                &VideoDecoderThread::stop, _1)));
        getNextBmps(false); // If the Queue is full, this breaks the lock in the thread.
        m_pDecoderThread->join();
        delete m_pDecoderThread;
        m_pDecoderThread = 0;
    }
}

void AsyncVideoDecoder::seek(int DestFrame)
{
    waitForSeekDone();
    m_bEOF = false;
    m_pCmdQ->push(Command<VideoDecoderThread>(boost::bind(
                &VideoDecoderThread::seek, _1, DestFrame)));
    m_bSeekPending = true;
}

IntPoint AsyncVideoDecoder::getSize()
{
    return m_Size;
}

int AsyncVideoDecoder::getNumFrames()
{
    assert(m_pDecoderThread);
    return m_NumFrames;
}

double AsyncVideoDecoder::getFPS()
{
    assert(m_pDecoderThread);
    return m_FPS;
}

PixelFormat AsyncVideoDecoder::getPixelFormat()
{
    assert(m_pDecoderThread);
    return m_PF;
}

bool AsyncVideoDecoder::renderToBmp(BitmapPtr pBmp, double TimeWanted)
{
    FrameVideoMsgPtr pFrameMsg = getNextBmps(TimeWanted == -1);
    if (pFrameMsg) {
        *pBmp = *(pFrameMsg->getBitmap(0));
        return true;
    } else {
        return false;
    }
}

bool AsyncVideoDecoder::renderToYCbCr420p(BitmapPtr pBmpY, BitmapPtr pBmpCb, 
       BitmapPtr pBmpCr, double TimeWanted)
{
    FrameVideoMsgPtr pFrameMsg = getNextBmps(TimeWanted == -1);
    if (pFrameMsg) {
        pBmpY->copyPixels(*(pFrameMsg->getBitmap(0)));
        pBmpCb->copyPixels(*(pFrameMsg->getBitmap(1)));
        pBmpCr->copyPixels(*(pFrameMsg->getBitmap(2)));
        return true;
    } else {
        return false;
    }
}

bool AsyncVideoDecoder::isEOF()
{
    return m_bEOF;
}

void AsyncVideoDecoder::getInfoMsg()
{
    VideoMsgPtr pMsg = m_pMsgQ->pop(true);
    InfoVideoMsgPtr pInfoMsg = dynamic_pointer_cast<InfoVideoMsg>(pMsg);
    ErrorVideoMsgPtr pErrorMsg(dynamic_pointer_cast<ErrorVideoMsg>(pMsg));
    if (pErrorMsg) {
        close();
        throw(pErrorMsg->getException());
    } else {
        assert(pInfoMsg); // The first message sent by the decoder thread after open
        // should be an info or error message.
        m_Size = pInfoMsg->getSize();
        m_NumFrames = pInfoMsg->getNumFrames();
        m_FPS = pInfoMsg->getFPS();
        m_PF = pInfoMsg->getPF();
    }
}

FrameVideoMsgPtr AsyncVideoDecoder::getNextBmps(bool bWait)
{
    try {
        waitForSeekDone();
        VideoMsgPtr pMsg = m_pMsgQ->pop(bWait);
        FrameVideoMsgPtr pFrameMsg = dynamic_pointer_cast<FrameVideoMsg>(pMsg);
        while (!pFrameMsg) {
            EOFVideoMsgPtr pEOFMsg(dynamic_pointer_cast<EOFVideoMsg>(pMsg));
            ErrorVideoMsgPtr pErrorMsg(dynamic_pointer_cast<ErrorVideoMsg>(pMsg));
            if (pEOFMsg) {
                m_bEOF = true;
                return FrameVideoMsgPtr();
            } else if(pErrorMsg) {
                m_bEOF = true;
                close();
                return FrameVideoMsgPtr();
            } else {
                // Unhandled message type.
                assert(false);
            }
            VideoMsgPtr pMsg = m_pMsgQ->pop(bWait);
            FrameVideoMsgPtr pFrameMsg = dynamic_pointer_cast<FrameVideoMsg>(pMsg);
        }
        return pFrameMsg;
    } catch (Exception& e) {
        return FrameVideoMsgPtr();
    }
}

void AsyncVideoDecoder::waitForSeekDone()
{
    if (m_bSeekPending) {
        m_bSeekPending = false;
        VideoMsgPtr pMsg;
        bool bDone = false;
        do {
            VideoMsgPtr pMsg = m_pMsgQ->pop(true);
            FrameVideoMsgPtr pFrameMsg = dynamic_pointer_cast<FrameVideoMsg>(pMsg);
            if (pFrameMsg) {
                bDone = pFrameMsg->isSeekDone();
            }
        } while (!bDone);
    }
}

}
