//
// $Id$
//

#include "MouseEvent.h"
#include "Node.h"

#include "../base/Logger.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

MouseEvent::MouseEvent(int eventType,
        bool leftButtonState, bool middleButtonState, bool rightButtonState,
        int xPosition, int yPosition, int button)
    : Event(eventType),
      m_pNode(0)
{
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

MouseEvent::~MouseEvent()
{
}

Node * MouseEvent::getElement() const
{
    return m_pNode;
}

bool MouseEvent::getLeftButtonState() const
{
    return m_LeftButtonState;
}

bool MouseEvent::getMiddleButtonState() const
{
    return m_MiddleButtonState;
}

bool MouseEvent::getRightButtonState() const
{
    return m_RightButtonState;
}

int MouseEvent::getXPosition() const
{
    return m_XPosition;
}

int MouseEvent::getYPosition() const
{
    return m_YPosition;
}

int MouseEvent::getButton() const
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

}
