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

#ifndef _VideoWriter_H_
#define _VideoWriter_H_

#include "../api.h"

#include "VideoWriterThread.h"

#include "../base/IFrameEndListener.h"
#include "../base/IPlaybackEndListener.h"
#include "../base/Point.h"

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include <string>

namespace avg {

class Canvas;

class AVG_API VideoWriter : public IFrameEndListener, IPlaybackEndListener  {
    public:
        VideoWriter(Canvas* pCanvas, const std::string& sOutFileName,
                int frameRate=30, int qMin=3, int qMax=5, bool bSyncToPlayback=true);
        virtual ~VideoWriter();
        void stop();

        std::string getFileName() const;
        int getFramerate() const;
        int getQMin() const;
        int getQMax() const;

        virtual void onFrameEnd();
        virtual void onPlaybackEnd();

    private:
        void handleFrame();
        void handleAutoSynchronizedFrame();

        void addFrame(BitmapPtr pBitmap);
        void writeDummyFrame();

        Canvas* m_pCanvas;
        std::string m_sOutFileName;
        int m_FrameRate;
        int m_QMin;
        int m_QMax;
        IntPoint m_FrameSize;

        bool m_bHasValidData;

        VideoWriterThread::CQueue m_CmdQueue;
        boost::thread* m_pThread;
        bool m_bSyncToPlayback;

        bool m_bStopped;

        int m_CurFrame;
        long long m_StartTime;
};

}
#endif
