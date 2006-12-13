
#include "TrackerEventSource.h"
#include "MouseEvent.h"

#include "../base/Logger.h"

#include "../imaging/ConnectedComps.h"
#if defined(AVG_ENABLE_1394) || defined(AVG_ENABLE_1394_2)
#include "../imaging/CameraUtils.h"
#endif
#include "../imaging/Camera.h"
#include "../imaging/CoordTransformer.h"

#include "../graphics/Rect.h"
#include "../graphics/HistoryPreProcessor.h"
#include "../graphics/Filterfill.h"
#include "../graphics/Pixel8.h"
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

#include <map>
#include <list>
#include <vector>
#include <iostream>

using namespace std;

namespace avg {
    //hmmm. just don't use two TrackerEventSources...
    //if you do, put these into thread local storage or something
    static double s_XOffset;
    static double s_YOffset;
    static double s_XScale;
    static double s_YScale;
    
    static CoordTransformerPtr s_Trafo; //

    class  EventStream
    //internal class to keep track of blob/event states
    {
        public:
            EventStream(BlobPtr first_blob);
            void update(BlobPtr new_blob);
            Event* pollevent();
            enum StreamState {
                FRESH, //fresh stream. not polled yet
                TOUCH_DELIVERED, //initial finger down delivered
                INMOTION, //recent position change
                RESTING, //finger resting
                FINGERUP, //finger disappeared, but fingerup yet to be delivered
                DONE // waiting to be cleared.
            };
            int m_Id;
            StreamState m_State;
            DPoint m_Pos;
            BlobPtr m_pBlob;
            static int s_LastLabel;
            bool m_Stale;
    };
    
    int EventStream::s_LastLabel = 0;

