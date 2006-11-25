//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  Current versions can be found at www.libavg.de
//

#include "../player/KeyEvent.h"
#include "../player/MouseEvent.h"
#include "../player/Node.h"
#include "../player/TrackerEventSource.h"

#include <boost/python.hpp>

using namespace boost::python;
using namespace avg;

void export_event()
{
    class_<Event, boost::noncopyable>("Event", 
            "Base class for user input events.\n"
            "Properties:\n"
            "    type: One of KEYUP, KEYDOWN, MOUSEMOTION, MOUSEBUTTONUP,\n"
            "          MOUSEBUTTONDOWN, MOUSEOVER, MOUSEOUT, RESIZE or QUIT.\n"
            "    when: The timestamp of the event in milliseconds.\n",
            no_init)
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
        .export_values()
    ;

    class_<KeyEvent, bases<Event> >("KeyEvent", 
            "Raised when a key is pressed or released.\n"
            "Properties:\n"
            "    scancode: The untranslated scancode of the key pressed. (ro)\n"
            "    keycode: The keycode of the key according to the current layout. (ro)\n"
            "    keystring: A character or word describing the key pressed. (ro)\n"
            "    modifiers: Any modifiers (shift, ctrl,...) pressed as well. (ro)\n",
            no_init)
        .add_property("scancode", &KeyEvent::getScanCode)
        .add_property("keycode", &KeyEvent::getKeyCode)
        .add_property("keystring", make_function(&KeyEvent::getKeyString, 
                return_value_policy<copy_const_reference>()))
        // TODO: Export possible modifiers as enum.
        .add_property("modifiers", &KeyEvent::getModifiers)
    ;    
    
    class_<MouseEvent, bases<Event> >("MouseEvent", 
            "Raised when a mouse-related event occurs.\n"
            "Properties:\n"
            "    leftbuttonstate: (ro)\n"
            "    middlebuttonstate: (ro)\n"
            "    rightbuttonstate: (ro)\n"
            "    x: x position in the global coordinate system. (ro)\n"
            "    y: y position in the global coordinate system. (ro)\n"
            "    button: The button that caused the event. (ro)\n"
            "    node: The node that the event handler was declared in. (ro)\n",
            no_init)
        .add_property("leftbuttonstate", &MouseEvent::getLeftButtonState)
        .add_property("middlebuttonstate", &MouseEvent::getMiddleButtonState)
        .add_property("rightbuttonstate", &MouseEvent::getRightButtonState)
        .add_property("x", &MouseEvent::getXPosition)
        .add_property("y", &MouseEvent::getYPosition)
        .add_property("button", &MouseEvent::getButton)
        .add_property("node", &MouseEvent::getElement);
    
    class_<TrackerEventSource, boost::noncopyable>("Tracker",
            "A tracker that uses a firewire camera to track moving objects\n"
            "(e.g. fingers) and delivers them to the player as avg events.\n"
            "Create using Player::addTracker().\n",
            no_init)
        .def("setThreshold", &TrackerEventSource::setThreshold)
        .def("getImage", &TrackerEventSource::getImage,
            return_value_policy<manage_new_object>(),
            "getImage(ImageID) -> Bitmap\n\n" 
            "Returns one of the intermediate images nessesary for tracking.\n");
}
