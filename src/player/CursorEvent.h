#ifndef _CursorEvent_h_
#define _CursorEvent_h_

#include "Event.h"
#include "Node.h"

namespace avg{
class CursorEvent: public Event {
    protected:
        int m_XPosition;
        int m_YPosition;
        int m_ID;
    public:
        CursorEvent(int id, Type eventType, int xPosition, int yPosition);
        virtual ~CursorEvent();
        virtual Event* cloneAs(Type EventType);
        int getXPosition() const;
        int getYPosition() const;
        int getCursorID() const;

};
}

#endif
