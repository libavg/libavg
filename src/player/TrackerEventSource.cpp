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

#include "../base/Logger.h"

#include "../graphics/Rect.h"
#include "../graphics/HistoryPreProcessor.h"
#include "../graphics/Filterfill.h"
#include "../graphics/Pixel8.h"

#include "../imaging/DeDistort.h"
#include "../imaging/ConnectedComps.h"
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

using namespace std;

const int MAXMISSINGFRAMES=1;

namespace avg {

    class  EventStream
    //internal class to keep track of blob/event states
    {
        public:
            EventStream(BlobPtr first_blob);
            void update(BlobPtr new_blob);
            Event* pollevent(DPoint& Offset, DPoint& Scale);
            enum StreamState {
                DOWN_PENDING, //fresh stream. not polled yet
                DOWN_DELIVERED, //initial finger down delivered
                MOTION_PENDING, //recent position change
                MOTION_DELIVERED, //finger resting
                VANISHED, // oops, no followup found -- wait a little while
                UP_PENDING, //finger disappeared, but fingerup yet to be delivered
                UP_DELIVERED // waiting to be cleared.
            };
            int m_Id;
            StreamState m_State;
            bool m_Stale;
        private:
            int m_VanishCounter;
            DPoint m_Pos;
            BlobPtr m_pBlob;
            static int s_LastLabel;
    };
    
    int EventStream::s_LastLabel = 0;

    EventStream::EventStream(BlobPtr first_blob)
    {
        m_Id = ++s_LastLabel;
        m_pBlob = first_blob;
        m_Pos = m_pBlob->center();
        m_State = DOWN_PENDING;
        m_Stale = false;
    };

    void EventStream::update(BlobPtr new_blob)
    {
        if (!new_blob){
            switch(m_State) {
                case DOWN_PENDING:
                    AVG_TRACE(Logger::EVENTS2, "Spurious blob suppressed.");
                    m_State = UP_DELIVERED;
                    break;
                case UP_PENDING:
                case UP_DELIVERED:
                    break;
                default:
                    m_State = VANISHED;
                    m_VanishCounter++;
                    if(m_VanishCounter>=MAXMISSINGFRAMES){
                        m_State = UP_PENDING;
                    }
                    break;
            }

            return;
        }
        assert(m_pBlob);
        m_VanishCounter = 0;
        DPoint c = new_blob->center();
        //Fixme replace m_Pos == c with something that takes resolution into account
        bool pos_unchanged = (c == m_Pos);
        switch(m_State) {
            case DOWN_PENDING:
                //finger touch has not been polled yet. update position
                break;
            case VANISHED:
                m_State = MOTION_PENDING;
                break;
            case DOWN_DELIVERED:
                //fingerdown delivered, change to motion states
                if (pos_unchanged)
                    m_State = MOTION_DELIVERED;
                else
                    m_State = MOTION_PENDING;
                break;
            case MOTION_PENDING:
                break;
            case MOTION_DELIVERED:
                if (!pos_unchanged) {
                    m_State = MOTION_PENDING;
                }
                break;
            default:
                //pass
                break;
            };
        m_Pos = c;
        m_pBlob = new_blob;
        m_Stale = false;
    };

    //REFACTORME: replace offset/scale with CoordTransformer
    Event* EventStream::pollevent(DPoint& Offset, DPoint& Scale)
    {
        assert(m_pBlob);
        switch(m_State){
            case DOWN_PENDING:
                m_State = DOWN_DELIVERED;
                return new TouchEvent(m_Id, Event::TOUCHDOWN,
                        (m_pBlob->getInfo()), m_pBlob, Offset, Scale);

                break;
            case MOTION_PENDING:
                m_State = MOTION_DELIVERED;
                return new TouchEvent(m_Id, Event::TOUCHMOTION,
                        (m_pBlob->getInfo()), m_pBlob, Offset, Scale);
                break;
            case UP_PENDING:
                m_State = UP_DELIVERED;
                return new TouchEvent(m_Id, Event::TOUCHUP,
                        (m_pBlob->getInfo()), m_pBlob, Offset, Scale);
                break;
            case DOWN_DELIVERED:
            case MOTION_DELIVERED:
            case UP_DELIVERED:
            default:
                //return no event
                return 0;
                break;
        }
    };


    TrackerEventSource::TrackerEventSource(CameraPtr pCamera, 
            bool bSubtractHistory)
        : m_TrackerConfig(),
          m_pCalibrator(0)
    {
        AVG_TRACE(Logger::CONFIG,"TrackerEventSource created");
        IntPoint ImgDimensions = pCamera->getImgSize();

        m_pBitmaps[0] = BitmapPtr(new Bitmap(pCamera->getImgSize(), I8));
        m_TrackerConfig.load("TrackerConfig.xml");
        // TODO: Clean this ROI handling mess up.
        handleROIChange();
        m_pUpdateMutex = MutexPtr(new boost::mutex);
        m_pTrackerMutex = MutexPtr(new boost::mutex);
        m_pCmdQueue = TrackerThread::CmdQueuePtr(new TrackerThread::CmdQueue);
        m_pTrackerThread = new boost::thread(
                TrackerThread(pCamera,
                    m_pBitmaps, 
                    m_pTrackerMutex,
                    *m_pCmdQueue,
                    this,
                    bSubtractHistory,
                    m_TrackerConfig
                    )
                );
        setConfig();
        handleROIChange();
    }