    EventStream::EventStream(BlobPtr first_blob){
        //need to lock s_LastLabel???
        m_Id = s_LastLabel++;
        m_pBlob = first_blob;
        m_Pos = m_pBlob->center();
        m_State = FRESH;
        m_Stale = false;
    };
    void EventStream::update(BlobPtr new_blob){
        if ((!new_blob)||(!m_pBlob)){
            m_pBlob = BlobPtr();
            switch(m_State) {
                case FRESH:
                    AVG_TRACE(Logger::EVENTS2, "Spurious blob suppressed.");
                    m_State = DONE;
                    break;
                default:
                    m_State = FINGERUP;
            }
            return;
        }
        DPoint c = new_blob->center();
        //Fixme replace m_Pos == c with something that takes resolution into account
        bool pos_unchanged = (c == m_Pos);
        switch(m_State) {
            case FRESH:
                //finger touch has not been polled yet. update position
                break;
            case TOUCH_DELIVERED:
                //fingerdown delivered, change to motion states
                if (pos_unchanged)
                    m_State = RESTING;
                else
                    m_State = INMOTION;
                break;
            case INMOTION:
                if (pos_unchanged) m_State = RESTING;
                break;
            case RESTING:
                if (pos_unchanged)
                    //pass
                    {;}
                else {
                    m_State = INMOTION;
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
    Event* EventStream::pollevent(){
        switch(m_State){
            m_Pos = s_Trafo->transform_point(m_Pos);
            case FRESH:
                m_State = TOUCH_DELIVERED;
                //return fingerdown
                return new MouseEvent(MouseEvent::MOUSEBUTTONDOWN, true, false, false, 
                        (int)(s_XScale*m_Pos.x+s_XOffset), 
                        (int)(s_YScale*m_Pos.y+s_YOffset), 
                        MouseEvent::LEFT_BUTTON); 
                break;
            case INMOTION:
                m_State = RESTING;
                //return motion
                return new MouseEvent(MouseEvent::MOUSEMOTION, true, false, false,
                        (int)(s_XScale*m_Pos.x+s_XOffset), 
                        (int)(s_YScale*m_Pos.y+s_YOffset), 
                        MouseEvent::LEFT_BUTTON); 
                break;
            case FINGERUP:
                m_State = DONE;
                return new MouseEvent(MouseEvent::MOUSEBUTTONUP, false, false, false,
                        (int)(s_XScale*m_Pos.x+s_XOffset), 
                        (int)(s_YScale*m_Pos.y+s_YOffset), 
                        MouseEvent::LEFT_BUTTON); 
                //return fingerup
                break;
            case TOUCH_DELIVERED:
            case RESTING:
            case DONE:
            default:
                //return no event
                return 0;
                break;
        }
    };


    TrackerEventSource::TrackerEventSource(CameraPtr pCamera, DRect TargetRect, bool bSubtractHistory)
        : m_TrackerConfig()
    {
        AVG_TRACE(Logger::CONFIG,"TrackerEventSource created");
        IntPoint ImgDimensions = pCamera->getImgSize();

        s_XOffset = TargetRect.tl.x;
        s_XOffset = TargetRect.tl.y;
        s_XScale = TargetRect.Width()/ImgDimensions.x;
        s_YScale = TargetRect.Height()/ImgDimensions.y;
        s_Trafo = CoordTransformerPtr( new CoordTransformer( IntRect(0,0,ImgDimensions.x,ImgDimensions.y), 0,0 ));
        for (int i=0; i<NUM_TRACKER_IMAGES-1; i++) {
            m_pBitmaps[i] = BitmapPtr(new Bitmap(ImgDimensions, I8));
        }
        m_pBitmaps[TRACKER_IMG_FINGERS] = BitmapPtr(new Bitmap(ImgDimensions, R8G8B8X8));
        m_pUpdateMutex = MutexPtr(new boost::mutex);
        m_pTrackerMutex = MutexPtr(new boost::mutex);
        m_pCmdQueue = TrackerThread::CmdQueuePtr(new TrackerThread::CmdQueue);
        m_pTrackerThread = new boost::thread(
                TrackerThread(pCamera,
                    m_pBitmaps, 
                    m_pTrackerMutex,
                    *m_pCmdQueue,
                    this,
                    bSubtractHistory
                    )
                );
        setConfig();
    }

    TrackerEventSource::~TrackerEventSource()
    {
        m_pCmdQueue->push(Command<TrackerThread>(boost::bind(&TrackerThread::stop, _1)));
        m_pTrackerThread->join();
        delete m_pTrackerThread;
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

    Bitmap * TrackerEventSource::getImage(TrackerImageID ImageID) const
    {
        boost::mutex::scoped_lock Lock(*m_pTrackerMutex);
        return new Bitmap(*m_pBitmaps[ImageID]);
    }
    

double distance(BlobPtr p1, BlobPtr p2) {
    DPoint c1 = p1->center();
    DPoint c2 = p2->center();

    return sqrt( (c1.x-c2.x)*(c1.x-c2.x) + (c1.y-c2.y)*(c1.y-c2.y));
}

    BlobPtr TrackerEventSource::matchblob(BlobPtr new_blob, BlobListPtr old_blobs, double threshold)
    {
        std::vector<BlobPtr> candidates;
        BlobPtr res;
        if (!new_blob) return BlobPtr();
        for(BlobList::iterator it=old_blobs->begin();it!=old_blobs->end();++it)
        {
            if (distance( (*it), new_blob)<threshold) 
                candidates.push_back( (*it) );
        }
        switch (candidates.size()) {
            case 0:
                res = BlobPtr();
                break;
            case 1:
                res = candidates[0];
                break;
            default:
                //FIXME duplicate calculation of distance should be eliminated...
                double act=1e10, tmp;
                for(std::vector<BlobPtr>::iterator it=candidates.begin();it!=candidates.end();++it){
                    if ((tmp = distance( (*it), new_blob))<act){
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

    void TrackerEventSource::update(BlobListPtr new_blobs){
        boost::mutex::scoped_lock Lock(*m_pUpdateMutex);
        BlobListPtr old_blobs = BlobListPtr(new BlobList());
        EventStreamPtr e;
        FilterFill<Pixel32>(Pixel32(0x00, 0x00, 0x00, 0xFF)).applyInPlace(
                m_pBitmaps[TRACKER_IMG_FINGERS]);
        for(EventMap::iterator it=m_Events.begin();it!=m_Events.end();++it){
            (*it).second->m_Stale = true;
            old_blobs->push_back((*it).first);
        }
        int known_counter=0, new_counter=0, ignored_counter=0; 
        for(BlobList::iterator it2 = new_blobs->begin();it2!=new_blobs->end();++it2){
            if (!isfinger(*it2)){
                (*it2)->render(&*m_pBitmaps[TRACKER_IMG_FINGERS], 
                        Pixel32(0x80, 0x80, 0x80, 0xFF), false); 
                ignored_counter++;
                continue;
            }else {
                (*it2)->render(&*m_pBitmaps[TRACKER_IMG_FINGERS], 
                        Pixel32(0xFF, 0xFF, 0xFF, 0xFF), true, 
                        Pixel32(0x00, 0x00, 0xFF, 0xFF)); 
            }
            BlobPtr old_match = matchblob((*it2), old_blobs, m_TrackerConfig.m_Similarity);
            if(old_match){
                //this blob has been identified with an old one
                known_counter++;
                if (m_Events.find(old_match) == m_Events.end()) {
                    //..but the blob already disappeared from the map
                    //=>EventStream already updated
                    continue;
                }
                e = m_Events.find(old_match)->second;
                e->update( (*it2) );
                //update the mapping!
                m_Events[(*it2)] = e;
                m_Events.erase(old_match);
            } else {
                new_counter++;
                //this is a new one
                m_Events[(*it2)] = EventStreamPtr( new EventStream((*it2)) ) ;
            }
        }
        //       AVG_TRACE(Logger::EVENTS2, "matched blobs: "<<known_counter<<"; new blobs: "<<new_counter<<"; ignored: "<<ignored_counter);
        int gone_counter = 0;
        for(EventMap::iterator it3=m_Events.begin();it3!=m_Events.end();++it3){
            //all event streams that are still stale haven't been updated: blob is gone, send the sentinel for this.
            if ((*it3).second->m_Stale) {
                (*it3).second->update( BlobPtr() );
                gone_counter++;
            }
        }

        //       AVG_TRACE(Logger::EVENTS2, ""<<gone_counter<<" fingers disappeared.");
    };
   std::vector<Event*> TrackerEventSource::pollEvents(){
        boost::mutex::scoped_lock Lock(*m_pUpdateMutex);
        std::vector<Event*> res = std::vector<Event *>();
        Event *t;
        int kill_counter = 0;
        for (EventMap::iterator it = m_Events.begin(); it!= m_Events.end();){
            t = (*it).second->pollevent();
            if (t) res.push_back(t);
            if ((*it).second->m_State == EventStream::DONE){
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


