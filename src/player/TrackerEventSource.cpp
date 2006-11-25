
#include "TrackerEventSource.h"

#include "../imaging/ConnectedComps.h"

#include <map>
#include <list>

namespace avg {
/*
    EventStream::EventStream(BlobPtr first_blob){
        //need to lock s_LastLabel???
        m_Id = s_LastLabel++;
        m_Blob = first_blob;
        m_Pos = m_Blob.center();
        m_State = FRESH;
        m_Stale = False;
    };
    void EventStream::update(BlobPtr new_blob){
        if (!new_blob){
            m_Blob = BlobPtr();
            m_State = FINGERUP;
            return;
        }
        DPoint c = new_blob.center();
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
                    {}
                else
                    m_State = INMOTION;
                break;
            else:
                //pass
            };
        m_Pos = c;
        m_Blob = new_blob;
        m_Stale = False;
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
            else:
                //return no event
                ;
        }
    };
*/

    TrackerEventSource::TrackerEventSource(std::string sDevice, 
            double FrameRate, std::string sMode)
    {
        
    }

    TrackerEventSource::~TrackerEventSource()
    {
    }

    // More parameters possible: Barrel/pincushion, history length,...
    void TrackerEventSource::setThreshold(int Threshold) 
    {
    }

    Bitmap * TrackerEventSource::getImage() const
    {
    }
    
/*
    bool TrackerEventSource::isfinger(BlobPtr blob)
    {
        return True;
    }
    BlobPtr TrackerEventSource::matchblob(BlobPtr new_blob, BlobListPtr old_blobs)
    {

    }
    void TrackerEventSource::update(BlobListPtr new_blobs){
       BlobListPtr old_blobs = BlobListPtr(new BlobList());
       EventStreamPtr e;
       for(EventMap::iterator it=m_Events.begin();it!=m_Events.end();it++){
           (*it)->second->m_Stale = True;
           old_blobs->push_back((*it)->first);
       }
       for(BlobListPtr::iterator it = new_blobs.begin();it!=new_blobs.end();it++){
            BlobPtr old_match = matchblob((*it), old_blobs);
            if(old_match){
                //this blob has been identified with an old one
                e = m_Events.find(old_match)->second;
                e->update( (*it) );
                //update the mapping!
                m_Events.erase(old_match);
                m_Events[(*it)] = e;
            } else {
                m_Events[(*it)] = EventStreamPtr( new EventStream((*it)) ) ;
            }
       }
       for(EventMap::iterator it=m_Events.begin();it!=m_Events.end();it++){
           if ((*it)->second->m_Stale) {
               (*it)->second->update( BlobPtr() );
           }

    }
    EventListPtr TrackerEventSource::pollevents(){
        EventListPtr res = EventListPtr(new EventList);
        EventPtr t;
        for (EventMap::iterator it = m_Events.begin(); it!= m_Events.end();it++){
            t = (*it)->second;
            if (t) res->push_back(t);
            if (t->m_State == DONE) m_Events.erase(it);
        }
        return res;
    }
*/
    
}
