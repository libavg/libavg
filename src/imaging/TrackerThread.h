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
#include "TrackerConfig.h"
#include "AsyncCamera.h"
#include "ConnectedComps.h"
#include "FilterDistortion.h"

#include "../base/WorkerThread.h"
#include "../base/Command.h"

#include "../graphics/HistoryPreProcessor.h"
#include "../graphics/Bitmap.h"

#include <boost/thread.hpp>

#include <string>
#include <list>

namespace avg {

typedef enum {
        TRACKER_IMG_CAMERA,
        TRACKER_IMG_DISTORTED,
        TRACKER_IMG_NOHISTORY,
        TRACKER_IMG_HISTOGRAM,
        TRACKER_IMG_HIGHPASS,
        TRACKER_IMG_FINGERS,
        NUM_TRACKER_IMAGES
} TrackerImageID;

typedef boost::shared_ptr<boost::mutex> MutexPtr;

class IBlobTarget {
    public:
        virtual ~IBlobTarget() {};
        virtual void update(BlobListPtr blobs, BitmapPtr pBitmap) = 0;
};


class TrackerThread: public WorkerThread<TrackerThread>
{
    public:
        TrackerThread(CameraPtr pCamera, 
                BitmapPtr ppBitmaps[NUM_TRACKER_IMAGES],
                MutexPtr pMutex,
                CmdQueue& CmdQ,
                IBlobTarget *target,
                bool bSubtractHistory);
        virtual ~TrackerThread();

        bool init();
        bool work();
        void deinit();

        void setConfig(TrackerConfig Config);
        void setBitmaps(IntRect ROI, BitmapPtr ppBitmaps[NUM_TRACKER_IMAGES]);
        void resetHistory();
    
    private:
        void checkMessages();
        void calcHistory();
        bool isfinger(BlobPtr blob);
        void drawHistogram(BitmapPtr pDestBmp, BitmapPtr pSrcBmp);

        std::string m_sDevice;
        double m_FrameRate;
        std::string m_sMode;

        int m_Threshold;
        BlobListPtr m_pBlobList;
        BitmapPtr m_pBitmaps[NUM_TRACKER_IMAGES];
        MutexPtr m_pMutex;

        CameraPtr  m_pCamera;
        IBlobTarget *m_pTarget;
        HistoryPreProcessorPtr m_pHistoryPreProcessor;
        FilterDistortionPtr m_pDistorter;
        IntRect m_ROI;
        bool m_bDebugEnabled;
};

}

#endif

