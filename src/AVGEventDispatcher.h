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

class AVGEventDispatcher {
    public:
        AVGEventDispatcher();
        virtual ~AVGEventDispatcher();
        void dispatch();
        void addSource(IAVGEventSource * pSource);
        void addSink(IAVGEventSink * pSink);
    private:
        std::vector<IAVGEventSource*> m_EventSources;
        std::vector<IAVGEventSink*> m_EventSinks;
};

#endif

