//
// $Id$
// 

%module avg
%{
#include "../player/KeyEvent.h"
%}

%include "Event.i"

%attribute(avg::KeyEvent, int, scancode, getScanCode);
%attribute(avg::KeyEvent, int, keycode, getKeyCode);
%attribute(avg::KeyEvent, const std::string&, keystring, getKeyString);
%attribute(avg::KeyEvent, int, modifiers, getModifiers);

namespace avg {

class KeyEvent: public Event
{
    private:
        KeyEvent();
};

}
