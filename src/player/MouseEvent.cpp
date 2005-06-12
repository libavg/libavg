//
// $Id$
//

#include "MouseEvent.h"
#include "MouseEventFactory.h"
#include "Node.h"

#include "../base/Logger.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

MouseEvent::MouseEvent()
    : m_pNode(0)
{
}

MouseEvent::~MouseEvent()
{
}

void MouseEvent::init(int eventType,
        bool leftButtonState, bool middleButtonState, bool rightButtonState,
        int xPosition, int yPosition, int button)
{
    Event::init(eventType);
    m_LeftButtonState = leftButtonState;
    m_MiddleButtonState = middleButtonState;
    m_RightButtonState = rightButtonState;
    m_XPosition = xPosition;
    m_YPosition = yPosition;
    if (eventType == MOUSEMOTION) {
        m_Button = 0;
    } else {
        m_Button = button;
    }
}

Node * MouseEvent::getElement()
{
    return m_pNode;
}

bool MouseEvent::getLeftButtonState()
{
    return m_LeftButtonState;
}

bool MouseEvent::getMiddleButtonState()
{
    return m_MiddleButtonState;
}

bool MouseEvent::getRightButtonState()
{
    return m_RightButtonState;
}

int MouseEvent::getXPosition()
{
    return m_XPosition;
}

int MouseEvent::getYPosition()
{
    return m_YPosition;
}

int MouseEvent::getButton()
{
    return m_Button;
}

void MouseEvent::setElement(Node * pNode)
{
    m_pNode = pNode;
}

void MouseEvent::trace()
{
    Event::trace();
    AVG_TRACE(Logger::EVENTS2, "pos: (" << m_XPosition 
            << ", " << m_YPosition << ")" 
            << ", button: " << m_Button);
}

JSFactoryBase* MouseEvent::getFactory()
{
    return MouseEventFactory::getInstance();
}

}
