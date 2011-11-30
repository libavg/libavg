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

#ifndef _AsyncDemuxer_H_
#define _AsyncDemuxer_H_

#include "../avgconfigwrapper.h"
#include "IDemuxer.h"
#include "VideoDemuxerThread.h"

#include <map>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

namespace avg {

    class AVG_API AsyncDemuxer: public IDemuxer {
        public:
            AsyncDemuxer(AVFormatContext * pFormatContext, 
                    std::vector<int> streamIndexes);
            virtual ~AsyncDemuxer();
           
            AVPacket * getPacket(int streamIndex);
            void seek(float destTime);
            
        private:
            void enableStream(int streamIndex);
            void waitForSeekDone();

            boost::thread* m_pDemuxThread;

            VideoDemuxerThread::CQueuePtr m_pCmdQ;
            std::map<int, VideoPacketQueuePtr> m_PacketQs;
            std::map<int, bool> m_bSeekDone;

            bool m_bSeekPending;
            AVFormatContext * m_pFormatContext;
            boost::mutex m_SeekMutex;
    };
    typedef boost::shared_ptr<AsyncDemuxer> AsyncDemuxerPtr;
}

#endif
