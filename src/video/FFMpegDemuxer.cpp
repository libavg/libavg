//=============================================================================
// Copyright (C) 2004-2006, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================

#include "FFMpegDemuxer.h"
#include "../base/ScopeTimer.h"

#include <iostream>

using namespace std;

namespace avg {

FFMpegDemuxer::FFMpegDemuxer(AVFormatContext * pFormatContext)
    : m_pFormatContext(pFormatContext)
{
}

FFMpegDemuxer::~FFMpegDemuxer()
{
    clearPacketCache();
}

void FFMpegDemuxer::enableStream(int StreamIndex)
{
    m_PacketLists[StreamIndex] = PacketList();
}

static ProfilingZone VideoPacketProfilingZone("        FFMpegDemuxer: getPacket");

AVPacket * FFMpegDemuxer::getPacket(int StreamIndex)
{
    // Make sure enableStream was called on StreamIndex.
    assert (m_PacketLists.find(StreamIndex) != m_PacketLists.end());
    PacketList & CurPacketList = m_PacketLists.find(StreamIndex)->second;
    AVPacket * pPacket;
    if (!CurPacketList.empty()) {
        pPacket = CurPacketList.front();
        CurPacketList.pop_front();
        return pPacket;
    } else {
        do {
            pPacket = new AVPacket;
            memset(pPacket, 0, sizeof(AVPacket));
            int err = av_read_frame(m_pFormatContext, pPacket); 
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
            }
        } while (!pPacket || pPacket->stream_index != StreamIndex);
        return pPacket; 
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
    cerr << "FFMpegDemuxer list sizes: " << endl;
    map<int, PacketList>::iterator it;
    for (it=m_PacketLists.begin(); it != m_PacketLists.end(); ++it) {
        cerr << "  " << it->second.size() << endl;
    }
}

}
