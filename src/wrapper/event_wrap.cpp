//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2011 Ulrich von Zadow
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
#include "../player/Contact.h"
#include "../player/TrackerInputDevice.h"
#include "../player/Publisher.h"

#include <boost/shared_ptr.hpp>
#include <string>

using namespace boost::python;
using namespace avg;
using namespace std;


class IInputDeviceWrapper : public IInputDevice, public wrapper<IInputDevice>
{
    public:
        IInputDeviceWrapper(const std::string& name,
                const DivNodePtr& pEventReceiverNode=DivNodePtr())
            : IInputDevice(name, pEventReceiverNode)
        {
        }

        IInputDeviceWrapper(const IInputDevice& inputDevice)
            : IInputDevice(inputDevice)
        {
        }

        virtual void start() 
        {
            override startMethod = this->get_override("start");
            if (startMethod) {
                startMethod();
            }
            IInputDevice::start();
        }

        void default_start() 
        {
            return this->IInputDevice::start();
        }

        virtual std::vector<EventPtr> pollEvents() 
        {
            return this->get_override("pollEvents")();
        }

};

void export_event()
{
    boost::python::to_python_converter<vector<TouchEventPtr>, 
        to_tuple<vector<TouchEventPtr> > >();
    boost::python::to_python_converter<vector<CursorEventPtr>, 
        to_tuple<vector<CursorEventPtr> > >();

    boost::python::to_python_converter<ContourSeq, to_list<ContourSeq> >();    
   
    from_python_sequence<ContourSeq, variable_capacity_policy>();
    from_python_sequence<vector<EventPtr>, variable_capacity_policy>();

    enum_<int>("KeyModifier")
        .value("KEYMOD_NONE", key::KEYMOD_NONE)
        .value("KEYMOD_LSHIFT", key::KEYMOD_LSHIFT)
        .value("KEYMOD_RSHIFT", key::KEYMOD_RSHIFT)
        .value("KEYMOD_LCTRL", key::KEYMOD_LCTRL)
        .value("KEYMOD_RCTRL", key::KEYMOD_RCTRL)
        .value("KEYMOD_LALT", key::KEYMOD_LALT)
        .value("KEYMOD_RALT", key::KEYMOD_RALT)
        .value("KEYMOD_LMETA", key::KEYMOD_LMETA)
        .value("KEYMOD_RMETA", key::KEYMOD_RMETA)
        .value("KEYMOD_NUM", key::KEYMOD_NUM)
        .value("KEYMOD_CAPS", key::KEYMOD_CAPS)
        .value("KEYMOD_MODE", key::KEYMOD_MODE)
        .value("KEYMOD_RESERVED", key::KEYMOD_RESERVED)
        .value("KEYMOD_CTRL", key::KEYMOD_CTRL)
        .value("KEYMOD_SHIFT", key::KEYMOD_SHIFT)
        .value("KEYMOD_ALT", key::KEYMOD_ALT)
        .value("KEYMOD_META", key::KEYMOD_META)
        .export_values()
    ;

    scope mainScope;

    scope eventScope = class_<Event, boost::noncopyable>("Event", init<Event::Type,
            Event::Source, optional<int> >())
        .add_property("type", &Event::getType)
        .add_property("source", &Event::getSource)
        .add_property("when", &Event::getWhen)
        .add_property("inputdevice", &Event::getInputDevice)
        .add_property("inputdevicename", make_function(&Event::getInputDeviceName,
                return_value_policy<copy_const_reference>()))
    ;

    enum_<Event::Type>("Type")
        .value("KEY_UP", Event::KEY_UP)
        .value("KEY_DOWN", Event::KEY_DOWN)
        .value("CURSOR_MOTION", Event::CURSOR_MOTION)
        .value("CURSOR_UP", Event::CURSOR_UP)
        .value("CURSOR_DOWN", Event::CURSOR_DOWN)
        .value("CURSOR_OVER", Event::CURSOR_OVER)
        .value("CURSOR_OUT", Event::CURSOR_OUT)
        .value("CUSTOM_EVENT", Event::CUSTOM_EVENT)
        .export_values()
    ;

    enum_<CursorEvent::Source>("Source")
        .value("MOUSE", CursorEvent::MOUSE)
        .value("TOUCH", CursorEvent::TOUCH)
        .value("TRACK", CursorEvent::TRACK)
        .value("CUSTOM", Event::CUSTOM)
        .value("NONE", Event::NONE)
        .export_values()
    ;

    scope oldScope1(mainScope);

    class_<CursorEvent, boost::shared_ptr<CursorEvent>, bases<Event> >("CursorEvent", 
            no_init)
        .add_property("source", &CursorEvent::getSource)
        .add_property("pos", &CursorEvent::getPos)
        .add_property("x", &CursorEvent::getXPosition)
        .add_property("y", &CursorEvent::getYPosition)
        .add_property("cursorid", &CursorEvent::getCursorID, &CursorEvent::setCursorID)
        .add_property("node", &CursorEvent::getNode)
        .add_property("speed", make_function(&CursorEvent::getSpeed,
                return_value_policy<copy_const_reference>()))
        .add_property("contact", &CursorEvent::getContact)
    ;

    class_<KeyEvent, bases<Event> >("KeyEvent", no_init)
        .add_property("scancode", &KeyEvent::getScanCode)
        .add_property("keycode", &KeyEvent::getKeyCode)
        .add_property("keystring", make_function(&KeyEvent::getKeyString, 
                return_value_policy<copy_const_reference>()))
        .add_property("unicode", &KeyEvent::getUnicode)
        .add_property("modifiers", &KeyEvent::getModifiers)
    ;    
    
    class_<MouseEvent, bases<CursorEvent> >("MouseEvent",
            init<Event::Type, bool, bool, bool, const IntPoint&, int,
                 optional<const glm::vec2&, int> >())
        .add_property("leftbuttonstate", &MouseEvent::getLeftButtonState)
        .add_property("middlebuttonstate", &MouseEvent::getMiddleButtonState)
        .add_property("rightbuttonstate", &MouseEvent::getRightButtonState)
        .add_property("button", &MouseEvent::getButton)
    ;

    class_<TouchEvent, bases<CursorEvent> >("TouchEvent", init<int, Event::Type,
            const IntPoint&, Event::Source, optional<const glm::vec2&> >())
        .add_property("area", &TouchEvent::getArea)
        .add_property("orientation", &TouchEvent::getOrientation)
        .add_property("eccentricity", &TouchEvent::getEccentricity)
        .add_property("center", make_function(&TouchEvent::getCenter,
                return_value_policy<copy_const_reference>()))
        .add_property("majoraxis", make_function(&TouchEvent::getMajorAxis,
                return_value_policy<copy_const_reference>()))
        .add_property("minoraxis", make_function(&TouchEvent::getMinorAxis,
                return_value_policy<copy_const_reference>()))
        .add_property("handorientation", &TouchEvent::getHandOrientation)
        .def("getRelatedEvents", &TouchEvent::getRelatedEvents)
        .def("getContour", &TouchEvent::getContour)
        ;

    object contactClass = class_<Contact, boost::shared_ptr<Contact>, bases<Publisher> >
            ("Contact", no_init)
        .add_property("id", &Contact::getID)
        .add_property("age", &Contact::getAge)
        .add_property("distancefromstart", &Contact::getDistanceFromStart)
        .add_property("motionangle", &Contact::getMotionAngle)
        .add_property("motionvec", &Contact::getMotionVec)
        .add_property("distancetravelled", &Contact::getDistanceTravelled)
        .add_property("events", &Contact::getEvents)
        .def("connectListener", &Contact::connectListener)
        .def("disconnectListener", &Contact::disconnectListener)
        ;
    exportMessages(contactClass, "Contact");

    enum_<TrackerImageID>("TrackerImageID")
        .value("IMG_CAMERA", TRACKER_IMG_CAMERA)
        .value("IMG_DISTORTED", TRACKER_IMG_DISTORTED)
        .value("IMG_NOHISTORY", TRACKER_IMG_NOHISTORY)
        .value("IMG_HISTOGRAM", TRACKER_IMG_HISTOGRAM)
        .value("IMG_FINGERS", TRACKER_IMG_FINGERS)
        .value("IMG_HIGHPASS", TRACKER_IMG_HIGHPASS)
        .export_values()
    ;

    class_<IInputDevicePtr>("IInputDevice")
    ;

    class_< IInputDeviceWrapper,
            boost::shared_ptr<IInputDeviceWrapper>,
            boost::noncopyable
    >("InputDevice", init<const std::string&, optional<const DivNodePtr&> >())
        .def("start", &IInputDevice::start, &IInputDeviceWrapper::default_start)
        .def("pollEvents", pure_virtual(&IInputDevice::pollEvents))
        .add_property("name",
                      make_function(&IInputDevice::getName,
                                    return_value_policy<copy_const_reference>()))
        .add_property("eventreceivernode",
                      make_function(&IInputDevice::getEventReceiverNode,
                                    return_value_policy<copy_const_reference>()))
    ;

    class_<TrackerInputDevice, boost::noncopyable>("Tracker", no_init)
        .def("getImage", &TrackerInputDevice::getImage,
            return_value_policy<manage_new_object>())
        .def("getDisplayROIPos", &TrackerInputDevice::getDisplayROIPos)
        .def("getDisplayROISize", &TrackerInputDevice::getDisplayROISize)
        .def("saveConfig", &TrackerInputDevice::saveConfig)
        .def("resetHistory", &TrackerInputDevice::resetHistory)
        .def("setDebugImages", &TrackerInputDevice::setDebugImages)
        .def("startCalibration", &TrackerInputDevice::startCalibration,
            return_value_policy<reference_existing_object>())
        .def("endCalibration", &TrackerInputDevice::endCalibration)
        .def("abortCalibration", &TrackerInputDevice::abortCalibration)
        .def("setParam", &TrackerInputDevice::setParam)
        .def("getParam", &TrackerInputDevice::getParam)
    ;

    class_<TrackerCalibrator, boost::noncopyable>("TrackerCalibrator", no_init)
        .def("nextPoint", &TrackerCalibrator::nextPoint)
        .def("getDisplayPoint", &TrackerCalibrator::getDisplayPoint)
        .def("setCamPoint", &TrackerCalibrator::setCamPoint)
    ;
}
