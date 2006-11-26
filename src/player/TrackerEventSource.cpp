
#include "TrackerEventSource.h"

#include "../imaging/ConnectedComps.h"

#include <map>
#include <list>
#include <vector>
#include <iostream>

using namespace std;

namespace avg {
    class  EventStream
    {
        public:
            EventStream(BlobPtr first_blob);
            void update(BlobPtr new_blob);
            EventPtr pollevent();
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

    EventStream::EventStream(BlobPtr first_blob){
        //need to lock s_LastLabel???
        m_Id = s_LastLabel++;
        m_pBlob = first_blob;
        m_Pos = m_pBlob->center();
        m_State = FRESH;
        m_Stale = false;
    };
    void EventStream::update(BlobPtr new_blob){
        if (!new_blob){
            m_pBlob = BlobPtr();
            m_State = FINGERUP;
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
    EventPtr EventStream::pollevent(){
        switch(m_State){
            case FRESH:
                m_State = TOUCH_DELIVERED;
                //return fingerdown
                break;
            case INMOTION:
                m_State = RESTING;
                //return motion
                break;
            case FINGERUP:
                m_State = DONE;
                //return fingerup
                break;
            default:
                //return no event
                break;
        }
    };

    TrackerEventSource::TrackerEventSource(std::string sDevice, 
            double FrameRate, std::string sMode)
        : m_Tracker(sDevice, FrameRate, sMode)
    {
        
    }

    TrackerEventSource::~TrackerEventSource()
    {
    }

    // More parameters possible: Barrel/pincushion, history length,...
    void TrackerEventSource::setThreshold(int Threshold) 
    {
    }

    Bitmap * TrackerEventSource::getImage(TrackerImageID ImageID) const
    {
        return m_Tracker.getImage(ImageID);
    }
    

    bool TrackerEventSource::isfinger(BlobPtr blob)
    {
        BlobInfoPtr info = blob->getInfo();
#define IN(x, pair) (((x)>=pair.first)&&((x)<=pair.second))
        bool res;
        res = IN(info->m_Area, m_BlobSelector.m_AreaBounds) && IN(info->m_Eccentricity, m_BlobSelector.m_EccentricityBounds);
        return res;
#undef IN
    }
    double distance(BlobPtr p1, BlobPtr p2) {
        DPoint c1 = p1->center();
        DPoint c2 = p2->center();

        return sqrt( (c1.x-c2.x)*(c1.x-c2.x) + (c1.y-c2.y)*(c1.y-c2.y));
    }
    BlobPtr TrackerEventSource::matchblob(BlobPtr new_blob, BlobListPtr old_blobs, double threshold = 10.0)
    {
        std::vector<BlobPtr> candidates;
        for(BlobList::iterator it=old_blobs->begin();it!=old_blobs->end();it++)
        {
            if (distance( (*it), new_blob)<threshold) 
                candidates.push_back( (*it) );
        }
        switch (candidates.size()) {
            case 0:
                return BlobPtr();
                break;
            case 1:
                return candidates[0];
                break;
            default:
                //FIXME!!! sort by distance!
                return candidates[0];
        }

    }

    void TrackerEventSource::update(BlobListPtr new_blobs){
       BlobListPtr old_blobs = BlobListPtr(new BlobList());
       EventStreamPtr e;
       for(EventMap::iterator it=m_Events.begin();it!=m_Events.end();it++){
           (*it).second->m_Stale = true;
           old_blobs->push_back((*it).first);
       }
       for(BlobList::iterator it2 = new_blobs->begin();it2!=new_blobs->end();it2++){
            BlobPtr old_match = matchblob((*it2), old_blobs);
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
        std::vector<Event*> res = std::vector<Event *>();
        Event *t;
        for (EventMap::iterator it = m_Events.begin(); it!= m_Events.end();it++){
            t = (*it).second->pollevent();
            if (t) res.push_back(t);
            if ((*it).second->m_State == EventStream::DONE) m_Events.erase(it);
        }
        return res;
    }
    
}
