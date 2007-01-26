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
#include "../player/TouchEvent.h"
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
        .value("MOUSEOVER", Event::CURSOROVER)
        .value("MOUSEOUT", Event::CURSOROUT)
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
        .add_property("cursorid", &MouseEvent::getCursorID)
        .add_property("button", &MouseEvent::getButton)
        .add_property("node", &MouseEvent::getElement);

    class_<TouchEvent, bases<Event> >("TouchEvent", 
            "Raised when a touch event occurs. Touch events happen only when a multi-touch\n"
            "sensitive surface is active.\n"
            "Properties:\n"
            "    area: size of the blob(ro)\n"
            "    orientation: (ro)\n"
            "    inertia: (ro)\n"
            "    eccentricity: (ro)\n"
            "    cursorid: (ro)\n"
            "    x: x position in the global coordinate system. (ro)\n"
            "    y: y position in the global coordinate system. (ro)\n"
            "    node: The node that the event handler was declared in. (ro)\n"
            "    center: position as double. Used for calibration (ro)\n",
            no_init)
        .add_property("area", &TouchEvent::getArea)
        .add_property("orientation", &TouchEvent::getOrientation)
        .add_property("inertia", &TouchEvent::getInertia)
        .add_property("eccentricity", &TouchEvent::getInertia)
        .add_property("x", &TouchEvent::getXPosition)
        .add_property("y", &TouchEvent::getYPosition)
        .add_property("cursorid", &TouchEvent::getCursorID)
        .add_property("node", &TouchEvent::getElement)
        .add_property("center", &TouchEvent::getCenter);
   
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
            "A tracker that uses a firewire camera to track moving objects\n"
            "(e.g. fingers) and delivers them to the player as avg events.\n"
            "Create using Player::addTracker().\n",
            no_init)
        .def("getImage", &TrackerEventSource::getImage,
            return_value_policy<manage_new_object>(),
            "getImage(ImageID) -> Bitmap\n\n" 
            "Returns one of the intermediate images necessary for tracking.\n"
            "These images are only available if setDebugImages was called before.\n")
        .def("saveConfig", &TrackerEventSource::saveConfig,
            "saveConfig() -> None\n\n"
            "Saves the tracker configuration to TrackerConfig.xml in the current\n"
            "directory.\n")
        .def("resetHistory", &TrackerEventSource::resetHistory,
            "resetHistory() -> None\n\n"
            "Throws away the current history image and generates a new one from\n"
            "the current image.\n")
        .def("setDebugImages", &TrackerEventSource::setDebugImages,
            "setDebugImages(Img, Finger) -> None\n\n"
            "Controls whether debug images of intermediate tracking results (Img)\n"
            "and detected finger positions (Finger) are generated.\n")
        .def("startCalibration", &TrackerEventSource::startCalibration,
            return_value_policy<manage_new_object>(),
            "startCalibration(DisplayExtents) -> TrackerCalibrator\n"
            "Starts coordinate calibration session.\n")
        .add_property("barrel", &TrackerEventSource::getBarrel,
            &TrackerEventSource::setBarrel)
        .add_property("trapezoid", &TrackerEventSource::getTrapezoid,
            &TrackerEventSource::setTrapezoid)

        .add_property("roileft", &TrackerEventSource::getROILeft,
            &TrackerEventSource::setROILeft)
        .add_property("roitop", &TrackerEventSource::getROITop,
            &TrackerEventSource::setROITop)
        .add_property("roiright", &TrackerEventSource::getROIRight,
            &TrackerEventSource::setROIRight)
        .add_property("roibottom", &TrackerEventSource::getROIBottom,
            &TrackerEventSource::setROIBottom)
        
        .add_property("threshold", &TrackerEventSource::getThreshold,
            &TrackerEventSource::setThreshold)
        .add_property("historyspeed", &TrackerEventSource::getHistorySpeed,
            &TrackerEventSource::setHistorySpeed)
        .add_property("brightness", &TrackerEventSource::getBrightness,
            &TrackerEventSource::setBrightness)
        .add_property("exposure", &TrackerEventSource::getExposure,
            &TrackerEventSource::setExposure)
        .add_property("gamma", &TrackerEventSource::getGamma,
            &TrackerEventSource::setGamma)
        .add_property("gain", &TrackerEventSource::getGain,
            &TrackerEventSource::setGain)
        .add_property("shutter", &TrackerEventSource::getShutter,
            &TrackerEventSource::setShutter)
        ;

class_<TrackerCalibrator, boost::noncopyable>("TrackerCalibrator",
            "Used to map display points to camera points by mapping a set of reference\n"
            "points. Python code should display reference points that the user must\n"
            "touch to establish a mapping. Created by Tracker::startCalibration.\n",
            no_init)
        .def("nextPoint", &TrackerCalibrator::nextPoint,
            "nextPoint(None) -> Bool\n"
            "Advances to the next point. Returns False and ends calibration if\n"
            "all points have been set.\n")
        .def("getDisplayPointX", &TrackerCalibrator::getDisplayPointX,
            "getDisplayPointX(None) -> Pos\n")
        .def("getDisplayPointY", &TrackerCalibrator::getDisplayPointY,
            "getDisplayPointY(None) -> Pos\n")
        .def("setCamPoint", &TrackerCalibrator::setCamPoint,
            "setCamPoint(x, y) -> None\n")
        .def("abort", &TrackerCalibrator::abort,
            "abort() -> None\n")
        ;
}
