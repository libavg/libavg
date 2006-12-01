
#include "TrackerEventSource.h"

#include "../imaging/ConnectedComps.h"
#if defined(AVG_ENABLE_1394) || defined(AVG_ENABLE_1394_2)
#include "../imaging/CameraUtils.h"
#endif
#include "../imaging/Camera.h"
#include "MouseEvent.h"
#include <map>
#include <list>
#include <vector>
#include <iostream>

using namespace std;

namespace avg {
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
            //FIXME translate coords
            case FRESH:
                m_State = TOUCH_DELIVERED;
                //return fingerdown
                return new MouseEvent(MouseEvent::MOUSEBUTTONDOWN, true, false, false, m_Pos.x,m_Pos.y, MouseEvent::LEFT_BUTTON); 
                break;
            case INMOTION:
                m_State = RESTING;
                //return motion
                return new MouseEvent(MouseEvent::MOUSEMOTION, true, false, false, m_Pos.x,m_Pos.y, MouseEvent::NO_BUTTON); 
                break;
            case FINGERUP:
                m_State = DONE;
                return new MouseEvent(MouseEvent::MOUSEBUTTONUP, false, false, false, m_Pos.x,m_Pos.y, MouseEvent::LEFT_BUTTON); 
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

    TrackerEventSource::TrackerEventSource(CameraPtr pCamera)
        : m_TrackerConfig(128, 10, 6, 50, 1, 3)
    {
#if defined(AVG_ENABLE_1394) || defined(AVG_ENABLE_1394_2)
        IntPoint ImgDimensions = pCamera->getImgSize();
#else
        IntPoint ImgDimensions(640,480);
#endif
        for (int i=0; i<NUM_TRACKER_IMAGES; i++) {
            m_pBitmaps[i] = BitmapPtr(new Bitmap(ImgDimensions, I8));
        }
        m_pMutex = MutexPtr(new boost::mutex);
        m_pCmdQueue = TrackerCmdQueuePtr(new TrackerCmdQueue());
        m_pThread = new boost::thread(
                TrackerThread(pCamera,
                    m_TrackerConfig.m_Threshold,
                    m_pBitmaps, 
                    m_pMutex,
                    m_pCmdQueue,
                    this
                    )
                );

    }

    TrackerEventSource::~TrackerEventSource()
    {
        // TODO: Send stop to thread, join()
        delete m_pThread;
    }

    // More parameters possible: Barrel/pincushion, history length,...
    void TrackerEventSource::setConfig(TrackerConfig config) 
    {
        TrackerCmdPtr cmd = TrackerCmdPtr( new TrackerCmd(TrackerCmd::CONFIG) );
        //FIXME convert the numbers from logical to physical!!
        cmd->config = TrackerConfigPtr(new TrackerConfig(config));
        m_pCmdQueue->push(cmd);
    }

    Bitmap * TrackerEventSource::getImage(TrackerImageID ImageID) const
    {
        boost::mutex::scoped_lock Lock(*m_pMutex);
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
        for(BlobList::iterator it=old_blobs->begin();it!=old_blobs->end();it++)
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
                for(std::vector<BlobPtr>::iterator it=candidates.begin();it!=candidates.end();it++){
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
       boost::mutex::scoped_lock Lock(*m_pMutex);
       BlobListPtr old_blobs = BlobListPtr(new BlobList());
       EventStreamPtr e;
       for(EventMap::iterator it=m_Events.begin();it!=m_Events.end();it++){
           (*it).second->m_Stale = true;
           old_blobs->push_back((*it).first);
       }
       for(BlobList::iterator it2 = new_blobs->begin();it2!=new_blobs->end();it2++){
           if (!isfinger(*it2)){
               continue;
           }
            BlobPtr old_match = matchblob((*it2), old_blobs, m_TrackerConfig.m_Similarity);
            if(old_match){
                //this blob has been identified with an old one
                e = m_Events.find(old_match)->second;
                e->update( (*it2) );
                //update the mapping!
                m_Events.erase(old_match);
                m_Events[(*it2)] = e;
            } else {
                //this is a new one
                m_Events[(*it2)] = EventStreamPtr( new EventStream((*it2)) ) ;
            }
       }
       for(EventMap::iterator it3=m_Events.begin();it3!=m_Events.end();it3++){
           //all event streams that are still stale haven't been updated: blob is gone, send the sentinel for this.
           if ((*it3).second->m_Stale) {
               (*it3).second->update( BlobPtr() );
           }

        }
    };
   std::vector<Event*> TrackerEventSource::pollEvents(){
        boost::mutex::scoped_lock Lock(*m_pMutex);
        std::vector<Event*> res = std::vector<Event *>();
        Event *t;
        for (EventMap::iterator it = m_Events.begin(); it!= m_Events.end();){
            t = (*it).second->pollevent();
            if (t) res.push_back(t);
            if ((*it).second->m_State == EventStream::DONE){
                EventMap::iterator tempit=it;
                it++;
                m_Events.erase(tempit);
            }else{
                it++;
            }

        }
        return res;
    }
    
}
