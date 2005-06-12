//=============================================================================
//
// $Id$
//

#ifndef _IEventSink_
#define _IEventSink_

#include "Event.h"

namespace avg {

class IEventSink {
    public:
        virtual bool handleEvent(Event * pEvent) =0;
};

}
#endif

