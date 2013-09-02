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

#ifndef _VideoWriterThread_H_
#define _VideoWriterThread_H_

#include "../api.h"

#include "../base/WorkerThread.h"
#include "../graphics/Bitmap.h"
#include "../video/WrapFFMpeg.h"

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>


#include <string>

namespace avg {

class AVG_API VideoWriterThread : public WorkerThread<VideoWriterThread>  {
    public:
        VideoWriterThread(CQueue& cmdQueue, const std::string& sFilename, IntPoint size,
                int frameRate, int qMin, int qMax);
        virtual ~VideoWriterThread();

        void encodeYUVFrame(BitmapPtr pBmp);
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

        AVFrame* createFrame(AVPixelFormat pixelFormat, IntPoint size);

        void convertRGBImage(BitmapPtr pSrcBmp);
        void convertYUVImage(BitmapPtr pSrcBmp);
        void writeFrame(AVFrame* pFrame);

        std::string m_sFilename;
        IntPoint m_Size;
        int m_FrameRate;
        int m_QMin;
        int m_QMax;
        
        AVOutputFormat* m_pOutputFormat;
        AVFormatContext* m_pOutputFormatContext;
        AVStream* m_pVideoStream;
        SwsContext* m_pFrameConversionContext;
        AVFrame* m_pConvertedFrame;
        unsigned char* m_pPictureBuffer;
        unsigned char* m_pVideoBuffer;
        int m_FramesWritten;
};

}
#endif
