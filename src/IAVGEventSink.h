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
#ifndef _Y60_INPUT_IEVENTSINK_INCLUDED_
#define _Y60_INPUT_IEVENTSINK_INCLUDED_

#include "AVGEvent.h"

class IAVGEventSink {
    public:
        virtual bool handleEvent(AVGEvent * pEvent) =0;
};

#endif

