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
        : m_DisplayExtents(DisplayExtents),
          m_pCalibrator(0),
          m_TrackerConfig(Config)
    {
        ObjectCounter::get()->incRef(&typeid(*this));

        IntPoint ImgSize = pCamera->getImgSize();
        m_pBitmaps[0] = BitmapPtr(new Bitmap(ImgSize, I8));
        m_pUpdateMutex = MutexPtr(new boost::mutex);
        m_pTrackerMutex = MutexPtr(new boost::mutex);
        handleROIChange();
        m_pCmdQueue = TrackerThread::CmdQueuePtr(new TrackerThread::CmdQueue);
        IntRect ROI = m_TrackerConfig.m_pTrafo->getActiveBlobArea(DPoint(m_DisplayExtents));
        if (ROI.tl.x < 0 || ROI.tl.y < 0 || ROI.br.x > ImgSize.x || ROI.br.y > ImgSize.y) {
            AVG_TRACE(Logger::ERROR, "Impossible tracker configuration: Region of interest is " 
                    << ROI << ", camera image size is " << ImgSize << ". Aborting.");
            exit(5);
        }
        pCamera->open();
        m_pTrackerThread = new boost::thread(
                TrackerThread(
                    ROI,
                    pCamera,
                    m_pBitmaps, 
                    m_pTrackerMutex,
                    *m_pCmdQueue,
                    this,
                    bSubtractHistory,
                    m_TrackerConfig
                    )
                );
        setConfig();
    }

    TrackerEventSource::~TrackerEventSource()
    {
        m_pCmdQueue->push(Command<TrackerThread>(boost::bind(&TrackerThread::stop, _1)));
        m_pTrackerThread->join();
        delete m_pTrackerThread;
        ObjectCounter::get()->decRef(&typeid(*this));
    }
    
    void TrackerEventSource::setParam(const std::string& sElement, const std::string& Value)
    {
        string sOldParamVal = m_TrackerConfig.getParam(BAD_CAST sElement.c_str());
        m_TrackerConfig.setParam(BAD_CAST sElement.c_str(), BAD_CAST Value.c_str());

        // Test if active area is outside camera.
        DRect Area = m_TrackerConfig.m_pTrafo->getActiveBlobArea(DPoint(m_DisplayExtents));
        if (Area.br.x > m_TrackerConfig.m_Size.x || 
            Area.br.y > m_TrackerConfig.m_Size.y ||
            Area.tl.x < 0 || Area.tl.y < 0)
        {
            m_TrackerConfig.setParam(BAD_CAST sElement.c_str(), BAD_CAST sOldParamVal.c_str());
        } else {
            setConfig();
            handleROIChange();
        }
//        m_TrackerConfig.dump();
    }
    
    string TrackerEventSource::getParam(const std::string& sElement)
    {
        return m_TrackerConfig.getParam(BAD_CAST sElement.c_str());
    }
       
    void TrackerEventSource::setDebugImages(bool bImg, bool bFinger)
    {
        m_TrackerConfig.m_bCreateDebugImages = bImg;
        m_TrackerConfig.m_bCreateFingerImage = bFinger;
        setConfig();
    }

    void TrackerEventSource::resetHistory()
    {
        m_pCmdQueue->push(Command<TrackerThread>(boost::bind(
                &TrackerThread::resetHistory, _1)));
    }

    void TrackerEventSource::saveConfig()
    {
        m_TrackerConfig.save();
    }

    void TrackerEventSource::setConfig()
    {
        m_pCmdQueue->push(Command<TrackerThread>(boost::bind(
                &TrackerThread::setConfig, _1, m_TrackerConfig)));
    }

    void TrackerEventSource::handleROIChange()
    {
        boost::mutex::scoped_lock Lock(*m_pTrackerMutex);
        DRect Area = m_TrackerConfig.m_pTrafo->getActiveBlobArea(DPoint(m_DisplayExtents));
        IntPoint ImgSize(Area.size());
        for (int i=1; i<NUM_TRACKER_IMAGES-1; i++) {
            m_pBitmaps[i] = BitmapPtr(new Bitmap(ImgSize, I8));
        }
        m_pBitmaps[TRACKER_IMG_FINGERS] = BitmapPtr(new Bitmap(ImgSize, B8G8R8A8));
        FilterFill<Pixel32>(Pixel32(0,0,0,0)).applyInPlace(m_pBitmaps[TRACKER_IMG_FINGERS]);
        if (m_pCmdQueue) {
            m_pCmdQueue->push(Command<TrackerThread>(boost::bind(
                    &TrackerThread::setBitmaps, _1, Area, m_pBitmaps)));
        }
    }

    Bitmap * TrackerEventSource::getImage(TrackerImageID ImageID) const
    {
        boost::mutex::scoped_lock Lock(*m_pTrackerMutex);
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

    inline bool isInbetween(double x, double pair[2])
    {
        return x>=pair[0] && x<=pair[1];
    }

    bool TrackerEventSource::isRelevant(BlobPtr pBlob, BlobConfigPtr pConfig)
    {
        bool res;
        res = isInbetween(pBlob->getArea(), pConfig->m_AreaBounds) && 
                isInbetween(pBlob->getEccentricity(), pConfig->m_EccentricityBounds);
        return res;
    }

    void TrackerEventSource::update(BlobVectorPtr pTrackBlobs, BitmapPtr pTrackBmp, 
            int TrackThreshold, BlobVectorPtr pTouchBlobs, BitmapPtr pTouchBmp, 
            int TouchThreshold, BitmapPtr pDestBmp)
    {
        boost::mutex::scoped_lock Lock(*m_pUpdateMutex);
        if (pTrackBlobs) {
            calcBlobs(pTrackBlobs, false);
        }
        if (pTouchBlobs) {
            calcBlobs(pTouchBlobs, true);
        }
        correlateBlobs();
        if (pDestBmp) {
            drawBlobs(pTrackBlobs, pTrackBmp, pDestBmp, TrackThreshold, false); 
            drawBlobs(pTouchBlobs, pTouchBmp, pDestBmp, TouchThreshold, true); 
        }
    }

    // Temporary structure to be put into heap of blob distances. Used only in 
    // calcBlobs.
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

    void TrackerEventSource::calcBlobs(BlobVectorPtr pNewBlobs, bool bTouch)
    {
        BlobConfigPtr pBlobConfig;
        EventMap * pEvents;
        if (bTouch) {
            pBlobConfig = m_TrackerConfig.m_pTouch;
            pEvents = &m_TouchEvents;
        } else {
            pBlobConfig = m_TrackerConfig.m_pTrack;
            pEvents = &m_TrackEvents;
        }
        BlobVector OldBlobs;
        for(EventMap::iterator it=pEvents->begin(); it!=pEvents->end(); ++it) {
            (*it).second->setStale();
            OldBlobs.push_back((*it).first);
        }
        BlobVector NewRelevantBlobs;
        for(BlobVector::iterator it = pNewBlobs->begin(); it!=pNewBlobs->end(); ++it) {
            if (isRelevant(*it, pBlobConfig)) {
                NewRelevantBlobs.push_back(*it);

                if (m_TrackerConfig.m_ContourVertexes)
                    (*it)->calcContour(m_TrackerConfig.m_ContourVertexes);
            }
            if (NewRelevantBlobs.size() > 50) {
                break;
            }
        }
        // Create a heap that contains all distances of old to new blobs < MaxDist
        double MaxDistSquared = pBlobConfig->m_Similarity*pBlobConfig->m_Similarity;
        priority_queue<BlobDistEntryPtr> DistHeap;
        for(BlobVector::iterator it = NewRelevantBlobs.begin(); 
                it!=NewRelevantBlobs.end(); ++it) 
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
//        cerr << "DistHeap: " << DistHeap.size() << endl;
        // Match up the closest blobs.
        set<BlobPtr> MatchedNewBlobs;
        set<BlobPtr> MatchedOldBlobs;
        int NumMatchedBlobs = 0;
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
                pStream->blobChanged(pNewBlob);
                // Update the mapping.
                (*pEvents)[pNewBlob] = pStream;
                pEvents->erase(pOldBlob);
            }
        }
