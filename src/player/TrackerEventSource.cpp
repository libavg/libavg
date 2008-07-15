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
//  Original author of this file is igor@c-base.org
//

#include "TrackerEventSource.h"
#include "TouchEvent.h"
#include "EventStream.h"

#include "../base/Logger.h"
#include "../base/ObjectCounter.h"
#include "../base/ScopeTimer.h"

#include "../graphics/HistoryPreProcessor.h"
#include "../graphics/Filterfill.h"
#include "../graphics/Filterflip.h"
#include "../graphics/Pixel8.h"

#include "../imaging/DeDistort.h"
#include "../imaging/CoordTransformer.h"
#if defined(AVG_ENABLE_1394) || defined(AVG_ENABLE_1394_2)
#include "../imaging/CameraUtils.h"
#endif

#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

#include <map>
#include <list>
#include <vector>
#include <queue>
#include <set>
#include <iostream>
#include <assert.h>

using namespace std;

namespace avg {

    TrackerEventSource::TrackerEventSource(CameraPtr pCamera, 
            const TrackerConfig& Config, const IntPoint& DisplayExtents,
            bool bSubtractHistory)
        : m_pTrackerThread(0),
          m_pCamera(pCamera),
          m_bSubtractHistory(bSubtractHistory),
          m_DisplayExtents(DisplayExtents),
          m_pCalibrator(0),
          m_TrackerConfig(Config)
    {
        ObjectCounter::get()->incRef(&typeid(*this));

        IntPoint ImgSize = pCamera->getImgSize();
        m_pBitmaps[0] = BitmapPtr(new Bitmap(ImgSize, I8));
        m_pMutex = MutexPtr(new boost::mutex);
        m_pCmdQueue = TrackerThread::CmdQueuePtr(new TrackerThread::CmdQueue);
        IntRect ROI = m_TrackerConfig.getTransform()->getActiveBlobArea(DPoint(m_DisplayExtents));
        if (ROI.tl.x < 0 || ROI.tl.y < 0 || ROI.br.x > ImgSize.x || ROI.br.y > ImgSize.y) {
            AVG_TRACE(Logger::ERROR, "Impossible tracker configuration: Region of interest is " 
                    << ROI << ", camera image size is " << ImgSize << ". Aborting.");
            exit(5);
        }
        m_InitialROI = ROI;
        createBitmaps(ROI);
        setDebugImages(false, false);
    }

    TrackerEventSource::~TrackerEventSource()
    {
        m_pCmdQueue->push(Command<TrackerThread>(boost::bind(&TrackerThread::stop, _1)));
        if (m_pTrackerThread) {
            m_pTrackerThread->join();
            delete m_pTrackerThread;
        }            
        ObjectCounter::get()->decRef(&typeid(*this));
    }
    
    void TrackerEventSource::start()
    {
        m_pCamera->open();
        m_pTrackerThread = new boost::thread(
                TrackerThread(
                    m_InitialROI,
                    m_pCamera,
                    m_pBitmaps, 
                    m_pMutex,
                    *m_pCmdQueue,
                    this,
                    m_bSubtractHistory,
                    m_TrackerConfig
                    )
                );
        setConfig();
    }

    void TrackerEventSource::setParam(const string& sElement, const string& sValue)
    {
        string sOldParamVal = m_TrackerConfig.getParam(sElement);
        m_TrackerConfig.setParam(sElement, sValue);

        // Test if active area is outside camera.
        DRect Area = m_TrackerConfig.getTransform()
                ->getActiveBlobArea(DPoint(m_DisplayExtents));
        DPoint Size = m_TrackerConfig.getPointParam("/camera/size/");
        int Prescale = m_TrackerConfig.getIntParam("/tracker/prescale/@value");
        if (Area.br.x > Size.x/Prescale || Area.br.y > Size.y/Prescale ||
            Area.tl.x < 0 || Area.tl.y < 0)
        {
            m_TrackerConfig.setParam(sElement, sOldParamVal);
        } else {
            setConfig();
        }
//        m_TrackerConfig.dump();
    }
    
    string TrackerEventSource::getParam(const string& sElement)
    {
        return m_TrackerConfig.getParam(sElement);
    }
       
