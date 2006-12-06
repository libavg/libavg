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

#ifndef _TrackerThread_H_
#define _TrackerThread_H_
#include "TrackerCmd.h"
#include "Camera.h"
#include "ConnectedComps.h"

#include "../graphics/Bitmap.h"
#include "../base/WorkerThread.h"
#include "../base/Command.h"

#include <boost/thread.hpp>

#include <string>
#include <list>

namespace avg {

typedef enum {
        TRACKER_IMG_CAMERA,
        TRACKER_IMG_HISTORY,
        TRACKER_IMG_NOHISTORY,
        TRACKER_IMG_COMPONENTS,
        NUM_TRACKER_IMAGES
} TrackerImageID;

typedef boost::shared_ptr<boost::mutex> MutexPtr;

class IBlobTarget {
    public:
        virtual ~IBlobTarget() {};
        virtual void update(BlobListPtr blobs) = 0;
};


class TrackerThread: public WorkerThread<TrackerThread>
{
    public:
        TrackerThread(CameraPtr pCamera, int Threshold, 
                BitmapPtr ppBitmaps[NUM_TRACKER_IMAGES],
                MutexPtr pMutex,
                CmdQueue& CmdQ,
                IBlobTarget *target);
        virtual ~TrackerThread();

        bool init();
        bool work();
        void deinit();

        void setThreshold(int Threshold); 
        void setBrightness(int Brightness); 
        void setExposure(int Exposure); 
        void setGain(int Gain); 
        void setShutter(int Shutter); 

    private:
        void checkMessages();
        void calcHistory();
        bool isfinger(BlobPtr blob);
        BitmapPtr subtractHistory();

        std::string m_sDevice;
        double m_FrameRate;
        std::string m_sMode;

        int m_Threshold;
        BlobListPtr m_pBlobList;
        BitmapPtr m_pBitmaps[NUM_TRACKER_IMAGES];
        MutexPtr m_pMutex;
        BitmapPtr m_pHistoryBmp;
        bool m_bHistoryInitialized;

        CameraPtr  m_pCamera;
        IBlobTarget *m_pTarget;
};

}

#endif

