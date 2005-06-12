//
// $Id$
//

#ifndef _DebugEventSink_h_
#define _AVGDebugEventSink_h_

#include "Event.h"
#include "IEventSink.h"

namespace avg {

class DebugEventSink: public IEventSink {
    public:
        DebugEventSink();
        virtual bool handleEvent(Event * pEvent);
};

}
#endif
