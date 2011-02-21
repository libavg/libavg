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
        VideoWriterThread(CQueue& CmdQueue, const std::string& fileName, IntPoint size,
                int frameRate, int qMin, int qMax);
        virtual ~VideoWriterThread();
        
        void encodeFrame(BitmapPtr pBmp);

    private:
        void open(const std::string& sFileName, IntPoint size, int frameRate,
                int qMin, int qMax);
 
        // Called by base class
        virtual bool work();
        virtual void deinit();

        void setupVideoStream(int frameRate, int qMin, int qMax);
        void openVideoCodec();

        ffmpeg::AVFrame* createFrame(ffmpeg::PixelFormat pixelFormat, IntPoint size);

        void convertImage(BitmapPtr pBitmap);
        void writeFrame(ffmpeg::AVFrame* pFrame);

        IntPoint m_Size;
        
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
