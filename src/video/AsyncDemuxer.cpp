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

#include "AsyncDemuxer.h"
#include "../base/ScopeTimer.h"

#include <boost/bind.hpp>

#include <iostream>

using namespace std;

namespace avg {

AsyncDemuxer::AsyncDemuxer(AVFormatContext * pFormatContext)
    : m_pCmdQ(new VideoDemuxerThread::CmdQueue)
{
    m_pSyncDemuxer = IDemuxerPtr(new FFMpegDemuxer(pFormatContext));
    m_pDemuxThread = new boost::thread(VideoDemuxerThread(*m_pCmdQ, pFormatContext));
}

AsyncDemuxer::~AsyncDemuxer()
{
    if (m_pDemuxThread) {
        m_pCmdQ->push(Command<VideoDemuxerThread>(boost::bind(
                &VideoDemuxerThread::stop, _1)));
        map<int, VideoPacketQueuePtr>::iterator it;
        for (it=m_PacketQs.begin(); it != m_PacketQs.end(); ++it) {
            // If the Queue is full, this breaks the lock in the thread.
            it->second->pop(false);
        }
        m_pDemuxThread->join();
        delete m_pDemuxThread;
        m_pDemuxThread = 0;
    }
}

void AsyncDemuxer::enableStream(int StreamIndex)
{
    VideoPacketQueuePtr pPacketQ(new VideoPacketQueue(100));
    m_PacketQs[StreamIndex] = pPacketQ;
    m_pCmdQ->push(Command<VideoDemuxerThread>(boost::bind(
                &VideoDemuxerThread::enableStream, _1, pPacketQ, StreamIndex)));
}

AVPacket * AsyncDemuxer::getPacket(int StreamIndex)
{
    // TODO: This blocks if there is no packet. Is that ok?
    PacketVideoMsgPtr pPacketMsg = m_PacketQs[StreamIndex]->pop(true);

    return pPacketMsg->getPacket();
}

void AsyncDemuxer::seek(int DestFrame, int StreamIndex)
{
    m_pCmdQ->push(Command<VideoDemuxerThread>(boost::bind(
                &VideoDemuxerThread::seek, _1, DestFrame, StreamIndex)));
}

}
