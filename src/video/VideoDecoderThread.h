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

#ifndef _VideoDecoderThread_H_
#define _VideoDecoderThread_H_

#include "IVideoDecoder.h"
#include "IVideoMsg.h"

#include "../base/WorkerThread.h"
#include "../base/Command.h"

#include <boost/thread.hpp>

#include <string>

namespace avg {

class VideoDecoderThread: public WorkerThread<VideoDecoderThread> {
    public:
        VideoDecoderThread(VideoMsgQueue& MsgQ, CmdQueue& CmdQ, 
                VideoDecoderPtr pDecoder, const std::string& sFilename, 
                YCbCrMode ycbcrMode);
        virtual ~VideoDecoderThread();
        bool init();
        bool work();
        void deinit();

        void seek(int DestFrame);

    private:
        VideoMsgQueue& m_MsgQ;
        VideoDecoderPtr m_pDecoder;
        std::string m_sFilename;
        YCbCrMode m_YCbCrMode;
};

}
#endif 

