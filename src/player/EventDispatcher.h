//
// $Id$
//

#ifndef _EventDispatcher_h_
#define _EventDispatcher_h_

#include "IEventSink.h"
#include "IEventSource.h"

#include <vector>
#include <queue>
#include <string>

namespace avg {

class Event;

class EventDispatcher {
    public:
        EventDispatcher();
        virtual ~EventDispatcher();
        void dispatch();
        void addSource(IEventSource * pSource);
        void addSink(IEventSink * pSink);

        void addEvent(Event* pEvent);

    private:
        std::vector<IEventSource*> m_EventSources;
        std::vector<IEventSink*> m_EventSinks;

        std::priority_queue<Event *,std::vector<Event *>,isEventAfter> 
            m_Events;
};

}

#endif

