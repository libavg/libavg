//
// $Id$
//

#include "DebugEventSink.h"
#include "Event.h"

#include <iostream>

using namespace std;

namespace avg {

DebugEventSink::DebugEventSink()
{
}

bool DebugEventSink::handleEvent(Event * pEvent) {
    if (pEvent->getType() != Event::MOUSEMOTION) {
        pEvent->trace();
    }
    return false; // Allow other handlers to really process the event.
}

}
