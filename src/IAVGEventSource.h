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

#ifndef _AVGEventSource_h_
#define _AVGEventSource_h_

#include "AVGEvent.h"
#include <vector>

class IAVGEventSource {
    public:
        virtual void initEventSource() {};
        virtual std::vector<AVGEvent *> pollEvents()=0;
};



#endif

