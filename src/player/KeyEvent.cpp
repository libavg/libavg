//
// $Id$
//

#include "KeyEvent.h"
#include "KeyEventFactory.h"
#include "Player.h"
#include "../base/Logger.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

KeyEvent::KeyEvent()
{
}

KeyEvent::~KeyEvent()
{
}

void KeyEvent::init(int eventType, unsigned char scanCode, int keyCode, 
                const string& keyString, int modifiers)
{
    Event::init(eventType);
    m_ScanCode = scanCode;
    m_KeyCode = keyCode;
    m_KeyString = keyString;
    m_Modifiers = modifiers;
}

unsigned char KeyEvent::getScanCode()
{
    return m_ScanCode;
}

int KeyEvent::getKeyCode()
{
    return m_KeyCode;
}

const std::string& KeyEvent::getKeyString()
{
    return m_KeyString;
}

int KeyEvent::getModifiers()
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

JSFactoryBase* KeyEvent::getFactory()
{
    return KeyEventFactory::getInstance();
}

}
