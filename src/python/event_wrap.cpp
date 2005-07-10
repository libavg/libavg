//
// $Id$
//

#include "../player/KeyEvent.h"
#include "../player/MouseEvent.h"

#include <boost/python.hpp>

using namespace boost::python;
using namespace avg;

void export_event()
{
    class_<Event, boost::noncopyable>("Event", no_init)
        .add_property("type", &Event::getType)
        .add_property("when", &Event::getWhen)
    ;

    enum_<Event::Type>("Type")
        .value("KEYUP", Event::KEYUP)
        .value("KEYDOWN", Event::KEYDOWN)
        .value("MOUSEMOTION", Event::MOUSEMOTION)
        .value("MOUSEBUTTONUP", Event::MOUSEBUTTONUP)
        .value("MOUSEBUTTONDOWN", Event::MOUSEBUTTONDOWN)
        .value("MOUSEOVER", Event::MOUSEOVER)
        .value("MOUSEOUT", Event::MOUSEOUT)
        .value("RESIZE", Event::RESIZE)
        .value("QUIT", Event::QUIT)
    ;

    class_<KeyEvent, bases<Event> >("KeyEvent", no_init)
        .add_property("scancode", &KeyEvent::getScanCode)
        .add_property("keycode", &KeyEvent::getKeyCode)
        .add_property("keystring", make_function(&KeyEvent::getKeyString, 
                return_value_policy<copy_const_reference>()))
        .add_property("modifiers",  &KeyEvent::getModifiers)
    ;    
}
