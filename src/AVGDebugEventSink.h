//=============================================================================
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

#ifndef _AVGDebugEventSink_h_
#define _AVGDebugEventSink_h_

#include "AVGEvent.h"
#include "IAVGEventSink.h"


class AVGDebugEventSink: public IAVGEventSink {
    public:
        AVGDebugEventSink();
        virtual bool handleEvent(AVGEvent * pEvent);
};


#endif
