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

#include <paintlib/plstdpch.h>

#include <xpcom/nsCOMPtr.h>
#include <xpcom/nsIComponentManager.h>

#include <string>

using namespace std;

namespace input {

    AVGEventDispatcher::AVGEventDispatcher() {
    };

    AVGEventDispatcher::~AVGEventDispatcher() {
    };

    void
    AVGEventDispatcher::dispatch() {
        for (int i = 0; i<m_EventSources.size(); ++i) {
            vector<AVGEvent*> curEvents = m_EventSources[i]->pollEvents();
            for (int i= 0; i<curEvents.size(); i++) {
                m_Events.push(curEvents[i]);
            }
        }

        while (!m_Events.empty()) {
            AVGEvent * curEvent = m_Events.top();
            m_Events.pop();
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

    void 
    AVGEventDispatcher::addEvent(AVGEvent* pEvent) {
        m_Events.push(pEvent);
    }

    AVGEvent * AVGEventDispatcher::createEvent(const char * pTypeName)
    {
        nsresult rv;
        nsCOMPtr<IAVGEvent> pXPEvent = do_CreateInstance((string("@c-base.org/")+pTypeName+";1").c_str(), &rv);
        PLASSERT(!NS_FAILED(rv));
        NS_IF_ADDREF((IAVGEvent*)pXPEvent);
        return dynamic_cast<AVGEvent*>((IAVGEvent*)pXPEvent);
    }

}

