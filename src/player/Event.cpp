//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2008 Ulrich von Zadow
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
#include "VisibleNode.h"

#include "../base/TimeSource.h"
#include "../base/Logger.h"
#include "../base/ObjectCounter.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

int Event::s_CurCounter = 0;

Event::Event(Type type, Source source, int when)
    : m_Type(type),
      m_pNode(),
      m_Source(source)
{
    ObjectCounter::get()->incRef(&typeid(*this));
    if (when == -1) {
        m_When = TimeSource::get()->getCurrentMillisecs();
    } else {
        m_When = when;
    }
    // Make sure two events with the same timestamp are ordered correctly.
    s_CurCounter++;
    m_Counter = s_CurCounter;    
}

Event::Event(const Event& e)
{
    ObjectCounter::get()->incRef(&typeid(*this));
    *this = e;
}

Event::~Event()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

long long Event::getWhen() const
{
    return m_When;
}

Event::Type Event::getType() const
{
    return m_Type;
}

Event::Source Event::getSource() const
{
    return m_Source;
}

void Event::setElement(VisibleNodePtr pNode)
{
    m_pNode = pNode;
}

string Event::typeStr() const
{
    return typeStr(m_Type);
}

string Event::typeStr(Event::Type type)
{
    switch(type) {
        case KEYUP:
            return "KEYUP";
        case KEYDOWN:
            return "KEYDOWN";
        case CURSORMOTION:
            return "CURSORMOTION";
        case CURSORUP:
            return "CURSORUP";
        case CURSORDOWN:
            return "CURSORDOWN";
        case CURSOROVER:
            return "CURSOROVER";
        case CURSOROUT:
            return "CURSOROUT";
        case RESIZE:
            return "RESIZE";
        case QUIT:
            return "QUIT";
        default:
            return "UNKNOWN";
    }
        
}

VisibleNodePtr Event::getElement() const
{
    return m_pNode;
}

void Event::trace()
{
    string sType = typeStr();
    if (!m_pNode) {
        AVG_TRACE(Logger::EVENTS, sType); 
    } else {
        AVG_TRACE(Logger::EVENTS, m_pNode->getID()+", "+sType); 
    }
}

}
