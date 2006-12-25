#include "Node.h"
#include "CursorEvent.h"
namespace avg{
CursorEvent::CursorEvent(int id, Type eventType, IntPoint Position)
            :Event(eventType),
            m_Position(Position),
            m_ID(id)
{}

CursorEvent::~CursorEvent(){}

Event *CursorEvent::cloneAs(Type EventType)
{
    assert(false);
}

int CursorEvent::getXPosition() const
{
    return m_Position.x;
}

int CursorEvent::getYPosition() const
{
    return m_Position.y;
}

int CursorEvent::getCursorID() const
{
    return m_ID;
}

}