    TrackerEventSource::~TrackerEventSource()
    {
        m_pCmdQueue->push(Command<TrackerThread>(boost::bind(&TrackerThread::stop, _1)));
        m_pTrackerThread->join();
        delete m_pTrackerThread;
    }
        
    void TrackerEventSource::setROILeft(int Left)
    {
        m_TrackerConfig.m_ROI.tl.x = Left;
        setConfig();
        handleROIChange();
    }

    int TrackerEventSource::getROILeft()
    {
        return m_TrackerConfig.m_ROI.tl.x;
    }

    void TrackerEventSource::setROITop(int Top)
    {
        m_TrackerConfig.m_ROI.tl.y = Top;
        setConfig();
        handleROIChange();
    }

    int TrackerEventSource::getROITop()
    {
        return m_TrackerConfig.m_ROI.tl.y;
    }

    void TrackerEventSource::setROIRight(int Right)
    {
        m_TrackerConfig.m_ROI.br.x = Right;
        setConfig();
        handleROIChange();
    }

    int TrackerEventSource::getROIRight()
    {
        return m_TrackerConfig.m_ROI.br.x;
    }

    void TrackerEventSource::setROIBottom(int Bottom)
    {
        m_TrackerConfig.m_ROI.br.y = Bottom;
        setConfig();
        handleROIChange();
    }

    int TrackerEventSource::getROIBottom()
    {
        return m_TrackerConfig.m_ROI.br.y;
    }

    void TrackerEventSource::setThreshold(int Threshold) 
    {
        m_TrackerConfig.m_Threshold = Threshold;
        setConfig();
    }

    int TrackerEventSource::getThreshold()
    {
        return m_TrackerConfig.m_Threshold;
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
        m_TrackerConfig.save("TrackerConfig.xml");
    }

    void TrackerEventSource::setConfig()
    {
        m_pCmdQueue->push(Command<TrackerThread>(boost::bind(
                &TrackerThread::setConfig, _1, m_TrackerConfig)));
    }

    void TrackerEventSource::handleROIChange()
    {
        IntPoint ImgDimensions(m_TrackerConfig.m_ROI.br - m_TrackerConfig.m_ROI.tl);
        for (int i=1; i<NUM_TRACKER_IMAGES-1; i++) {
            m_pBitmaps[i] = BitmapPtr(new Bitmap(ImgDimensions, I8));
        }
        m_pBitmaps[TRACKER_IMG_FINGERS] = BitmapPtr(new Bitmap(ImgDimensions, R8G8B8X8));
        if (m_pCmdQueue) {
            m_pCmdQueue->push(Command<TrackerThread>(boost::bind(
                    &TrackerThread::setBitmaps, _1, m_TrackerConfig.m_ROI, m_pBitmaps)));
        }
    }

    Bitmap * TrackerEventSource::getImage(TrackerImageID ImageID) const
    {
        boost::mutex::scoped_lock Lock(*m_pTrackerMutex);
        return new Bitmap(*m_pBitmaps[ImageID]);
    }
    

    double distance(BlobPtr p1, BlobPtr p2) 
    {
        DPoint c1 = p1->center();
        DPoint c2 = p2->center();

        return sqrt( (c1.x-c2.x)*(c1.x-c2.x) + (c1.y-c2.y)*(c1.y-c2.y));
    }

    BlobList::iterator TrackerEventSource::matchblob(BlobPtr old_blob, BlobListPtr new_blobs, double threshold)
    {
        //return an iterator object in new_blobs for the the matching new_blob or new_blobs->end()
        assert(old_blob);
        std::vector<BlobList::iterator> candidates;
        BlobList::iterator res;
        for(BlobList::iterator it=new_blobs->begin();it!=new_blobs->end();++it)
        {
            if (distance( (*it), old_blob)<threshold) 
                candidates.push_back( it );
        }
        switch (candidates.size()) {
            case 0:
                res = new_blobs->end();
                break;
            case 1:
                res = candidates[0];
                break;
            default:
                //FIXME duplicate calculation of distance should be eliminated...
                double act=1e10, tmp;
                for(std::vector<BlobList::iterator>::iterator it=candidates.begin();it!=candidates.end();++it){
                    if ((tmp = distance( *(*it), old_blob))<act){
                        res = (*it);
                        act = tmp;
                    }
                }
        }
        return res;
    }

