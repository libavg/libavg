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

#ifndef _TrackerEventSource_H_
#define _TrackerEventSource_H_

#include "Event.h"
#include "IEventSource.h"
#include "../graphics/Rect.h"
#include "../graphics/Bitmap.h"
#include "../graphics/Filter.h"
#include "../imaging/Camera.h"
#include "../imaging/TrackerThread.h"
#include "../imaging/ConnectedComps.h"

#include <string>
#include <map>
#include <list>
#include <vector>
#include <utility>
namespace avg {

class EventStream;
typedef boost::shared_ptr<EventStream> EventStreamPtr;
typedef std::map<BlobPtr, EventStreamPtr> EventMap;

class TrackerEventSource: public IBlobTarget, public IEventSource
{
    public:
        TrackerEventSource(CameraPtr pCamera, 
                bool bSubtractHistory = true);
        virtual ~TrackerEventSource();

        void setBarrel(double Barrel);
        double getBarrel();
        void setTrapezoid(double Trapezoid);
        double getTrapezoid();
        void setROILeft(int Left);
        int getROILeft();
        void setROITop(int Top);
        int getROITop();
        void setROIRight(int Right);
        int getROIRight();
        void setROIBottom(int Bottom);
        int getROIBottom();

        void setThreshold(int Threshold);
        int getThreshold();
        void setHistorySpeed(int UpdateInterval);
        int getHistorySpeed();
        void setBrightness(int Brightness);
        int getBrightness();
        void setExposure(int Exposure);
        int getExposure();
        void setGamma(int Gamma);
        int getGamma();
        void setGain(int Gain);
        int getGain();
        void setShutter(int Shutter);
        int getShutter();
        void setDebug(bool bDebug);
        bool getDebug();
        void resetHistory();

        void saveConfig();

        Bitmap * getImage(TrackerImageID ImageID) const;
        std::vector<Event *> pollEvents();//main thread

        /*implement IBlobTarget*/
        virtual void update(BlobListPtr new_blobs, BitmapPtr pBitmap);//tracker thread

    private:
        bool isfinger(BlobPtr blob);
        BlobPtr matchblob(BlobPtr new_blob, BlobListPtr old_blobs, double threshold);
        void setConfig();
        void handleROIChange();

        TrackerConfig m_TrackerConfig;
        EventMap m_Events;
        MutexPtr m_pTrackerMutex;
        MutexPtr m_pUpdateMutex;

        boost::thread* m_pTrackerThread;

        TrackerThread::CmdQueuePtr m_pCmdQueue;
        BlobListPtr m_pBlobList;
        BitmapPtr m_pBitmaps[NUM_TRACKER_IMAGES];
        DPoint m_Offset;
        DPoint m_Scale;
        
        CoordTransformerPtr m_Trafo; //
};

typedef boost::shared_ptr<TrackerEventSource> TrackerEventSourcePtr;

}

#endif

