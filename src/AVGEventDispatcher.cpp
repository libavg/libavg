//=============================================================================
//
// Original code Copyright (C) 2003, ART+COM AG Berlin
//
// Released under LGPL.
//
//=============================================================================
//
//   $RCSfile$
//   $Author$
//   $Revision$
//   $Date$
//
//=============================================================================

#include "AVGEventDispatcher.h"
#include "AVGEvent.h"

#include <queue>

using namespace std;

namespace input {

    AVGEventDispatcher::AVGEventDispatcher() {
    };

    AVGEventDispatcher::~AVGEventDispatcher() {
    };

    void
    AVGEventDispatcher::dispatch() {
        std::priority_queue<AVGEvent *,vector<AVGEvent *>,isEventAfter> sortedEvents;

        for (int i = 0; i<m_EventSources.size(); ++i) {
            vector<AVGEvent*> curEvents = m_EventSources[i]->pollEvents();
            for (int i= 0; i<curEvents.size(); i++) {
                sortedEvents.push(curEvents[i]);
            }
        }

        while (!sortedEvents.empty()) {
            AVGEvent * curEvent = sortedEvents.top();
            sortedEvents.pop();
            for (int i = 0; i < m_EventSinks.size(); ++i) {
                if (m_EventSinks[i]->handleEvent(curEvent)) {
                    break;
                }
            }
            NS_IF_RELEASE(curEvent);
        }
        
    };

    void
    AVGEventDispatcher::addSource(IAVGEventSource * pSource) {
        m_EventSources.push_back(pSource);
        pSource->initEventSource();
    }

    void
    AVGEventDispatcher::addSink(IAVGEventSink * pSink) {
        m_EventSinks.push_back(pSink);
    }
}

