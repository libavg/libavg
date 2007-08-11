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

#include "VideoDemuxerThread.h"
#include "FrameVideoMsg.h"
#include "InfoVideoMsg.h"
#include "ErrorVideoMsg.h"
#include "EOFVideoMsg.h"

#include "../base/Logger.h"
#include "../base/TimeSource.h"

#include <climits>

using namespace std;

namespace avg {

VideoDemuxerThread::VideoDemuxerThread(CmdQueue& CmdQ, 
                AVFormatContext * pFormatContext)
    : WorkerThread<VideoDemuxerThread>("VideoDemuxer", CmdQ),
      m_bEOF(false),
      m_pFormatContext(pFormatContext),
      m_pDemuxer()

{
}

VideoDemuxerThread::~VideoDemuxerThread()
{
}

bool VideoDemuxerThread::init()
{
    m_pDemuxer = FFMpegDemuxerPtr(new FFMpegDemuxer(m_pFormatContext));
    return true;
};

bool VideoDemuxerThread::work() 
{
    if (m_PacketQs.empty() || m_bEOF) {
        // replace this with waitForMessage()
        TimeSource::get()->msleep(10);
    } else {

        map<int, VideoPacketQueuePtr>::iterator it;
        int ShortestQ=0;
        int ShortestLength = INT_MAX;
        for (it=m_PacketQs.begin(); it != m_PacketQs.end(); it++) {
            if (it->second->size() < ShortestLength) {
                ShortestLength = it->second->size();
                ShortestQ = it->first;
            }
        }
        // TODO: This might lose some packets on eof if there is more than one stream.
        AVPacket * pPacket = m_pDemuxer->getPacket(ShortestQ);
        if (pPacket == 0) {
            m_bEOF = true;
        }
        m_PacketQs[ShortestQ]->push(PacketVideoMsgPtr(new PacketVideoMsg(pPacket, false)));
    }
    return true;
}

void VideoDemuxerThread::deinit()
{
}

void VideoDemuxerThread::enableStream(VideoPacketQueuePtr pPacketQ, int StreamIndex)
{
    m_PacketQs[StreamIndex] = pPacketQ;
    m_pDemuxer->enableStream(StreamIndex);
}

void VideoDemuxerThread::seek(int DestFrame, int StartTimestamp, int StreamIndex)
{
    map<int, VideoPacketQueuePtr>::iterator it;
    m_pDemuxer->seek(DestFrame, StartTimestamp, StreamIndex);
    for (it=m_PacketQs.begin(); it != m_PacketQs.end(); it++) {
        VideoPacketQueuePtr pPacketQ = it->second;
        pPacketQ->push(PacketVideoMsgPtr(new PacketVideoMsg(0, true)));
    }
    m_bEOF = false;
}

}
