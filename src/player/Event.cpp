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
#include "Node.h"

#include "../base/TimeSource.h"
#include "../base/Logger.h"

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

int Event::getWhen() const
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

void Event::setElement(NodePtr pNode)
{
    m_pNode = pNode;
}

NodePtr Event::getElement() const
{
    return m_pNode;
}
void Event::trace()
{
    string sType;
    switch(m_Type) {
        case KEYUP:
            sType = "KEYUP";
            break;
        case KEYDOWN:
            sType = "KEYDOWN";
            break;
        case CURSORMOTION:
            sType = "CURSORMOTION";
            break;
        case CURSORUP:
            sType = "CURSORUP";
            break;
        case CURSORDOWN:
            sType = "CURSORDOWN";
            break;
        case CURSOROVER:
            sType = "CURSOROVER";
            break;
        case CURSOROUT:
            sType = "CURSOROUT";
            break;
        case RESIZE:
            sType = "RESIZE";
            break;
        case QUIT:
            sType = "QUIT";
            break;
        default:
            sType = "UNKNOWN EVENT ";
            break;
    }
    if (!m_pNode) {
        AVG_TRACE(Logger::EVENTS, sType); 
    } else {
        AVG_TRACE(Logger::EVENTS, m_pNode->getID()+", "+sType); 
    }
}

}
