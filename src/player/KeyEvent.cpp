//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2014 Ulrich von Zadow
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

#include "KeyEvent.h"
#include "../base/Logger.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

KeyEvent::KeyEvent(Type eventType, int scanCode, const string& sKeyString, int modifiers)
    : Event(eventType)
{
    m_ScanCode = scanCode;
    m_sKeyString = sKeyString;
    m_Modifiers = modifiers;
}

KeyEvent::~KeyEvent()
{
}

unsigned char KeyEvent::getScanCode() const
{
    return m_ScanCode;
}

void KeyEvent::setText(const string& sText)
{
    AVG_ASSERT(getType() == KEY_DOWN);
    m_sText = sText;
}

string KeyEvent::getText() const
{
    return m_sText;
}

const std::string& KeyEvent::getKeyString() const
{
    return m_sKeyString;
}

int KeyEvent::getModifiers() const
{
    return m_Modifiers;
}

void KeyEvent::trace()
{
    Event::trace();
    AVG_TRACE(Logger::category::EVENTS, Logger::severity::DEBUG,
            "Scancode: " << m_ScanCode << ", Text: " << m_sText << ", KeyString: "
            << m_sKeyString << ", Modifiers: " << m_Modifiers);
}

}
