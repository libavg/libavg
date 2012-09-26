//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2011 Ulrich von Zadow
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
#include "IInputDevice.h"
#include "Player.h"

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
      m_Source(source),
      m_pInputDevice()
{
    ObjectCounter::get()->incRef(&typeid(*this));
    if (when == -1) {
        m_When = Player::get()->getFrameTime();
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

IInputDevicePtr Event::getInputDevice() const
{
    return m_pInputDevice.lock();
}

bool Event::hasInputDevice() const
{
    return !m_pInputDevice.expired();
}

void Event::setInputDevice(IInputDevicePtr pInputDevice)
{
    m_pInputDevice = pInputDevice;
}

const std::string& Event::getInputDeviceName() const
{
    return m_pInputDevice.lock()->getName();
}

string Event::typeStr() const
{
    return typeStr(m_Type);
}

string Event::typeStr(Event::Type type)
{
    switch(type) {
        case KEY_UP:
            return "KEY_UP";
        case KEY_DOWN:
            return "KEY_DOWN";
        case CURSOR_MOTION:
            return "CURSOR_MOTION";
        case CURSOR_UP:
            return "CURSOR_UP";
        case CURSOR_DOWN:
            return "CURSOR_DOWN";
        case CURSOR_OVER:
            return "CURSOR_OVER";
        case CURSOR_OUT:
            return "CURSOR_OUT";
        case CUSTOM_EVENT:
            return "CUSTOM_EVENT";
        case QUIT:
            return "QUIT";
        default:
            return "UNKNOWN";
    }
        
}

void Event::trace()
{
    string sType = typeStr();
    AVG_TRACE(Logger::EVENTS, sType); 
}

}
