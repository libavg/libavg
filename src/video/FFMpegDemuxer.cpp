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

#include "FFMpegDemuxer.h"
#include "../base/ScopeTimer.h"
#include "../base/ObjectCounter.h"

#include <iostream>

using namespace std;

namespace avg {

FFMpegDemuxer::FFMpegDemuxer(AVFormatContext * pFormatContext)
    : m_pFormatContext(pFormatContext)
{
    ObjectCounter::get()->incRef(&typeid(*this));
}

FFMpegDemuxer::~FFMpegDemuxer()
{
    clearPacketCache();
    ObjectCounter::get()->decRef(&typeid(*this));
}

void FFMpegDemuxer::enableStream(int StreamIndex)
{
    m_PacketLists[StreamIndex] = PacketList();
}

AVPacket * FFMpegDemuxer::getPacket(int StreamIndex)
{
    // Make sure enableStream was called on StreamIndex.
    assert (m_PacketLists.find(StreamIndex) != m_PacketLists.end());
    PacketList & CurPacketList = m_PacketLists.find(StreamIndex)->second;
    AVPacket * pPacket;
    if (!CurPacketList.empty()) {
        pPacket = CurPacketList.front();
        CurPacketList.pop_front();
    } else {
        do {
            pPacket = new AVPacket;
            memset(pPacket, 0, sizeof(AVPacket));
            int err = av_read_frame(m_pFormatContext, pPacket);
            // TODO: Check url_ferror here too.
            if (err < 0) {
                av_free_packet(pPacket);
                delete pPacket;
                pPacket = 0;
                return 0;
            }
            if (pPacket->stream_index != StreamIndex) {
                if (m_PacketLists.find(pPacket->stream_index) != m_PacketLists.end()) {
                    av_dup_packet(pPacket);
                    PacketList& OtherPacketList = 
                            m_PacketLists.find(pPacket->stream_index)->second;
                    OtherPacketList.push_back(pPacket);
                } else {
                    av_free_packet(pPacket);
                    delete pPacket;
                    pPacket = 0;
                } 
            } else {
                av_dup_packet(pPacket);
            }
        } while (!pPacket || pPacket->stream_index != StreamIndex);
    }
    return pPacket;
}

void FFMpegDemuxer::seek(int DestFrame, int StartTimestamp, int StreamIndex)
{
    AVStream * pVStream = m_pFormatContext->streams[StreamIndex];
#if LIBAVFORMAT_BUILD <= 4616
    av_seek_frame(m_pFormatContext, StreamIndex, 
            int((double(DestFrame)*1000000*1000)/pVStream->r_frame_rate+StartTimestamp));
#else
#if LIBAVFORMAT_BUILD < ((49<<16)+(0<<8)+0)
    av_seek_frame(m_pFormatContext, StreamIndex, 
            int((double(DestFrame)*1000000*1000)/pVStream->r_frame_rate+StartTimestamp), 0);
#else
    double framerate = (pVStream->r_frame_rate.num)/pVStream->r_frame_rate.den;
    double FrameStartOffset = framerate*StartTimestamp/1000.0;
    av_seek_frame(m_pFormatContext, -1, 
            int((double(DestFrame+FrameStartOffset)*AV_TIME_BASE)/framerate),
            AVSEEK_FLAG_BACKWARD);
#endif
#endif
    clearPacketCache();
    map<int, PacketList>::iterator it;
    for (it=m_PacketLists.begin(); it != m_PacketLists.end(); ++it) {
        int CurStreamIndex = it->first;
        AVStream * pStream = m_pFormatContext->streams[CurStreamIndex];
        avcodec_flush_buffers(pStream->codec);
    }
}

void FFMpegDemuxer::clearPacketCache()
{
    map<int, PacketList>::iterator it;
    for (it=m_PacketLists.begin(); it != m_PacketLists.end(); ++it) {
        PacketList::iterator it2;
        PacketList* thePacketList = &(it->second);
        for (it2=thePacketList->begin(); it2 != thePacketList->end(); ++it2) {
            av_free_packet(*it2);
            delete *it2;
        }
        thePacketList->clear();
    }
}

void FFMpegDemuxer::dump()
{
    map<int, PacketList>::iterator it;
    for (it=m_PacketLists.begin(); it != m_PacketLists.end(); ++it) {
        cerr << "  " << it->second.size() << endl;
    }
}

}
