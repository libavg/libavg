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

#include "AsyncDemuxer.h"

#include "../base/ScopeTimer.h"
#include "../base/Exception.h"
#include "../base/ObjectCounter.h"

#include <boost/bind.hpp>

#include <iostream>

#define PACKET_QUEUE_LENGTH 50

using namespace std;

typedef boost::mutex::scoped_lock scoped_lock;

namespace avg {

AsyncDemuxer::AsyncDemuxer(AVFormatContext * pFormatContext, vector<int> streamIndexes)
    : m_pCmdQ(new VideoDemuxerThread::CQueue),
      m_pFormatContext(pFormatContext)
{
    ObjectCounter::get()->incRef(&typeid(*this));
    for (unsigned i = 0; i < streamIndexes.size(); ++i) {
        enableStream(streamIndexes[i]);
    }
    m_pDemuxThread = new boost::thread(VideoDemuxerThread(*m_pCmdQ, m_pFormatContext,
            m_PacketQs));
}

AsyncDemuxer::~AsyncDemuxer()
{
    if (m_pDemuxThread) {
        m_pCmdQ->pushCmd(boost::bind(&VideoDemuxerThread::stop, _1));
        map<int, VideoMsgQueuePtr>::iterator it;
        for (it = m_PacketQs.begin(); it != m_PacketQs.end(); ++it) {
            // If the Queue is full, this breaks the lock in the thread.
            VideoMsgPtr pPacketMsg;
            pPacketMsg = it->second->pop(false);
            if (pPacketMsg) {
                pPacketMsg->freePacket();
            }
        }
        m_pDemuxThread->join();
        delete m_pDemuxThread;
        m_pDemuxThread = 0;
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
    ObjectCounter::get()->decRef(&typeid(*this));
}

AVPacket * AsyncDemuxer::getPacket(int streamIndex)
{
//    cerr << "  AsyncDemuxer::getPacket" << endl;
    AVG_ASSERT(m_pCurMsgs[streamIndex]);
    return checkPacket(streamIndex);
}
            
AVPacket * AsyncDemuxer::checkPacket(int streamIndex)
{
//    cerr << "  AsyncDemuxer::checkPacket" << endl;
    VideoMsgPtr pMsg = m_pCurMsgs[streamIndex];
    if (!pMsg) {
        return 0;
    } else {
        m_pCurMsgs[streamIndex] = VideoMsgPtr();
        switch(pMsg->getType()) {
            case VideoMsg::PACKET:
    //            cerr << "PACKET " << pMsg->getPacket() << endl;
                return pMsg->getPacket();
            case VideoMsg::END_OF_FILE:
    //            cerr << "END_OF_FILE" << endl;
                return 0;
            case VideoMsg::CLOSED:
                cerr << "  AsyncDemuxer::CLOSED" << endl;
                m_bStreamClosed[streamIndex] = true;
                return 0;
            default:
                pMsg->dump();
                AVG_ASSERT(false);
                return 0;
        }
    }
}
            
float AsyncDemuxer::isSeekDone(int streamIndex, int& seqNum, bool bWait)
{
    m_pCurMsgs[streamIndex] = m_PacketQs[streamIndex]->pop(bWait);
    if (m_pCurMsgs[streamIndex] &&
            m_pCurMsgs[streamIndex]->getType() == VideoMsg::SEEK_DONE)
    {
        float seekTime = -1;
        while (m_pCurMsgs[streamIndex] &&
                m_pCurMsgs[streamIndex]->getType() == VideoMsg::SEEK_DONE)
        {
//            cerr << "  AsyncDemuxer::isSeekDone: true" << endl;
            seekTime = m_pCurMsgs[streamIndex]->getSeekTime();
            seqNum = m_pCurMsgs[streamIndex]->getSeekSeqNum();
     
            m_pCurMsgs[streamIndex] = m_PacketQs[streamIndex]->pop(bWait);
        }
        return seekTime;
    } else {
        return -1;
    }
}

bool AsyncDemuxer::isClosed(int streamIndex)
{
    return m_bStreamClosed[streamIndex];
}

void AsyncDemuxer::seek(int seqNum, float destTime)
{
    AVG_ASSERT(seqNum != -1); //TODO: Remove when audio works.
    m_pCmdQ->pushCmd(boost::bind(&VideoDemuxerThread::seek, _1, seqNum, destTime));
//    cerr << "  AsyncDemuxer::seek end" << endl;
}
            
void AsyncDemuxer::close()
{
//    cerr << "AsyncDemuxer::close" << endl;
    m_pCmdQ->pushCmd(boost::bind(&VideoDemuxerThread::close, _1));
    m_pDemuxThread->join();
}

VideoDemuxerThread::CQueuePtr AsyncDemuxer::getCmdQ()
{
    return m_pCmdQ;
}

void AsyncDemuxer::enableStream(int streamIndex)
{
    VideoMsgQueuePtr pPacketQ(new VideoMsgQueue(PACKET_QUEUE_LENGTH));
    m_PacketQs[streamIndex] = pPacketQ;
    m_pCurMsgs[streamIndex] = VideoMsgPtr();
    m_bStreamClosed[streamIndex] = false;
}

}
