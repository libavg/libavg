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

#ifndef _TrackerThread_H_
#define _TrackerThread_H_

#include "../api.h"
#include "TrackerConfig.h"
#include "Camera.h"
#include "Blob.h"
#include "FilterDistortion.h"
#include "DeDistort.h"

#include "../base/WorkerThread.h"
#include "../base/Command.h"

#include "../graphics/HistoryPreProcessor.h"
#include "../graphics/Bitmap.h"
#include "../graphics/Pixel8.h"
#include "../graphics/Filter.h"

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
class OGLImagingContext;

class AVG_API IBlobTarget {
    public:
        virtual ~IBlobTarget() {};
        // Note that this function is called by TrackerThread in it's own thread!
        virtual void update(BlobVectorPtr pTrackBlobs, BlobVectorPtr pTouchBlobs,
                long long time) = 0;
};


class AVG_API TrackerThread: public WorkerThread<TrackerThread>
{
    public:
        TrackerThread(IntRect ROI, 
                CameraPtr pCamera, 
                BitmapPtr ppBitmaps[NUM_TRACKER_IMAGES],
                MutexPtr pMutex,
                CmdQueue& CmdQ,
                IBlobTarget *target,
                bool bSubtractHistory,
                TrackerConfig &config);
        virtual ~TrackerThread();

        bool init();
        bool work();
        void deinit();

        void setConfig(TrackerConfig Config, IntRect ROI, 
                BitmapPtr ppBitmaps[NUM_TRACKER_IMAGES]);
        void setDebugImages(bool bImg, bool bFinger);
        void resetHistory();
    
    private:
        void setBitmaps(IntRect ROI, BitmapPtr ppBitmaps[NUM_TRACKER_IMAGES]);
        void createBandpassFilter();
        void checkMessages();
        void calcHistory();
        void drawHistogram(BitmapPtr pDestBmp, BitmapPtr pSrcBmp);
        void calcBlobs(BitmapPtr pTrackBmp, BitmapPtr pTouchBmp, long long time);
        bool isRelevant(BlobPtr pBlob, int MinArea, int MaxArea,
                double MinEccentricity, double MaxEccentricity);
        BlobVectorPtr findRelevantBlobs(BlobVectorPtr pBlobs, bool bTouch);
        void drawBlobs(BlobVectorPtr pBlobs, BitmapPtr pSrcBmp, BitmapPtr pDestBmp,
                int Offset, bool bTouch);
        void calcContours(BlobVectorPtr pBlobs);
        void correlateHands(BlobVectorPtr pTrackBlobs, BlobVectorPtr pTouchBlobs);

        std::string m_sDevice;
        std::string m_sMode;

        TrackerConfigPtr m_pConfig;
        BitmapPtr m_pCameraMaskBmp;

        int m_TouchThreshold; // 0 => no touch events.
        int m_TrackThreshold; // 0 => no generic tracking events.
        int m_Prescale;
        int m_ClearBorder;
        bool m_bTrackBrighter;
        BlobVectorPtr m_pBlobVector;
        IntRect m_ROI;
        BitmapPtr m_pBitmaps[NUM_TRACKER_IMAGES];
        MutexPtr m_pMutex;

        CameraPtr  m_pCamera;
        IBlobTarget *m_pTarget;
        HistoryPreProcessorPtr m_pHistoryPreProcessor;
        FilterDistortionPtr m_pDistorter;
        DeDistortPtr m_pTrafo;
        bool m_bCreateDebugImages;
        bool m_bCreateFingerImage;
        int m_NumFrames;
        int m_NumCamFramesDiscarded;
        
        OGLImagingContext* m_pImagingContext;
        FilterPtr m_pBandpassFilter;
};

}

#endif

