//
// $Id$
//

#include "KeyEvent.h"
#include "Player.h"
#include "../base/Logger.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

KeyEvent::KeyEvent(Type eventType, unsigned char scanCode, int keyCode, 
                const string& keyString, int modifiers)
    : Event(eventType)
{
    m_ScanCode = scanCode;
    m_KeyCode = keyCode;
    m_KeyString = keyString;
    m_Modifiers = modifiers;
}

KeyEvent::~KeyEvent()
{
}

unsigned char KeyEvent::getScanCode() const
{
    return m_ScanCode;
}

int KeyEvent::getKeyCode() const
{
    return m_KeyCode;
}

const std::string& KeyEvent::getKeyString() const
{
    return m_KeyString;
}

int KeyEvent::getModifiers() const
{
    return m_Modifiers;
}

void KeyEvent::trace()
{
    Event::trace();
    AVG_TRACE(Logger::EVENTS2, "Scancode: " << m_ScanCode 
            << ", Keycode: " << m_KeyCode << ", KeyString: " 
            << m_KeyString << ", Modifiers: " << m_Modifiers);
}

}