    void TrackerEventSource::setDebugImages(bool bImg, bool bFinger)
    {
        m_pCmdQueue->push(Command<TrackerThread>(boost::bind(
                &TrackerThread::setDebugImages, _1, bImg, bFinger)));
    }

    void TrackerEventSource::resetHistory()
    {
        m_pCmdQueue->push(Command<TrackerThread>(boost::bind(
                &TrackerThread::resetHistory, _1)));
    }

    void TrackerEventSource::saveConfig(const string& sFilename)
    {
        m_TrackerConfig.save(sFilename);
    }

    void TrackerEventSource::setConfig()
    {
        DRect Area = m_TrackerConfig.getTransform()
                ->getActiveBlobArea(DPoint(m_DisplayExtents));
        createBitmaps(Area);
        m_pCmdQueue->push(Command<TrackerThread>(boost::bind(
                &TrackerThread::setConfig, _1, m_TrackerConfig, Area, m_pBitmaps)));
    }

    void TrackerEventSource::createBitmaps(const IntRect & Area)
    {
        boost::mutex::scoped_lock Lock(*m_pMutex);
        for (int i=1; i<NUM_TRACKER_IMAGES; i++) {
            switch (i) {
                case TRACKER_IMG_HISTOGRAM:
                    m_pBitmaps[TRACKER_IMG_HISTOGRAM] = 
                            BitmapPtr(new Bitmap(IntPoint(256, 256), I8));
                    FilterFill<Pixel8>(Pixel8(0)).
                            applyInPlace(m_pBitmaps[TRACKER_IMG_HISTOGRAM]);
                    break;
                case TRACKER_IMG_FINGERS:
                    m_pBitmaps[TRACKER_IMG_FINGERS] = 
                            BitmapPtr(new Bitmap(Area.size(), B8G8R8A8));
                    FilterFill<Pixel32>(Pixel32(0,0,0,0)).
                            applyInPlace(m_pBitmaps[TRACKER_IMG_FINGERS]);
                    break;
                default:
                    m_pBitmaps[i] = BitmapPtr(new Bitmap(Area.size(), I8));
                    FilterFill<Pixel8>(Pixel8(0)).applyInPlace(m_pBitmaps[i]);
            }
        }
    }

    Bitmap * TrackerEventSource::getImage(TrackerImageID ImageID) const
    {
        boost::mutex::scoped_lock Lock(*m_pMutex);
        return new Bitmap(*m_pBitmaps[ImageID]);
    }
    
    double distSquared(BlobPtr p1, BlobPtr p2) 
    {
        DPoint c1 = p1->getCenter();
        DPoint c2 = p2->getCenter();

        return (c1.x-c2.x)*(c1.x-c2.x) + (c1.y-c2.y)*(c1.y-c2.y);
    }

    double distance(BlobPtr p1, BlobPtr p2) 
    {
        DPoint c1 = p1->getCenter();
        DPoint c2 = p2->getCenter();

        return sqrt( (c1.x-c2.x)*(c1.x-c2.x) + (c1.y-c2.y)*(c1.y-c2.y));
    }

    static ProfilingZone ProfilingZoneCalcTrack("trackBlobIDs(track)");
    static ProfilingZone ProfilingZoneCalcTouch("trackBlobIDs(touch)");

    void TrackerEventSource::update(BlobVectorPtr pTrackBlobs, 
            BlobVectorPtr pTouchBlobs, long long time)
    {
        if (pTrackBlobs) {
            ScopeTimer Timer(ProfilingZoneCalcTrack);
            trackBlobIDs(pTrackBlobs, time, false);
        }
        if (pTouchBlobs) {
            ScopeTimer Timer(ProfilingZoneCalcTouch);
            trackBlobIDs(pTouchBlobs, time, true);
        }
    }

    // Temporary structure to be put into heap of blob distances. Used only in 
    // trackBlobIDs.
    struct BlobDistEntry {
        BlobDistEntry(double Dist, BlobPtr pNewBlob, BlobPtr pOldBlob) 
            : m_Dist(Dist),
              m_pNewBlob(pNewBlob),
              m_pOldBlob(pOldBlob)
        {
        }

        double m_Dist;
        BlobPtr m_pNewBlob;
        BlobPtr m_pOldBlob;
    };
    typedef boost::shared_ptr<struct BlobDistEntry> BlobDistEntryPtr;

