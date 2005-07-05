//
// $Id$
// 

%module avg
%{
#include "../player/MouseEvent.h"
%}

%include "Event.i"
%include "Node.i"

%attribute(avg::MouseEvent, bool, leftbuttonstate, getLeftButtonState);
%attribute(avg::MouseEvent, bool, middlebuttonstate, getMiddleButtonState);
%attribute(avg::MouseEvent, bool, rightbuttonstate, getRightButtonState);
%attribute(avg::MouseEvent, int, x, getXPosition);
%attribute(avg::MouseEvent, int, y, getYPosition);
%attribute(avg::MouseEvent, int, button, getButton);
%attribute(avg::MouseEvent, avg::Node *, node, getElement);

namespace avg {

class MouseEvent: public Event
{
    private:
        MouseEvent();
};

}

