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

#ifndef _TrackerInputDevice_H_
#define _TrackerInputDevice_H_

#include "../api.h"
#include "CursorEvent.h"
#include "IInputDevice.h"
#include "TrackerCalibrator.h"
#include "TrackerInputDeviceBase.h"

#include "../imaging/TrackerThread.h"
#include "../imaging/Blob.h"

#include "../graphics/Bitmap.h"
#include "../graphics/Filter.h"

#include "../base/Rect.h"

#include <string>
#include <map>
#include <list>
#include <vector>
#include <utility>

namespace avg {

class TrackerTouchStatus;
typedef boost::shared_ptr<TrackerTouchStatus> TrackerTouchStatusPtr;

class AVG_API TrackerInputDevice: public TrackerInputDeviceBase
{
    typedef std::map<BlobPtr, TrackerTouchStatusPtr> TouchStatusMap;

    public:
        TrackerInputDevice();
        virtual ~TrackerInputDevice();
        void start();

        void setParam(const std::string& sElement, const std::string& sValue);
        std::string getParam(const std::string& sElement);
                
        void resetHistory();
        void setDebugImages(bool bImg, bool bFinger);
        void saveConfig();
        Bitmap * getImage(TrackerImageID imageID) const;
        DPoint getDisplayROIPos() const;
        DPoint getDisplayROISize() const;

        std::vector<EventPtr> pollEvents(); //main thread

        // implement TrackerInputDeviceBase
        // Called from Tracker Thread!
        virtual void updateBlobs(BlobVectorPtr pBlobs,BlobType type, long long time);

        TrackerCalibrator* startCalibration();
        void endCalibration();
        void abortCalibration();

    private:
        void setConfig();
        void createBitmaps(const IntRect& area);

        boost::thread* m_pTrackerThread;

        // Used by main thread
        void pollEventType(std::vector<EventPtr>& res, TouchStatusMap& events,
                CursorEvent::Source source);
        void copyRelatedInfo(std::vector<EventPtr> pTouchEvents,
                std::vector<EventPtr> pTrackEvents);
        void findFingertips(std::vector<EventPtr>& pTouchEvents);

        IntRect m_InitialROI;
        CameraPtr m_pCamera;
        bool m_bSubtractHistory;
        DeDistortPtr m_pDeDistort;
        DeDistortPtr m_pOldTransformer;
        IntPoint m_ActiveDisplaySize;
        DRect m_DisplayROI;
        DRect m_OldDisplayROI;
        TrackerCalibrator * m_pCalibrator;
        bool m_bFindFingertips;

        // Used by tracker thread
        void trackBlobIDs(BlobVectorPtr new_blobs, long long time, bool bTouch);

        // Used by both threads
        TouchStatusMap m_TouchEvents;
        TouchStatusMap m_TrackEvents;
        TrackerConfig m_TrackerConfig;

        MutexPtr m_pMutex;
        BitmapPtr m_pBitmaps[NUM_TRACKER_IMAGES];

        TrackerThread::CQueuePtr m_pCmdQueue;
};

typedef boost::shared_ptr<TrackerInputDevice> TrackerInputDevicePtr;

}

#endif

