//=============================================================================
// Copyright (C) 2003, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================
//
//   $RCSfile$
//   $Author$
//   $Revision$
//   $Date$
//
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
