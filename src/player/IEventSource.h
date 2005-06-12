//=============================================================================
//
// $Id$
//

#ifndef _EventSource_h_
#define _EventSource_h_

#include "Event.h"
#include <vector>

namespace avg {

class IEventSource {
    public:
        virtual void initEventSource() {};
        virtual std::vector<Event *> pollEvents()=0;
};

}

#endif

