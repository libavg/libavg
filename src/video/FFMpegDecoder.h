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

#ifndef _FFMpegDecoder_H_
#define _FFMpegDecoder_H_

#include "IVideoDecoder.h"

#ifdef _WIN32
#define EMULATE_INTTYPES
#endif
#include <ffmpeg/avformat.h>

#include <boost/thread/mutex.hpp>

namespace avg {

class FFMpegDecoder: public IVideoDecoder
{
    public:
        FFMpegDecoder();
        virtual ~FFMpegDecoder();
        virtual void open(const std::string& sFilename, YCbCrMode ycbcrMode);
        virtual void close();
        virtual void seek(int DestFrame);
        virtual IntPoint getSize();
        virtual int getNumFrames();
        virtual double getFPS();
        virtual bool renderToBmp(BitmapPtr pBmp);
        virtual bool renderToYCbCr420p(BitmapPtr pBmpY, BitmapPtr pBmpCb, 
                BitmapPtr pBmpCr);
        virtual PixelFormat getPixelFormat();

    private:
        void initVideoSupport();
        void readFrame(AVFrame& Frame);
        bool getNextVideoPacket(AVPacket & Packet);
        PixelFormat calcPixelFormat(YCbCrMode ycbcrMode);

        AVFormatContext * m_pFormatContext;
        int m_VStreamIndex;
        AVStream * m_pVStream;
        bool m_bEOF;
        PixelFormat m_PF;

        unsigned char * m_pPacketData;
        AVPacket m_Packet;
        int m_PacketLenLeft;
        bool m_bFirstPacket;
        std::string m_sFilename;
        IntPoint m_Size;

        static bool m_bInitialized;
        // Prevents different decoder instances from executing open/close simultaneously
        static boost::mutex s_OpenMutex;   
};

}
#endif 

