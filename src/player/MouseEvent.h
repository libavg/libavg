//
// $Id$
//

#ifndef _MouseEvent_h_
#define _MouseEvent_h_

#include "Event.h"

namespace avg {

class Node;

class MouseEvent : public Event {
    public:
        MouseEvent(int eventType,
                bool leftButtonState, bool middleButtonState, 
                bool rightButtonState,
                int xPosition, int yPosition, int button);
        virtual ~MouseEvent();
       
        Node * getElement() const;
        bool getLeftButtonState() const;
        bool getMiddleButtonState() const;
        bool getRightButtonState() const;
        int getXPosition() const;
        int getYPosition() const;
        int getButton() const;

        void setElement(Node * pNode);
        virtual void trace();
        
        static const long NO_BUTTON=0;
        static const long LEFT_BUTTON=1;
        static const long RIGHT_BUTTON=2;
        static const long MIDDLE_BUTTON=3;

    private:
        bool m_LeftButtonState;
        bool m_MiddleButtonState;
        bool m_RightButtonState;
        int m_XPosition;
        int m_YPosition;
        int m_Button; // only used in button events
        Node * m_pNode;
};

}

#endif

