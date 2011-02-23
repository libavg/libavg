//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2008 Ulrich von Zadow
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

#ifndef _VideoWriterThread_H_
#define _VideoWriterThread_H_

#include "../api.h"

#include "../base/WorkerThread.h"
#include "../graphics/Bitmap.h"

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#ifdef _WIN32
#define EMULATE_INTTYPES
#if !defined INT64_C
#define INT64_C(c) c##i64
#endif
#else
// This is probably GCC-specific.
#if !defined INT64_C
#if defined __WORDSIZE && __WORDSIZE == 64
#define INT64_C(c) c ## L
#else
#define INT64_C(c) c ## LL
#endif
#endif
#endif

namespace ffmpeg {
extern "C" {
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
}
}

#include <string>

namespace avg {

class AVG_API VideoWriterThread : public WorkerThread<VideoWriterThread>  {
    public:
        VideoWriterThread(CQueue& CmdQueue, const std::string& sFilename, IntPoint size,
                int frameRate, int qMin, int qMax);
        virtual ~VideoWriterThread();

        void encodeFrame(BitmapPtr pBmp);
        void close();

    private:
        bool init();
        void open();
 
        // Called by base class
        virtual bool work();
        virtual void deinit();

        void setupVideoStream();
        void openVideoCodec();

        ffmpeg::AVFrame* createFrame(ffmpeg::PixelFormat pixelFormat, IntPoint size);

        void convertImage(BitmapPtr pBitmap);
        void writeFrame(ffmpeg::AVFrame* pFrame);

        std::string m_sFilename;
        IntPoint m_Size;
        int m_FrameRate;
        int m_QMin;
        int m_QMax;
        
        ffmpeg::AVOutputFormat* m_pOutputFormat;
        ffmpeg::AVFormatContext* m_pOutputFormatContext;
        ffmpeg::AVStream* m_pVideoStream;
        ffmpeg::SwsContext* m_pFrameConversionContext;
        ffmpeg::AVFrame* m_pConvertedFrame;
        unsigned char* m_pPictureBuffer;
        unsigned char* m_pVideoBuffer;
        int m_VideoBufferSize;
        ffmpeg::PixelFormat m_StreamPixelFormat;
};

}
#endif
