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

#ifndef _FFMpegDemuxer_H_
#define _FFMpegDemuxer_H_

#include "../avgconfigwrapper.h"
#include "IDemuxer.h"

#include "WrapFFMpeg.h"

#include <list>
#include <vector>
#include <map>

#include <boost/shared_ptr.hpp>

namespace avg {

    class AVG_API FFMpegDemuxer: public IDemuxer {
        public:
            FFMpegDemuxer(AVFormatContext * pFormatContext, 
                    std::vector<int> streamIndexes);
            virtual ~FFMpegDemuxer();
           
            AVPacket * getPacket(int streamIndex);
            void seek(float destTime);
            void dump();
            
        private:
            void clearPacketCache();

            typedef std::list<AVPacket *> PacketList;
            std::map<int, PacketList> m_PacketLists;
           
            AVFormatContext * m_pFormatContext;
    };
    typedef boost::shared_ptr<FFMpegDemuxer> FFMpegDemuxerPtr;
}

#endif
