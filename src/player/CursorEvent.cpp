#include "Node.h"
#include "CursorEvent.h"
namespace avg{
CursorEvent::CursorEvent(int id, Type eventType, int xPosition, int yPosition)
            :Event(eventType),
            m_XPosition(xPosition),
            m_YPosition(yPosition),
            m_ID(id)
{}

CursorEvent::~CursorEvent(){}

Event *CursorEvent::cloneAs(Type EventType)
{
    assert(false);
}

int CursorEvent::getXPosition() const
{
    return m_XPosition;
}

int CursorEvent::getCursorID() const
{
    return m_ID;
}

int CursorEvent::getYPosition() const
{
    return m_YPosition;
}


}

