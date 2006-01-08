//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  Current versions can be found at www.libavg.de
//

#include "Event.h"

#include "../base/TimeSource.h"
#include "../base/Logger.h"

#include <iostream>
#include <sstream>

namespace avg {

int Event::s_CurCounter = 0;

Event::Event(Type type, int when)
    : m_Type(type)
{
    if (when == -1) {
        m_When = TimeSource::get()->getCurrentMillisecs();
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
