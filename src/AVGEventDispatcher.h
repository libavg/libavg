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
#ifndef _AVGEventDispatcher_h_
#define _AVGEventDispatcher_h_

#include "IAVGEventSink.h"
#include "IAVGEventSource.h"

#include <vector>
#include <queue>

class AVGEvent;

class AVGEventDispatcher {
    public:
        AVGEventDispatcher();
        virtual ~AVGEventDispatcher();
        void dispatch();
        void addSource(IAVGEventSource * pSource);
        void addSink(IAVGEventSink * pSink);

        void addEvent(AVGEvent* pEvent);
        static AVGEvent * createEvent(const char * pTypeName);

    private:
        std::vector<IAVGEventSource*> m_EventSources;
        std::vector<IAVGEventSink*> m_EventSinks;

        std::priority_queue<AVGEvent *,std::vector<AVGEvent *>,isEventAfter> 
            m_Events;
};

#endif