    // The heap is sorted by least distance, so this operator does the
    // _opposite_ of what is expected!
    bool operator < (const BlobDistEntryPtr& e1, const BlobDistEntryPtr& e2) 
    {
        return e1->m_Dist > e2->m_Dist;
    }

    void TrackerEventSource::trackBlobIDs(BlobVectorPtr pNewBlobs, long long time, 
            bool bTouch)
    {
        EventMap * pEvents;
        string sConfigPath;
        if (bTouch) {
            sConfigPath = "/tracker/touch/";
            pEvents = &m_TouchEvents;
        } else {
            sConfigPath = "/tracker/track/";
            pEvents = &m_TrackEvents;
        }
        BlobVector OldBlobs;
        for(EventMap::iterator it=pEvents->begin(); it!=pEvents->end(); ++it) {
            (*it).second->setStale();
            OldBlobs.push_back((*it).first);
        }
        // Create a heap that contains all distances of old to new blobs < MaxDist
        double MaxDist = m_TrackerConfig.getDoubleParam(sConfigPath+"similarity/@value");
        double MaxDistSquared = MaxDist*MaxDist;
        priority_queue<BlobDistEntryPtr> DistHeap;
        for(BlobVector::iterator it = pNewBlobs->begin(); 
                it!=pNewBlobs->end(); ++it) 
        {
            BlobPtr pNewBlob = *it;
            for(BlobVector::iterator it2 = OldBlobs.begin(); it2!=OldBlobs.end(); ++it2) { 
                BlobPtr pOldBlob = *it2;
                if (distSquared(pNewBlob, pOldBlob) <= MaxDistSquared) {
                    BlobDistEntryPtr pEntry = BlobDistEntryPtr(
                            new BlobDistEntry(distance(pNewBlob, pOldBlob), 
                                    pNewBlob, pOldBlob));
                    DistHeap.push(pEntry);
                }
            }
        }
        // Match up the closest blobs.
        set<BlobPtr> MatchedNewBlobs;
        set<BlobPtr> MatchedOldBlobs;
        int NumMatchedBlobs = 0;
        bool bEventOnMove = m_TrackerConfig.getBoolParam("/tracker/eventonmove/@value");
        while(!DistHeap.empty()) {
            BlobDistEntryPtr pEntry = DistHeap.top();
            DistHeap.pop();
            if (MatchedNewBlobs.find(pEntry->m_pNewBlob) == MatchedNewBlobs.end() &&
                MatchedOldBlobs.find(pEntry->m_pOldBlob) == MatchedOldBlobs.end())
            {
                // Found a pair of matched blobs.
                NumMatchedBlobs++;
                BlobPtr pNewBlob = pEntry->m_pNewBlob; 
                BlobPtr pOldBlob = pEntry->m_pOldBlob; 
                MatchedNewBlobs.insert(pNewBlob);
                MatchedOldBlobs.insert(pOldBlob);
                assert (pEvents->find(pOldBlob) != pEvents->end());
                EventStreamPtr pStream;
                pStream = pEvents->find(pOldBlob)->second;
                pStream->blobChanged(pNewBlob, time, bEventOnMove);
                // Update the mapping.
                (*pEvents)[pNewBlob] = pStream;
                pEvents->erase(pOldBlob);
            }
        }
        // Blobs have been matched. Left-overs are new blobs.
        for(BlobVector::iterator it = pNewBlobs->begin(); 
                it!=pNewBlobs->end(); ++it) 
        {
            if (MatchedNewBlobs.find(*it) == MatchedNewBlobs.end()) {
                (*pEvents)[(*it)] = EventStreamPtr( 
                        new EventStream(*it, time));
            }
        }

        // All event streams that are still stale haven't been updated: blob is gone, 
        // set the sentinel for this.
        for(EventMap::iterator it=pEvents->begin(); it!=pEvents->end(); ++it) {
            if ((*it).second->isStale()) {
                (*it).second->blobGone();
            }
        }
    };

