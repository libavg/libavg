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

#include "VideoDemuxerThread.h"

#include "../base/Logger.h"
#include "../base/TimeSource.h"

#include <climits>

using namespace std;

namespace avg {

VideoDemuxerThread::VideoDemuxerThread(CQueue& cmdQ, AVFormatContext * pFormatContext,
        const map<int, VideoMsgQueuePtr>& packetQs)
    : WorkerThread<VideoDemuxerThread>("VideoDemuxer", cmdQ),
      m_PacketQs(packetQs),
      m_bEOF(false),
      m_pFormatContext(pFormatContext),
      m_pDemuxer()
{
    map<int, VideoMsgQueuePtr>::iterator it;
    for (it = m_PacketQs.begin(); it != m_PacketQs.end(); it++) {
        int streamIndex = it->first;
        m_PacketQbEOF[streamIndex] = false;
    }
}

VideoDemuxerThread::~VideoDemuxerThread()
{
}

bool VideoDemuxerThread::init()
{
    map<int, VideoMsgQueuePtr>::iterator it;
    vector<int> streamIndexes;
    for (it = m_PacketQs.begin(); it != m_PacketQs.end(); it++) {
        streamIndexes.push_back(it->first);
    }
    m_pDemuxer = FFMpegDemuxerPtr(new FFMpegDemuxer(m_pFormatContext, streamIndexes));
    return true;
}

bool VideoDemuxerThread::work() 
{
    if (m_bEOF) {
        waitForCommand();
    } else {
        map<int, VideoMsgQueuePtr>::iterator it;
        int shortestQ = -1;
        int shortestLength = INT_MAX;
        for (it = m_PacketQs.begin(); it != m_PacketQs.end(); it++) {
            if (it->second->size() < shortestLength && 
                    it->second->size() < it->second->getMaxSize() &&
                    !m_PacketQbEOF[it->first])
            {
                shortestLength = it->second->size();
                shortestQ = it->first;
            }
        }
        
        if (shortestQ < 0) {
            // All queues are at their max capacity. Take a nap and try again later.
            msleep(10);
            return true;
        }
        int dummy;
        m_pDemuxer->isSeekDone(shortestQ, dummy); // Ignore here - handled in seek() 

        AVPacket * pPacket = m_pDemuxer->getPacket(shortestQ);
        VideoMsgPtr pMsg(new VideoMsg);
        if (pPacket == 0) {
            onStreamEOF(shortestQ);
            pMsg->setEOF();
        } else {
            pMsg->setPacket(pPacket);
        }
        m_PacketQs[shortestQ]->push(pMsg);
        msleep(0);
    }
    return true;
}

void VideoDemuxerThread::deinit()
{
}

void VideoDemuxerThread::seek(int seqNum, float destTime)
{
    map<int, VideoMsgQueuePtr>::iterator it;
    m_pDemuxer->seek(destTime);
    for (it = m_PacketQs.begin(); it != m_PacketQs.end(); it++) {
        VideoMsgQueuePtr pPacketQ = it->second;
        clearQueue(pPacketQ);

        // send SEEK_DONE
        VideoMsgPtr pMsg(new VideoMsg);
        pMsg->setSeekDone(seqNum, destTime);
        pPacketQ->push(pMsg);
        m_PacketQbEOF[it->first] = false;
    }
    m_bEOF = false;
}
       
void VideoDemuxerThread::close()
{
    map<int, VideoMsgQueuePtr>::iterator it;
    for (it = m_PacketQs.begin(); it != m_PacketQs.end(); it++) {
        VideoMsgQueuePtr pPacketQ = it->second;
        clearQueue(pPacketQ);

        VideoMsgPtr pMsg(new VideoMsg);
        pMsg->setClosed();
        pPacketQ->push(pMsg);
        m_PacketQbEOF[it->first] = false;
    }
    stop();
}
        
void VideoDemuxerThread::onStreamEOF(int streamIndex)
{
    m_PacketQbEOF[streamIndex] = true;
                
    m_bEOF = true;
    map<int, bool>::iterator it;
    for (it = m_PacketQbEOF.begin(); it != m_PacketQbEOF.end(); it++) {
        if (!it->second) {
            m_bEOF = false;
            break;
        }
    }
}
        
void VideoDemuxerThread::clearQueue(VideoMsgQueuePtr pPacketQ)
{
    VideoMsgPtr pMsg;
    do {
        pMsg = pPacketQ->pop(false);
        if (pMsg) {
            pMsg->freePacket();
        }
    } while (pMsg);
}

}
