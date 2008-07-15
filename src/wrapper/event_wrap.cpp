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

#include "WrapHelper.h"

#include "../player/KeyEvent.h"
#include "../player/MouseEvent.h"
#include "../player/TouchEvent.h"
#include "../player/Node.h"
#include "../player/TrackerEventSource.h"

using namespace boost::python;
using namespace avg;
using namespace std;

void export_event()
{
    boost::python::to_python_converter<vector<TouchEvent*>, 
        to_tuple<vector<TouchEvent *> > >();
          
    boost::python::to_python_converter<ContourSeq, 
        to_list<ContourSeq> >();    
   
    from_python_sequence<ContourSeq, variable_capacity_policy>();

    class_<Event, boost::noncopyable>("Event", 
            "Base class for user input events.\n",
            no_init)
        .add_property("type", &Event::getType,
                "One of KEYUP, KEYDOWN, CURSORMOTION, CURSORUP,\n"
                "CURSORDOWN, CURSOROVER, CURSOROUT, RESIZE or QUIT.\n")
        .add_property("when", &Event::getWhen,
                "The timestamp of the event in milliseconds.\n")
    ;

    enum_<Event::Type>("Type")
        .value("KEYUP", Event::KEYUP)
        .value("KEYDOWN", Event::KEYDOWN)
        .value("CURSORMOTION", Event::CURSORMOTION)
        .value("CURSORUP", Event::CURSORUP)
        .value("CURSORDOWN", Event::CURSORDOWN)
        .value("CURSOROVER", Event::CURSOROVER)
        .value("CURSOROUT", Event::CURSOROUT)
        .value("RESIZE", Event::RESIZE)
        .value("QUIT", Event::QUIT)
        .export_values()
    ;

    enum_<CursorEvent::Source>("Source")
        .value("MOUSE", CursorEvent::MOUSE)
        .value("TOUCH", CursorEvent::TOUCH)
        .value("TRACK", CursorEvent::TRACK)
        .value("NONE", Event::NONE)
        .export_values()
    ;

    class_<KeyEvent, bases<Event> >("KeyEvent", 
            "Raised when a key is pressed or released.\n",
            no_init)
        .add_property("scancode", &KeyEvent::getScanCode,
            "The untranslated scancode of the key pressed (ro).\n")
        .add_property("keycode", &KeyEvent::getKeyCode,
            "The keycode of the key according to the current layout (ro).\n")
        .add_property("keystring", make_function(&KeyEvent::getKeyString, 
                return_value_policy<copy_const_reference>()),
            "A character or word describing the key pressed (ro).\n")
        // TODO: Export possible modifiers as enum.
        .add_property("modifiers", &KeyEvent::getModifiers,
            "Any modifiers (shift, ctrl,...) pressed as well (ro).\n")
    ;    
    
    class_<MouseEvent, bases<Event> >("MouseEvent", 
            "Raised when a mouse-related event occurs.\n",
            no_init)
        .add_property("source", &MouseEvent::getSource,
                "Always MOUSE")
        .add_property("leftbuttonstate", &MouseEvent::getLeftButtonState)
        .add_property("middlebuttonstate", &MouseEvent::getMiddleButtonState)
        .add_property("rightbuttonstate", &MouseEvent::getRightButtonState)
        .add_property("x", &MouseEvent::getXPosition,
                "x position in the global coordinate system (ro).\n")
        .add_property("y", &MouseEvent::getYPosition,
                "y position in the global coordinate system (ro).\n")
        .add_property("cursorid", &MouseEvent::getCursorID,
                "Always -1 for mouse events, but can be used to handle mouse and \n"
                "tracking events at once (ro).\n")
        .add_property("button", &MouseEvent::getButton,
                "The button that caused the event (ro).\n")
        .add_property("node", &MouseEvent::getElement,
                "The node that the event handler was declared in (ro).\n")
        ;

    class_<TouchEvent, bases<Event> >("TouchEvent", 
            "Raised when a touch or other tracking event occurs. Touch events happen \n"
            "only when a multi-touch sensitive surface or other camera tracker is \n"
            "active. For each touch event, statistical data based on moments are \n"
            "calculated.",
            no_init)
        .add_property("source", &TouchEvent::getSource,
                "TOUCH for actual touches, TRACK for hands above the surface or\n"
                "generic tracking.\n")
        .add_property("area", &TouchEvent::getArea,
                "Size of the blob found in pixels (ro).\n")
        .add_property("orientation", &TouchEvent::getOrientation,
                "Angle of the blob. For hovering hands, this is roughly the direction \n"
                "of the hand (ro).\n")
        .add_property("inertia", &TouchEvent::getInertia)
        .add_property("eccentricity", &TouchEvent::getInertia)
        .add_property("x", &TouchEvent::getXPosition,
                "x position in the global coordinate system (ro).\n")
        .add_property("y", &TouchEvent::getYPosition,
                "y position in the global coordinate system (ro).\n")
        .add_property("cursorid", &TouchEvent::getCursorID)
        .add_property("node", &TouchEvent::getElement,
                "The node that the event handler was declared in (ro).\n")
        .add_property("center", make_function(&TouchEvent::getCenter,
                return_value_policy<copy_const_reference>()),
                "Position as double. Used for calibration (ro).\n")
        .add_property("majoraxis", make_function(&TouchEvent::getMajorAxis,
                return_value_policy<copy_const_reference>()),
                "Major axis of an ellipse that is similar to the blob (ro).\n")
        .add_property("minoraxis", make_function(&TouchEvent::getMinorAxis,
                return_value_policy<copy_const_reference>()),
                "Minor axis of an ellipse that is similar to the blob (ro).\n")
        .add_property("speed", make_function(&TouchEvent::getSpeed,
                return_value_policy<copy_const_reference>()),
                "Current speed of the blob in pixels per millisecond as a\n"
                "2-component vector (ro).\n")
        .def("getRelatedEvents", &TouchEvent::getRelatedEvents,
                "getRelatedEvents() -> events\n"
                "Returns a python tuple containing the events 'related' to this one.\n"
                "For TOUCH events (fingers), the tuple contains one element: the\n"
                "corresponding TRACK event (hand). For TRACK events, the tuple contains\n"
                "all TOUCH events that belong to the same hand.\n")
        .def("getContour", &TouchEvent::getContour,
                "getContour()\n"
                "Extracts contour envelope sequence for the event\n")
        ;
   
    enum_<TrackerImageID>("TrackerImageID")
        .value("IMG_CAMERA", TRACKER_IMG_CAMERA)
        .value("IMG_DISTORTED", TRACKER_IMG_DISTORTED)
        .value("IMG_NOHISTORY", TRACKER_IMG_NOHISTORY)
        .value("IMG_HISTOGRAM", TRACKER_IMG_HISTOGRAM)
        .value("IMG_FINGERS", TRACKER_IMG_FINGERS)
        .value("IMG_HIGHPASS", TRACKER_IMG_HIGHPASS)
        .export_values()
    ;

    class_<TrackerEventSource, boost::noncopyable>("Tracker",
            "A tracker that uses a camera to track moving objects\n"
            "(e.g. fingers) and delivers them to the player as avg events.\n"
            "Create a tracker by using Player::addTracker(). The properties\n"
            "of this class are explained under U{https://www.libavg.de/wiki/index.php/Tracker_Setup}.",
            no_init)
        .def("getImage", &TrackerEventSource::getImage,
            return_value_policy<manage_new_object>(),
            "getImage(imageid) -> bitmap\n" 
            "Returns one of the intermediate images necessary for tracking.\n"
            "These images are only available if setDebugImages was called before\n"
            "with appropriate parameters.\n"
            "@param imageid: One of IMG_CAMERA, IMG_DISTORTED, IMG_NOHISTORY,\n"
            "IMG_HISTOGRAM, IMG_FINGERS or IMG_HIGHPASS.\n")
        .def("saveConfig", &TrackerEventSource::saveConfig,
            "saveConfig(filename)\n"
            "Saves the current tracker configuration to the filename given. If filename\n"
            "is empty, the file used to construct the tracker is used.\n")
        .def("resetHistory", &TrackerEventSource::resetHistory,
            "resetHistory()\n"
            "Throws away the current history image and generates a new one from\n"
            "the next second of images.\n")
        .def("setDebugImages", &TrackerEventSource::setDebugImages,
            "setDebugImages(img, finger)\n"
            "Controls whether debug images of intermediate tracking results\n"
            "and detected finger positions are generated and exported to\n"
            "python. Generating the debug images takes a moderate amount of\n"
            "time, so it is turned off by default.\n"
            "@param img: Whether to generate intermediate result images.\n")
        .def("startCalibration", &TrackerEventSource::startCalibration,
            return_value_policy<reference_existing_object>(),
            "startCalibration(displayextents) -> trackercalibrator\n"
            "Starts coordinate calibration session. The returned TrackerCalibrator\n"
            "exists until endCalibration or abortCalibration is called.\n"
            "@param displayextents: The width and height of the display area.\n")
        .def("endCalibration", &TrackerEventSource::endCalibration,
            "endCalibration()\n"
            "Ends coordinate calibration session and activates the coordinate\n"
            "transformer generated.\n")
        .def("abortCalibration", &TrackerEventSource::abortCalibration,
            "abortCalibration()\n"
            "Aborts coordinate calibration session and restores the previous\n"
            "coordinate transformer.\n")

        .def("setParam", &TrackerEventSource::setParam,
            "setParam()\n"
            "Updates an arbitrary parameter of tracker configuration.\n")
        .def("getParam", &TrackerEventSource::getParam,
            "getParam()\n"
            "Retrieves an arbitrary parameter of tracker configuration.\n")

        ;

class_<TrackerCalibrator, boost::noncopyable>("TrackerCalibrator",
            "Used to map display points to camera points by mapping a set of reference\n"
            "points. Python code should display reference points that the user must\n"
            "touch to establish a mapping. Created by Tracker::startCalibration.\n",
            no_init)
        .def("nextPoint", &TrackerCalibrator::nextPoint,
            "nextPoint() -> done\n"
            "Advances to the next point. Returns False and ends calibration if\n"
            "all points have been set.\n")
        .def("getDisplayPoint", &TrackerCalibrator::getDisplayPoint,
            "getDisplayPoint() -> pos\n")
        .def("setCamPoint", &TrackerCalibrator::setCamPoint,
            "setCamPoint(pt)\n")
        ;
}