    TrackerCalibrator* TrackerEventSource::startCalibration()
    {
        assert(!m_pCalibrator);
        m_pOldTransformer = m_TrackerConfig.getTransform();
        m_TrackerConfig.setTransform(DeDistortPtr(new DeDistort(
                DPoint(m_pBitmaps[0]->getSize()), DPoint(m_DisplayExtents))));
        setConfig();
        m_pCalibrator = new TrackerCalibrator(m_pBitmaps[0]->getSize(),
                m_DisplayExtents);
        return m_pCalibrator;
    }

    void TrackerEventSource::endCalibration()
    {
        assert(m_pCalibrator);
        m_TrackerConfig.setTransform(m_pCalibrator->makeTransformer());
        DRect Area = m_TrackerConfig.getTransform()
                ->getActiveBlobArea(DPoint(m_DisplayExtents));
        if (Area.size().x*Area.size().y > 1024*1024*8) {
            AVG_TRACE(Logger::WARNING, "Ignoring calibration - resulting area would be " 
                    << Area);
            m_TrackerConfig.setTransform(m_pOldTransformer);
        }
        setConfig();
        delete m_pCalibrator;
        m_pCalibrator = 0;
        m_pOldTransformer = DeDistortPtr();
    }

    void TrackerEventSource::abortCalibration()
    {
        assert(m_pCalibrator);
        m_TrackerConfig.setTransform(m_pOldTransformer);
        setConfig();
        m_pOldTransformer = DeDistortPtr();
        delete m_pCalibrator;
        m_pCalibrator = 0;
    }

    vector<EventPtr> TrackerEventSource::pollEvents()
    {
        boost::mutex::scoped_lock Lock(*m_pMutex);
        vector<EventPtr> pTouchEvents;
        vector<EventPtr> pTrackEvents;
        pollEventType(pTouchEvents, m_TouchEvents, CursorEvent::TOUCH);
        pollEventType(pTrackEvents, m_TrackEvents, CursorEvent::TRACK);
        copyRelatedInfo(pTouchEvents, pTrackEvents);
        pTouchEvents.insert(pTouchEvents.end(), 
                pTrackEvents.begin(), pTrackEvents.end());
        return pTouchEvents;
    }
   
    void TrackerEventSource::pollEventType(vector<EventPtr>& res, EventMap& Events,
            CursorEvent::Source source) 
    {
        EventPtr pEvent;
        int kill_counter = 0;
        DeDistortPtr pDeDistort = m_TrackerConfig.getTransform();
        bool bEventOnMove = m_TrackerConfig.getBoolParam("/tracker/eventonmove/@value");
        for (EventMap::iterator it = Events.begin(); it!= Events.end();) {
            EventStreamPtr pStream = (*it).second;
            pEvent = pStream->pollevent(pDeDistort, m_DisplayExtents,
                    source, bEventOnMove);
            if (pEvent) {
                res.push_back(pEvent);
            }
            if ((*it).second->isGone()) {
                Events.erase(it++);
                kill_counter++;
            } else {
                ++it;
            }
        }
    }

    void TrackerEventSource::copyRelatedInfo(vector<EventPtr> pTouchEvents,
            vector<EventPtr> pTrackEvents)
    {
        // Copy related blobs to related events.
        // Yuck.
        vector<EventPtr>::iterator it;
        for (it=pTouchEvents.begin(); it != pTouchEvents.end(); ++it) {
            TouchEventPtr pTouchEvent = boost::dynamic_pointer_cast<TouchEvent>(*it);
            BlobPtr pTouchBlob = pTouchEvent->getBlob();
            BlobPtr pRelatedBlob = pTouchBlob->getFirstRelated();
            if (pRelatedBlob) {
                vector<EventPtr>::iterator it2;
                TouchEventPtr pTrackEvent;
                BlobPtr pTrackBlob;
                for (it2=pTrackEvents.begin(); 
                        pTrackBlob != pRelatedBlob && it2 != pTrackEvents.end(); 
                        ++it2) 
                {
                    pTrackEvent = boost::dynamic_pointer_cast<TouchEvent>(*it2);
                    pTrackBlob = pTrackEvent->getBlob();
                }
                if (it2 != pTrackEvents.end()) {
                    pTouchEvent->addRelatedEvent(pTrackEvent);
                    pTrackEvent->addRelatedEvent(pTouchEvent);
                }
            }
        }
    }

}




