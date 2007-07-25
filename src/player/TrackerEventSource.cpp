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

#include "../graphics/Rect.h"
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
        AVG_TRACE(Logger::CONFIG,"TrackerEventSource created");

        IntPoint ImgSize = pCamera->getImgSize();
        m_pBitmaps[0] = BitmapPtr(new Bitmap(ImgSize, I8));
        m_pUpdateMutex = MutexPtr(new boost::mutex);
        m_pTrackerMutex = MutexPtr(new boost::mutex);
        handleROIChange();
        m_pCmdQueue = TrackerThread::CmdQueuePtr(new TrackerThread::CmdQueue);
        m_pTrackerThread = new boost::thread(
                TrackerThread(
                    m_TrackerConfig.m_pTrafo->getActiveBlobArea(DPoint(m_DisplayExtents)),
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
        
    void TrackerEventSource::setThreshold(int Threshold) 
    {
        m_TrackerConfig.m_pTouch->m_Threshold = Threshold;
        setConfig();
    }

    int TrackerEventSource::getThreshold()
    {
        return m_TrackerConfig.m_pTouch->m_Threshold;
    }
    
    void TrackerEventSource::setHistorySpeed(int UpdateInterval)
    {
        m_TrackerConfig.m_HistoryUpdateInterval = UpdateInterval;
        setConfig();
    }
    
    int TrackerEventSource::getHistorySpeed()
    {
        return m_TrackerConfig.m_HistoryUpdateInterval;
    }

    void TrackerEventSource::setBrightness(int Brightness) 
    {
        m_TrackerConfig.m_Brightness = Brightness;
        setConfig();
    }

    int TrackerEventSource::getBrightness()
    {
        return m_TrackerConfig.m_Brightness;
    }

    void TrackerEventSource::setExposure(int Exposure) 
    {
        m_TrackerConfig.m_Exposure = Exposure;
        setConfig();
    }

    int TrackerEventSource::getExposure()
    {
        return m_TrackerConfig.m_Exposure;
    }

    void TrackerEventSource::setGamma(int Gamma) 
    {
        m_TrackerConfig.m_Gamma = Gamma;
        setConfig();
    }

    int TrackerEventSource::getGamma()
    {
        return m_TrackerConfig.m_Gamma;
    }

    void TrackerEventSource::setGain(int Gain) 
    {
        m_TrackerConfig.m_Gain = Gain;
        setConfig();
    }

    int TrackerEventSource::getGain()
    {
        return m_TrackerConfig.m_Gain;
    }

    void TrackerEventSource::setShutter(int Shutter) 
    {
        m_TrackerConfig.m_Shutter = Shutter;
        setConfig();
    }

    int TrackerEventSource::getShutter()
    {
        return m_TrackerConfig.m_Shutter;
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
        IntPoint ImgSize(int(Area.Width()), int(Area.Height()));
        for (int i=1; i<NUM_TRACKER_IMAGES-1; i++) {
            m_pBitmaps[i] = BitmapPtr(new Bitmap(ImgSize, I8));
        }
        m_pBitmaps[TRACKER_IMG_FINGERS] = BitmapPtr(new Bitmap(ImgSize, R8G8B8A8));
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
    

    double distance(BlobPtr p1, BlobPtr p2) 
    {
        DPoint c1 = p1->getInfo()->getCenter();
        DPoint c2 = p2->getInfo()->getCenter();

        return sqrt( (c1.x-c2.x)*(c1.x-c2.x) + (c1.y-c2.y)*(c1.y-c2.y));
    }

    BlobPtr TrackerEventSource::matchblob(BlobPtr new_blob, BlobArrayPtr old_blobs, 
            double threshold, EventMap * pEvents)
    {
        assert(new_blob);
        std::vector<BlobPtr> candidates;
        BlobPtr res;
        for(BlobArray::iterator it=old_blobs->begin();it!=old_blobs->end();++it)
        {
            if (distance( (*it), new_blob)<threshold &&
                pEvents->find(*it) != pEvents->end())
            {
                candidates.push_back( (*it) );
            }
        }
        switch (candidates.size()) {
            case 0:
                res = BlobPtr();
                break;
            case 1:
                res = candidates[0];
                break;
            default:
                double act=1e10, tmp;
                for(std::vector<BlobPtr>::iterator it=candidates.begin();
                        it!=candidates.end();++it)
                {
                    if ((tmp = distance( (*it), new_blob))<act){
                        res = (*it);
                        act = tmp;
                    }
                }
        }
        return res;
    }

    bool TrackerEventSource::isRelevant(BlobPtr blob, BlobConfigPtr pConfig)
    {
        BlobInfoPtr info = blob->getInfo();
        // FIXME!
#define IN(x, pair) (((x)>=pair[0])&&((x)<=pair[1]))
        bool res;
        res = IN(info->getArea(), pConfig->m_AreaBounds) && 
                IN(info->getEccentricity(), pConfig->m_EccentricityBounds);
        return res;
#undef IN
    }

    void TrackerEventSource::update(BlobArrayPtr pTrackBlobs, BitmapPtr pTrackBmp, 
            int TrackThreshold, BlobArrayPtr pTouchBlobs, BitmapPtr pTouchBmp, 
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

    void TrackerEventSource::calcBlobs(BlobArrayPtr new_blobs, bool bTouch)
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
        BlobArrayPtr old_blobs = BlobArrayPtr(new BlobArray());
        for(EventMap::iterator it=pEvents->begin();it!=pEvents->end();++it) {
            (*it).second->setStale();
            old_blobs->push_back((*it).first);
        }
        for(BlobArray::iterator it2 = new_blobs->begin();it2!=new_blobs->end();++it2) {
            if (isRelevant(*it2, pBlobConfig)) {
                BlobPtr old_match = matchblob((*it2), old_blobs, 
                        pBlobConfig->m_Similarity, pEvents);
                if(old_match) {
                    assert (pEvents->find(old_match) != pEvents->end());
                    //this blob has been identified with an old one
                    EventStreamPtr e;
                    e = pEvents->find(old_match)->second;
                    e->blobChanged((*it2)->getInfo());
                    //update the mapping!
                    (*pEvents)[(*it2)] = e;
                    pEvents->erase(old_match);
                } else {
                    //this is a new one
                    (*pEvents)[(*it2)] = EventStreamPtr( 
                            new EventStream(((*it2)->getInfo())));
                }
            }
        }
        int gone_counter = 0;
        for(EventMap::iterator it3=pEvents->begin();it3!=pEvents->end();++it3){
            //all event streams that are still stale haven't been updated: blob is gone, send the sentinel for this.
            if ((*it3).second->isStale()) {
                (*it3).second->blobGone();
                gone_counter++;
            }
        }
    };
        
   void TrackerEventSource::correlateBlobs()
   {
        for(EventMap::iterator it2=m_TrackEvents.begin(); it2!=m_TrackEvents.end(); ++it2) {
            BlobPtr pTrackBlob = it2->first;
            pTrackBlob->getInfo()->m_RelatedBlobs.clear();
        }
        for(EventMap::iterator it1=m_TouchEvents.begin(); it1!=m_TouchEvents.end(); ++it1) {
            BlobPtr pTouchBlob = it1->first;
            BlobInfoPtr pTouchInfo = pTouchBlob->getInfo();
            pTouchInfo->m_RelatedBlobs.clear();
            IntPoint TouchCenter = (IntPoint)(pTouchInfo->getCenter());
            for(EventMap::iterator it2=m_TrackEvents.begin(); it2!=m_TrackEvents.end(); ++it2) {
                BlobPtr pTrackBlob = it2->first;
                if (pTrackBlob->contains(TouchCenter)) {
                    pTouchInfo->m_RelatedBlobs.push_back( pTrackBlob->getInfo());
                    pTrackBlob->getInfo()->m_RelatedBlobs.push_back(pTouchInfo);
                    break;
                }
            }
        }
   }

    void TrackerEventSource::drawBlobs(BlobArrayPtr pBlobs, BitmapPtr pSrcBmp, 
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
        
        for(BlobArray::iterator it2 = pBlobs->begin();it2!=pBlobs->end();++it2) {
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

    std::vector<Event*> TrackerEventSource::pollEvents()
    {
        boost::mutex::scoped_lock Lock(*m_pUpdateMutex);
        std::vector<Event*> res = std::vector<Event *>();
        pollEventType(res, m_TouchEvents, CursorEvent::TOUCH);
        pollEventType(res, m_TrackEvents, CursorEvent::TRACK);
        return res;
    }
   
    void TrackerEventSource::pollEventType(std::vector<Event*>& res, EventMap& Events,
            CursorEvent::Source source) 
    {
        Event *t;
        int kill_counter = 0;
        for (EventMap::iterator it = Events.begin(); it!= Events.end();) {
            t = (*it).second->pollevent(m_TrackerConfig.m_pTrafo, m_DisplayExtents, source);
            if (t) {
                res.push_back(t);
            }
            if ((*it).second->isGone()) {
                Events.erase(it++);
                kill_counter++;
            } else {
                ++it;
            }
        }
    }

}




