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

#ifndef _IDemuxer_H_
#define _IDemuxer_H_

#ifdef _WIN32
#define EMULATE_INTTYPES
#else
// This is probably GCC-specific.
#define INT64_C(c)    c ## L
#endif
#include <ffmpeg/avformat.h>

#include <boost/shared_ptr.hpp>

namespace avg {

    class IDemuxer {
        public:
            virtual ~IDemuxer() {};
           
            virtual void enableStream(int StreamIndex) = 0;
            virtual AVPacket * getPacket(int StreamIndex) = 0;
            virtual void seek(int DestFrame, int StartTimestamp, int StreamIndex) = 0;
            
    };
    typedef boost::shared_ptr<IDemuxer> IDemuxerPtr;
}

#endif
