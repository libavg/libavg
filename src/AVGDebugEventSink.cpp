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

#include "AVGDebugEventSink.h"
#include "AVGEvent.h"

#include <iostream>

using namespace std;

AVGDebugEventSink::AVGDebugEventSink()
{
}

bool AVGDebugEventSink::handleEvent(AVGEvent * pEvent) {
    if (pEvent->getType() != AVGEvent::MOUSEMOTION) {
        pEvent->trace();
    }
    return false; // Allow other handlers to really process the event.
}

