//
// $Id$
//

#include "Event.h"

#include "../base/TimeSource.h"
#include "../base/Logger.h"

#include <iostream>
#include <sstream>

namespace avg {

int Event::s_CurCounter = 0;

Event::Event(int type, int when)
    : m_Type(type)
{
    if (when == -1) {
        m_When = TimeSource::get()->getCurrentTicks();
    } else {
        m_When = when;
    }
    // Make sure two events with the same timestamp are ordered correctly.
    s_CurCounter++;
    m_Counter = s_CurCounter;    
}

Event::~Event()
{
}

int Event::getWhen() const
{
    return m_When;
}

Event::Type Event::getType() const
{
    return m_Type;
}

void Event::trace()
{
    switch(m_Type) {
        case KEYUP:
            AVG_TRACE(Logger::EVENTS, "KEYUP");
            break;
        case KEYDOWN:
            AVG_TRACE(Logger::EVENTS, "KEYDOWN");
            break;
        case MOUSEMOTION:
            AVG_TRACE(Logger::EVENTS, "MOUSEMOTION");
            break;
        case MOUSEBUTTONUP:
            AVG_TRACE(Logger::EVENTS, "MOUSEBUTTONUP");
            break;
        case MOUSEBUTTONDOWN:
            AVG_TRACE(Logger::EVENTS, "MOUSEBUTTONDOWN");
            break;
        case MOUSEOVER:
            AVG_TRACE(Logger::EVENTS, "MOUSEOVER");
            break;
        case MOUSEOUT:
            AVG_TRACE(Logger::EVENTS, "MOUSEOUT");
            break;
        case RESIZE:
            AVG_TRACE(Logger::EVENTS, "RESIZE");
            break;
        case QUIT:
            AVG_TRACE(Logger::EVENTS, "QUIT");
            break;
    }
}

}
