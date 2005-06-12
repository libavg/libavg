//
// $Id$
//

#include "EventDispatcher.h"
#include "Event.h"
#include "../JSFactoryBase.h"

#include <string>

using namespace std;

namespace avg {

    EventDispatcher::EventDispatcher() {
    };

    EventDispatcher::~EventDispatcher() {
    };

    void
    EventDispatcher::dispatch() {
        for (unsigned int i = 0; i<m_EventSources.size(); ++i) {
            vector<Event*> curEvents = m_EventSources[i]->pollEvents();
            for (unsigned int i= 0; i<curEvents.size(); i++) {
                m_Events.push(curEvents[i]);
            }
        }

        while (!m_Events.empty()) {
            Event * curEvent = m_Events.top();
            m_Events.pop();
            for (unsigned int i = 0; i < m_EventSinks.size(); ++i) {
                if (m_EventSinks[i]->handleEvent(curEvent)) {
                    break;
                }
            }
        }
    };

    void
    EventDispatcher::addSource(IEventSource * pSource) {
        m_EventSources.push_back(pSource);
        pSource->initEventSource();
    }

    void
    EventDispatcher::addSink(IEventSink * pSink) {
        m_EventSinks.push_back(pSink);
    }

    void 
    EventDispatcher::addEvent(Event* pEvent) {
        m_Events.push(pEvent);
    }

}