//        cerr << "Matched: " << NumMatchedBlobs << endl;
        // Blobs have been matched. Left-overs are new blobs.
        for(BlobVector::iterator it = NewRelevantBlobs.begin(); 
                it!=NewRelevantBlobs.end(); ++it) 
        {
            if (MatchedNewBlobs.find(*it) == MatchedNewBlobs.end()) {
                (*pEvents)[(*it)] = EventStreamPtr( 
                        new EventStream(*it));
            }
        }

        // All event streams that are still stale haven't been updated: blob is gone, 
        // set the sentinel for this.
        for(EventMap::iterator it=pEvents->begin(); it!=pEvents->end(); ++it) {
            if ((*it).second->isStale()) {
                (*it).second->blobGone();
            }
        }
//        cerr << pEvents->size() << endl;
    };

   void TrackerEventSource::correlateBlobs()
   {
        for(EventMap::iterator it2=m_TrackEvents.begin(); it2!=m_TrackEvents.end(); ++it2) {
            BlobPtr pTrackBlob = it2->first;
            pTrackBlob->clearRelated();
        }
        for(EventMap::iterator it1=m_TouchEvents.begin(); it1!=m_TouchEvents.end(); ++it1) {
            BlobPtr pTouchBlob = it1->first;
            pTouchBlob->clearRelated();
            IntPoint TouchCenter = (IntPoint)(pTouchBlob->getCenter());
            for(EventMap::iterator it2=m_TrackEvents.begin(); it2!=m_TrackEvents.end(); ++it2) {
                BlobPtr pTrackBlob = it2->first;
                if (pTrackBlob->contains(TouchCenter)) {
                    pTouchBlob->addRelated(pTrackBlob);
                    pTrackBlob->addRelated(pTouchBlob);
                    break;
                }
            }
        }
   }

    void TrackerEventSource::drawBlobs(BlobVectorPtr pBlobs, BitmapPtr pSrcBmp, 
            BitmapPtr pDestBmp, int Offset, bool bTouch)
    {
        if (!pBlobs) {
            return;
        }
        BlobConfigPtr pBlobConfig;
        if (bTouch) {
            pBlobConfig = m_TrackerConfig.m_pTouch;
        } else {
            pBlobConfig = m_TrackerConfig.m_pTrack;
        }
        // Get max. pixel value in Bitmap
        int Max = 0;
        HistogramPtr pHist = pSrcBmp->getHistogram(2);
        int i;
        for(i=255; i>=0; i--) {
            if ((*pHist)[i] != 0) {
                Max = i;
                i = 0;
            }
        }
        
        for(BlobVector::iterator it2 = pBlobs->begin();it2!=pBlobs->end();++it2) {
            if (isRelevant(*it2, pBlobConfig)) {
                if (bTouch) {
                    (*it2)->render(pSrcBmp, pDestBmp, 
                            Pixel32(0xFF, 0xFF, 0xFF, 0xFF), Offset, Max, bTouch, true,  
                            Pixel32(0x00, 0x00, 0xFF, 0xFF));
                } else {
                    (*it2)->render(pSrcBmp, pDestBmp, 
                            Pixel32(0xFF, 0xFF, 0x00, 0x80), Offset, Max, bTouch, true, 
                            Pixel32(0x00, 0x00, 0xFF, 0xFF));
                }
            } else {
                if (bTouch) {
                    (*it2)->render(pSrcBmp, pDestBmp, 
                            Pixel32(0xFF, 0x00, 0x00, 0xFF), Offset, Max, bTouch, false);
                } else {
                    (*it2)->render(pSrcBmp, pDestBmp, 
                            Pixel32(0x80, 0x80, 0x00, 0x80), Offset, Max, bTouch, false);
                }
            }
        }
    }

    TrackerCalibrator* TrackerEventSource::startCalibration()
    {
        assert(!m_pCalibrator);
        m_pOldTransformer = m_TrackerConfig.m_pTrafo;
        m_TrackerConfig.m_pTrafo = DeDistortPtr(new DeDistort(
                DPoint(m_pBitmaps[0]->getSize()), DPoint(m_DisplayExtents)));
        setConfig();
        handleROIChange();
        m_pCalibrator = new TrackerCalibrator(m_pBitmaps[0]->getSize(),
                m_DisplayExtents);
        return m_pCalibrator;
    }

    void TrackerEventSource::endCalibration()
    {
        assert(m_pCalibrator);
        m_TrackerConfig.m_pTrafo = m_pCalibrator->makeTransformer();
        setConfig();
        handleROIChange();
        delete m_pCalibrator;
        m_pCalibrator = 0;
        m_pOldTransformer = DeDistortPtr();
    }

    void TrackerEventSource::abortCalibration()
    {
        assert(m_pCalibrator);
        m_TrackerConfig.m_pTrafo = m_pOldTransformer;
        setConfig();
        handleROIChange();
        m_pOldTransformer = DeDistortPtr();
        delete m_pCalibrator;
        m_pCalibrator = 0;
    }

    vector<Event*> TrackerEventSource::pollEvents()
    {
        boost::mutex::scoped_lock Lock(*m_pUpdateMutex);
        vector<Event*> pTouchEvents = std::vector<Event *>();
        vector<Event*> pTrackEvents = std::vector<Event *>();
        pollEventType(pTouchEvents, m_TouchEvents, CursorEvent::TOUCH);
        pollEventType(pTrackEvents, m_TrackEvents, CursorEvent::TRACK);
        copyRelatedInfo(pTouchEvents, pTrackEvents);
        pTouchEvents.insert(pTouchEvents.end(), 
                pTrackEvents.begin(), pTrackEvents.end());
        return pTouchEvents;
    }
   
    void TrackerEventSource::pollEventType(vector<Event*>& res, EventMap& Events,
            CursorEvent::Source source) 
    {
        Event *pEvent;
        int kill_counter = 0;
        for (EventMap::iterator it = Events.begin(); it!= Events.end();) {
            EventStreamPtr pStream = (*it).second;
            pEvent = pStream->pollevent(m_TrackerConfig.m_pTrafo, m_DisplayExtents, 
                    source);
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

    void TrackerEventSource::copyRelatedInfo(vector<Event*> pTouchEvents,
            vector<Event*> pTrackEvents)
    {
        // Copy related blobs to related events.
        // Yuck.
        vector<Event *>::iterator it;
        for (it=pTouchEvents.begin(); it != pTouchEvents.end(); ++it) {
            TouchEvent * pTouchEvent = dynamic_cast<TouchEvent *>(*it);
            BlobPtr pTouchBlob = pTouchEvent->getBlob();
            BlobPtr pRelatedBlob = pTouchBlob->getFirstRelated();
            if (pRelatedBlob) {
                vector<Event *>::iterator it2;
                TouchEvent * pTrackEvent = 0;
                BlobPtr pTrackBlob;
                for (it2=pTrackEvents.begin(); 
                        pTrackBlob != pRelatedBlob && it2 != pTrackEvents.end(); 
                        ++it2) 
                {
                    pTrackEvent = dynamic_cast<TouchEvent *>(*it2);
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




