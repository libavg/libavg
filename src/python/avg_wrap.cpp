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

void export_node();
void export_event();
#ifndef WIN32
void export_devices();
#endif

#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../player/Player.h"
#include "../player/AVGNode.h"
#include "../player/DivNode.h"
#include "../player/PanoImage.h"
#include "../player/TrackerEventSource.h"

#include <boost/python.hpp>
#include <boost/version.hpp>
#include <boost/shared_ptr.hpp>

using namespace boost::python;
using namespace avg;

void exception_translator(Exception const & e) 
{
    PyErr_SetString(PyExc_RuntimeError, e.GetStr().c_str());
}

BOOST_PYTHON_MODULE(avg)
{
#if (BOOST_VERSION / 100000) > 1 || ((BOOST_VERSION / 100) % 1000) >= 33
    register_exception_translator<Exception>(exception_translator);
#endif
    register_ptr_to_python< DivNodePtr >();
    register_ptr_to_python< AVGNodePtr >();

    class_<Logger>("Logger", 
            "Interface to the logger used by the avg player. Enables the setting\n"
            "of different logging categories and a destination file. Can be used\n"
            "to log application-specific messages as well.\n"
            "Each log entry contains the time the message was written, the category\n"
            "of the entry and the message itself.",
            no_init)
        .def("get", &Logger::get, 
                return_value_policy<reference_existing_object>(),
                "get() -> Logger\n\n"
                "There is only one logger. This method gives access to it.")
        .staticmethod("get")
        .def("setConsoleDest", &Logger::setConsoleDest,
                "setConsoleDest() -> None\n\n"
                "Sets the log to be output to the console (stderr, to be precise).\n"
                "This is the default.")
        .def("setFileDest", &Logger::setFileDest,
                "setFileDest(filename) -> None\n\n"
                "Sets a file that the log should be written to. If opening the file\n"
                "fails, the console is used instead.")
        .def("setSyslogDest", &Logger::setSyslogDest,
                "setSyslogDest(facility, logopt) -> None\n\n"
                "Causes log output to be written to the unix system log facility.\n"
                "facility and logopt are passed to the system log verbatim. See\n"
                "man 3 syslog for details of these parameters. Ident is set to\n"
                "'libavg'")
        .def("setCategories", &Logger::setCategories,
                "setCategories(categories) -> None\n\n"
                "Sets the types of messages that should be logged. Possible \n"
                "categories are:\n"
                "    NONE: No logging except for errors.\n"
                "    BLTS: Display subsystem logging. Useful for timing/performance\n"
                "          measurements.\n"
                "    PROFILE: Outputs performance statistics on player termination.\n"
                "    PROFILE_LATEFRAMES: Outputs performance statistics whenever a\n"
                "                        frame is displayed late.\n"
                "    EVENTS: Outputs basic event data.\n"
                "    EVENTS2: Outputs all event data available.\n"
                "    CONFIG: Outputs configuration data.\n"
                "    WARNING: Outputs warning messages. Default is on.\n"
                "    ERROR: Outputs error messages. Can't be shut off.\n"
                "    MEMORY: Currently unused.\n"
                "    APP: Reserved for application-level messages issued by python\n"
                "         code.\n"
                "Categories can be or'ed together.")
        .def("trace", &Logger::trace,
                "trace(category, message) -> None\n\n"
                "Logs message to the log if category is active. The category should\n"
                "in most cases be APP.")
        .def_readonly("NONE", &Logger::NONE)
        .def_readonly("BLTS", &Logger::BLTS)
        .def_readonly("PROFILE", &Logger::PROFILE)
        .def_readonly("PROFILE_LATEFRAMES", &Logger::PROFILE_LATEFRAMES)
        .def_readonly("EVENTS", &Logger::EVENTS)
        .def_readonly("EVENTS2", &Logger::EVENTS2)
        .def_readonly("CONFIG", &Logger::CONFIG)
        .def_readonly("WARNING", &Logger::WARNING)
        .def_readonly("ERROR", &Logger::ERROR)
        .def_readonly("MEMORY", &Logger::MEMORY)
        .def_readonly("APP", &Logger::APP)
    ;

#ifndef WIN32
     export_devices();
#endif
    export_event();
    export_node();

    enum_<Player::DisplayEngineType>("DisplayEngineType")
        .value("DFB", Player::DFB)
        .value("OGL", Player::OGL)
        .export_values()
    ;

    enum_<YCbCrMode>("YCbCrMode")
        .value("shader", OGL_SHADER)
        .value("mesa", OGL_MESA)
        .value("apple", OGL_APPLE)
        .value("none", OGL_NONE)
        .export_values()
    ;

    class_<TestHelper>("TestHelper", "Miscelaneous routines used by tests.", no_init)
        .def("getNumDifferentPixels", &TestHelper::getNumDifferentPixels, "")
        .def("useFakeCamera", &TestHelper::useFakeCamera, "")
        .def("fakeMouseEvent", &TestHelper::fakeMouseEvent, "")
    ;

    class_<Player>("Player", 
                "The class used to load and play avg files.")
        .def("setDisplayEngine", &Player::setDisplayEngine,
                "setDisplayEngine(engine) -> None\n\n"
                "Determines which display backend to use. Parameter can be either\n"
                "avg.DFB (for DirectFB rendering) or avg.OGL (for OpenGL rendering).\n"
                "Must be called before loadFile.")
        .def("setResolution", &Player::setResolution,
                "setResolution(fullscreen, width, height, bpp) -> None\n\n"
                "Sets display engine parameters. width and height set the window size\n"
                "(if fullscreen is false) or screen resolution (if fullscreen is true).\n"
                "bpp is the number of bits per pixel to use. Must be called before\n"
                "loadFile.")
        .def("setOGLOptions", &Player::setOGLOptions,
                "setOGLOptions(UsePOW2Textures, YCbCrMode, UseRGBOrder, UsePixelBuffers, MultiSampleSamples)\n"
                "       -> None\n\n"
                "Determines which OpenGL extensions to check for and use if possible.\n"
                "Mainly used for debugging purposes while developing libavg, but can\n"
                "also be used to work around buggy drivers.\n"
                "UsePOW2Textures=true restricts textures to power-of-two dimensions.\n"
                "YCbCrMode can be shader, mesa, apple or none and selects the preferred\n"
                "method of copying video textures to the screen.\n"
                "UseRGBOrder=true swaps the order of red and blue components in textures.\n"
                "UsePixelBuffers=false disables the use of OpenGL pixel buffer objects.\n"
                "MultiSampleSamples is the number of samples per pixel to compute. This\n"
                "costs performance and smoothes the edges of polygons. A value of 1 turns\n"
                "multisampling off. Good values are dependent on the graphics driver.")
        .def("loadFile", &Player::loadFile,
                "loadFile(fileName) -> None\n\n"
                "Loads the avg file specified in fileName.")
        .def("play", &Player::play,
                "play() -> None\n\n"
                "Opens a playback window or screen and starts playback. framerate is\n"
                "the number of frames per second that should be displayed. play returns\n"
                "when playback has ended.")
        .def("stop", &Player::stop,
                "stop() -> None\n\n"
                "Stops playback and resets the video mode if nessesary.")
        .def("isPlaying", &Player::isPlaying,
                "isPlaying() -> bool\n\n"
                "Returns true if play() is currently executing, false if not.")
        .def("setFramerate", &Player::setFramerate,
                "setFramerate(rate) -> None\n\n"
                "Sets the desired framerate for playback and turns off syncronization\n"
                "to the vertical blanking interval.")
        .def("setVBlankFramerate", &Player::setVBlankFramerate,
                "setVBlankFramerate(rate) -> bool\n\n"
                "Sets the desired number of vertical blanking intervals before the next\n"
                "frame is displayed. The resulting framerate is determined by the\n"
                "monitor refresh rate divided by the rate parameter.")
        .def("getTestHelper", &Player::getTestHelper,
                return_value_policy<reference_existing_object>(),
                "")
        .def("createNode", &Player::createNodeFromXmlString,
                "createNode(xml) -> Node\n\n"
                "Creates a new Node from an xml string. This node can be used as\n"
                "parameter to DivNode::addChild().")
        .def("addTracker", &Player::addTracker,
                return_value_policy<reference_existing_object>(),
                "addTracker(device, framerate, mode) -> tracker\n\n"
                "Adds a tracker to the avg player. The tracker immediately starts\n"
                "reporting events.")
        .def("setInterval", &Player::setInterval,
                "setInterval(time, pyfunc) -> id\n\n"
                "Sets a python callable object that should be executed every time\n"
                "milliseconds. setInterval returns an id that can be used to\n"
                "call clearInterval() to stop the code from being called.")
        .def("setTimeout", &Player::setTimeout, 
                "setTimeout(time, pyfunc) -> id\n\n"
                "Sets a python callable object that should be executed after time\n"
                "milliseconds. setTimeout returns an id that can be used to\n"
                "call clearInterval() to stop the code from being called.")
        .def("clearInterval", &Player::clearInterval,
                "clearInterval(id) -> ok\n\n"
                "Stops a timeout or an interval from being called. Returns 1 if\n"
                "there was an interval with the given id, 0 if not.\n")
        .def("getCurEvent", &Player::getCurEvent,
                return_value_policy<reference_existing_object>(),
                "getCurEvent() -> Event\n\n"
                "Gets an interface to the current event. Only valid inside event\n"
                "handlers (onmouseup, onmousedown, etc.)")
        .def("getMouseState", &Player::getMouseState,
                return_value_policy<reference_existing_object>(),
                "getMouseState() -> Event\n\n"
                "Gets an interface to the last mouse event.")
        .def("screenshot", &Player::screenshot,
                return_value_policy<manage_new_object>(),
                "screenshot() -> Bitmap\n\n"
                "Returns the contents of the current screen as a bitmap.\n")
        .def("showCursor", &Player::showCursor,
                "showCursor(show) -> None\n\n"
                "Shows or hides the mouse cursor. (Currently, this only works for\n"
                "OpenGL. Showing the DirectFB mouse cursor seems to expose some\n"
                "issue with DirectFB.)")
        .def("getElementByID", &Player::getElementByID,
                "getElementByID(id) -> Node\n\n"
                "Returns an element in the avg tree. The id corresponds to the id\n"
                "attribute of the node.")
        .def("getRootNode", &Player::getRootNode,
                "getRootNode() -> AVGNode\n\n"
                "Returns the outermost element in the avg tree.")
        .def("getFramerate", &Player::getFramerate,
                "getFramerate() -> framerate\n\n"
                "Returns the current framerate in frames per second.")
        .def("getVideoRefreshRate", &Player::getVideoRefreshRate,
                "getVideoRefreshRate() -> refreshrate\n\n"
                "Returns the current hardware video refresh rate in number of\n"
                "refreshes per second.")
        .def("setGamma", &Player::setGamma,
                "setGamma(red, green, blue) -> None\n\n"
                "Sets display gamma. This is a control for overall brightness and\n"
                "contrast that leaves black and white unchanged but adjusts greyscale\n"
                "values. 1.0 is identity, higher values give a brighter image, lower\n"
                "values a darker one.\n")
    ;

}
