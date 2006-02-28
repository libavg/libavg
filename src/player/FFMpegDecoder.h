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

#include <ffmpeg/avformat.h>

namespace avg {

class FFMpegDecoder: public IVideoDecoder
{
    public:
        FFMpegDecoder();
        virtual ~FFMpegDecoder();
        virtual bool open(const std::string& sFilename, 
                int* pWidth, int* pHeight);
        virtual void close();
        virtual void seek(int DestFrame, int CurFrame);
        virtual int getNumFrames();
        virtual double getFPS();
        virtual bool renderToBmp(BitmapPtr pBmp);
        virtual bool renderToYCbCr420p(BitmapPtr pBmpY, BitmapPtr pBmpCb, 
                BitmapPtr pBmpCr);
        virtual bool canRenderToBuffer(int BPP);
        bool isYCbCrSupported(); 

    private:
        void initVideoSupport();
        void readFrame(AVFrame& Frame);
        bool getNextVideoPacket(AVPacket & Packet);

        AVFormatContext * m_pFormatContext;
        int m_VStreamIndex;
        AVStream * m_pVStream;
        bool m_bEOF;

        unsigned char * m_pPacketData;
        AVPacket m_Packet;
        int m_PacketLenLeft;
        bool m_bFirstPacket;
        std::string m_sFilename;

        static bool m_bInitialized;
};

}
#endif 