    bool TrackerEventSource::isfinger(BlobPtr blob)
    {
        BlobInfoPtr info = blob->getInfo();
#define IN(x, pair) (((x)>=pair[0])&&((x)<=pair[1]))
        bool res;
        res = IN(info->m_Area, m_TrackerConfig.m_AreaBounds) && IN(info->m_Eccentricity, m_TrackerConfig.m_EccentricityBounds);
        return res;
#undef IN
    }

    void TrackerEventSource::update(BlobListPtr new_blobs, BitmapPtr pBitmap)
    {
        boost::mutex::scoped_lock Lock(*m_pUpdateMutex);
        BlobListPtr old_blobs = BlobListPtr(new BlobList());
        EventStreamPtr e;
        if (pBitmap) {
            Pixel32 Black(0x00, 0x00, 0x00, 0x00);
            FilterFill<Pixel32>(Black).applyInPlace(
                pBitmap);
        }
        for(EventMap::iterator it=m_Events.begin();it!=m_Events.end();++it){
            (*it).second->m_Stale = true;
            old_blobs->push_back((*it).first);
        }
        int known_counter=0, new_counter=0, ignored_counter=0; 
        // 2. For each old_blob choose the best match of an new blob and update its event stream accordingly
        // 3. update map m_Events accordingly.
        // 4. remove matched new_blob row from the array by zeroing.
        // 3. Everything still left is a new event
        for(BlobList::iterator it2 = old_blobs->begin();it2!=old_blobs->end();++it2){
            if (!isfinger(*it2)){
                if (pBitmap) {
                    (*it2)->render(&*pBitmap, 
                            Pixel32(0xFF, 0x00, 0x00, 0xFF), false);
                }
                ignored_counter++;
                continue;
            }else {
                if (pBitmap) {
                    (*it2)->render(&*pBitmap, 
                            Pixel32(0xFF, 0xFF, 0xFF, 0xFF), true, 
                            Pixel32(0x00, 0x00, 0xFF, 0xFF)); 
                }
            }
            BlobList::iterator new_match = matchblob((*it2), new_blobs, m_TrackerConfig.m_Similarity);
            if(new_match!=new_blobs->end()){
                //this blob has a valid successor
                known_counter++;
                e = m_Events.find(*it2)->second;
                e->update( (*new_match) );
                //update the mapping!
                m_Events[(*new_match)] = e;
                m_Events.erase(*it2);
                *new_match = BlobPtr();//mark as removed
            }  
        }

        int gone_counter = 0;
        for(EventMap::iterator it3=m_Events.begin();it3!=m_Events.end();++it3){
            //all event streams that are still stale haven't been updated: blob is gone, send the sentinel for this.
            if ((*it3).second->m_Stale) {
                (*it3).second->update( BlobPtr() );
                gone_counter++;
            }
        }
        for(BlobList::iterator it2 = new_blobs->begin();it2!=new_blobs->end();++it2){
            if(*it2) {
                m_Events[*it2] = EventStreamPtr(new EventStream(*it2));
                new_counter++;
            }
        }

        //       AVG_TRACE(Logger::EVENTS2, "matched blobs: "<<known_counter<<"; new blobs: "<<new_counter<<"; ignored: "<<ignored_counter);
        //       AVG_TRACE(Logger::EVENTS2, ""<<gone_counter<<" fingers disappeared.");
    };
        
    TrackerCalibrator* TrackerEventSource::startCalibration(int XDisplayExtents, 
            int YDisplayExtents)
    {
        assert(!m_pCalibrator);
        m_pOldTransformer = m_TrackerConfig.m_pTrafo; 
        m_OldROI = m_TrackerConfig.m_ROI;
        m_TrackerConfig.m_ROI = IntRect(IntPoint(0,0), m_pBitmaps[0]->getSize());
        m_TrackerConfig.m_pTrafo = DeDistortPtr(new DeDistort());
        setConfig();
        handleROIChange();
        m_pCalibrator = new TrackerCalibrator(m_pBitmaps[0]->getSize(),
                m_TrackerConfig.m_ROI,
                IntPoint(XDisplayExtents, YDisplayExtents));
        return m_pCalibrator;
    }

    void TrackerEventSource::endCalibration()
    {
        assert(m_pCalibrator);
        m_pCalibrator->makeTransformer(m_TrackerConfig.m_pTrafo, m_Offset, m_Scale); //OUT parameter!
        m_TrackerConfig.m_ROI = m_OldROI;
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
        m_TrackerConfig.m_ROI = m_OldROI;
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
        Event *t;
        int kill_counter = 0;
        for (EventMap::iterator it = m_Events.begin(); it!= m_Events.end();){
            t = (*it).second->pollevent(m_Offset, m_Scale);
            if (t) res.push_back(t);
            if ((*it).second->m_State == EventStream::UP_DELIVERED){
                m_Events.erase(it++);
                kill_counter++;
            }else{
                ++it;
            }
        }

//        if (kill_counter)
//            AVG_TRACE(Logger::EVENTS2, ""<<kill_counter<<" EventStreams removed.");
        return res;
    }
    
}




