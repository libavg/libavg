//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2008 Ulrich von Zadow
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
void export_anim();

#include "WrapHelper.h"

#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/OSHelper.h"
#include "../player/Player.h"
#include "../player/AVGNode.h"
#include "../player/DivNode.h"
#include "../player/TrackerEventSource.h"
#include "../player/TouchEvent.h"
#include "../player/TestHelper.h"
#include "../player/Scene.h"
#include "../player/OffscreenScene.h"

#include <boost/version.hpp>
#include <boost/shared_ptr.hpp>

using namespace boost::python;
using namespace avg;
using namespace std;

void exception_translator(Exception const & e) 
{
    PyErr_SetString(PyExc_RuntimeError, e.GetStr().c_str());
}

BOOST_PYTHON_MODULE(avg)
{
    docstring_options doc_options(true, false);

    scope().attr("__doc__") =
        "The main libavg module.\n"
        "G{classtree Node}\n\n"
        "G{classtree Bitmap}\n\n"
        "G{classtree Tracker TrackerCalibrator}\n\n"
        "G{classtree Logger}\n\n"
        "G{classtree ConradRelais ParPort}";

#if (BOOST_VERSION / 100000) > 1 || ((BOOST_VERSION / 100) % 1000) >= 33
    register_exception_translator<Exception>(exception_translator);
#endif
    register_ptr_to_python<DivNodePtr>();
    register_ptr_to_python<AVGNodePtr>();
    register_ptr_to_python<EventPtr>();
    register_ptr_to_python<MouseEventPtr>();
    register_ptr_to_python<TouchEventPtr>();

    to_python_converter<IntPoint, Point_to_python_tuple<int> >();
    DPoint_from_python_tuple<DPoint, double>();
    DPoint_from_python_tuple<ConstDPoint, double>();
    DPoint_from_python_tuple<IntPoint, int>();
    
    IntTriple_from_python_tuple();
    
   // IntTriple_from_python_tuple<IntTriple, int>();

    to_python_converter<vector<DPoint>, to_list<vector<DPoint> > >();    
    to_python_converter<vector<string>, to_list<vector<string> > >();    
   
    from_python_sequence<vector<DPoint>, variable_capacity_policy>();
    from_python_sequence<vector<IntPoint>, variable_capacity_policy>();
    from_python_sequence<vector<string>, variable_capacity_policy>();
  
    from_python_sequence<vector<IntTriple>, variable_capacity_policy>();

    def("getMemoryUsage", getMemoryUsage,
            "Returns the amount of memory used by the application in bytes. More\n"
            "precisely, this function returns the resident set size of the process\n"
            "in bytes. This does not include shared libraries or memory paged out to\n"
            "disk.\n");

    class_<Logger>("Logger", 
            "Interface to the logger used by the avg player. Enables the setting\n"
            "of different logging categories. Categories can be set either by calling\n"
            "Logger.setCategories or by setting the AVG_LOG_CATEGORIES environment\n"
            "variable. Default categories are ERROR, WARNING, and APP. Log output\n"
            "is sent to the console (stderr).\n"
            "Each log entry contains the time the message was written, the category\n"
            "of the entry and the message itself.\n",
            no_init)
        .def("get", &Logger::get, 
                return_value_policy<reference_existing_object>(),
                "This method gives access to the logger. There is only one instance.\n")
        .staticmethod("get")
        .def("setCategories", &Logger::setCategories,
                "Sets the types of messages that should be logged.\n" 
                "@param categories: Or'ed list of categories. Possible categories are:\n"
                "    - NONE: No logging except for errors.\n"
                "    - BLTS: Display subsystem logging. Useful for timing/performance"
                "            measurements.\n"
                "    - PROFILE: Outputs performance statistics on player termination.\n"
                "    - PROFILE_LATEFRAMES: Outputs performance statistics whenever a"
                "                          frame is displayed late.\n"
                "    - EVENTS: Outputs basic event data.\n"
                "    - EVENTS2: Outputs all event data available.\n"
                "    - CONFIG: Outputs configuration data.\n"
                "    - WARNING: Outputs warning messages.\n"
                "    - ERROR: Outputs error messages. Can't be shut off.\n"
                "    - MEMORY: Outputs open/close information whenever a media file is\n"
                "              accessed.\n"
                "    - APP: Reserved for application-level messages issued by python\n"
                "           code.\n"
                "    - PLUGIN: Messages generated by loading plugins.\n"
                "    - PLAYER: General libavg playback messages.\n")
        .def("pushCategories", &Logger::pushCategories,
                "Pushes the current set of categories on an internal stack. Useful\n"
                "for saving and restoring the logging state so it can be changed\n"
                "for a short amount of time.\n")
        .def("popCategories", &Logger::popCategories,
                "Pops the current set of categories from the internal stack, restoring\n"
                "the state when the corresponding push was called.\n")
        .def("trace", &Logger::trace,
                "Logs message to the log if category is active.\n"
                "@param category: One of the categories listed for setCategories().\n"
                "Should in most cases be APP.\n"
                "@param message: The log message.\n")
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
        .def_readonly("PLUGIN", &Logger::PLUGIN)
    ;

#ifndef WIN32
    export_devices();
#endif
    export_event();
    export_node();
    export_anim();

    class_<TestHelper>("TestHelper", "Miscelaneous routines used by tests.", no_init)
        .def("fakeMouseEvent", &TestHelper::fakeMouseEvent, "")
        .def("fakeTouchEvent", &TestHelper::fakeTouchEvent, "")
        .def("fakeKeyEvent", &TestHelper::fakeKeyEvent, "")
        .def("dumpObjects", &TestHelper::dumpObjects, "")
    ;

    class_<Player>("Player", 
                "The class used to load and play avg files.")
        .def("get", &Player::get, 
                return_value_policy<reference_existing_object>(),
                "This method gives access to the player, which must have been created.\n"
                "before by calling the constructor.\n")
        .staticmethod("get")
        .def("setResolution", &Player::setResolution,
                "setResolution(fullscreen, width, height, bpp)\n"
                "Sets display engine parameters. Must be called before loadFile or\n"
                "loadString.\n"
                "@param fullscreen: True if the avg file should be rendered fullscreen.\n"
                "@param width, height: Set the window size\n"
                "(if fullscreen is false) or screen resolution (if fullscreen is true).\n"
                "@param bpp: Number of bits per pixel to use.\n")
        .def("setWindowPos", &Player::setWindowPos,
                "setWindowPos(x, y)\n"
                "Sets the location of the player window. Must be called before loadFile\n"
                "or loadString.\n")
        .def("setOGLOptions", &Player::setOGLOptions,
                "setOGLOptions(UsePOW2Textures, UseYCbCrShaders, UsePixelBuffers, MultiSampleSamples)\n"
                "Determines which OpenGL extensions to check for and use if possible.\n"
                "Mainly used for debugging purposes while developing libavg, but can\n"
                "also be used to work around buggy drivers. The values set here\n"
                "override those in avgrc. Note that with the exception of\n"
                "MultiSampleSamples, fallbacks are always used - if a feature is\n"
                "specified that the system doesn't support, a less demanding one will\n"
                "be used.\n"
                "@param UsePOW2Textures: If True, restricts textures to power-of-two\n"
                "dimensions.\n"
                "@param UseYCbCrShaders: If True, shaders are used to copy YCbCr\n"
                "to the screen. Results in a major video playback performance boost.\n"
                "@param UsePixelBuffers: If False, disables the use of OpenGL pixel\n"
                "buffer objects.\n"
                "@param MultiSampleSamples: The number of samples per pixel to compute.\n"
                "This costs performance and smoothes the edges of polygons. A value of\n"
                "1 turns multisampling (also knowna as FSAA - Full-Screen Antialiasing)\n"
                "off. Good values are dependent on the graphics driver.\n")
        .def("setMultiSampleSamples", &Player::setMultiSampleSamples,
                "setMultiSampleSamples(multiSampleSamples)\n"
                "@param multiSampleSamples: The number of samples per pixel to compute.\n"
                "This costs performance and smoothes the edges of polygons. A value of\n"
                "1 turns multisampling (also knowna as FSAA - Full-Screen Antialiasing)\n"
                "off. Good values are dependent on the graphics driver.\n")
        .def("enableAudio", &Player::enableAudio,
                "enableAudio(bEnable)\n"
                "Enables or disables audio playback. If audio playback is disabled, no\n"
                "nodes with sound can be created. Mainly used to speed up the test\n"
                "suite.\n")
        .def("loadFile", &Player::loadFile,
                "loadFile(filename) -> scene\n"
                "Loads the avg file specified in filename. Returns the scene loaded."
                "The scene is the main scene displayed onscreen."
                "@param filename: ")
        .def("loadString", &Player::loadString,
                "loadString(avgString) -> scene\n"
                "Parses avgString and loads the nodes it contains. Returns the scene"
                "loaded. The scene is the main scene displayed onscreen."
                "@param avgString: An xml string containing an avg node hierarchy.")
        .def("loadSceneFile", &Player::loadSceneFile,
                "loadSceneFile(filename)\n"
                "Loads the scene file specified in filename and adds it to the\n"
                "registered offscreen scenes.\n"
                "@param filename: ")
        .def("loadSceneString", &Player::loadSceneString,
                "loadSceneString(avgString)\n"
                "Parses avgString, loads the nodes it contains and adds the hierarchy\n"
                "to the registered offscreen scenes.\n"
                "@param filename: ")
        .def("deleteScene", &Player::deleteScene,
                "deleteScene(id)\n"
                "Removes the scene given by id from the player's internal list of"
                "scenes. If the scene is not referenced by a node, it is deleted."
                "If it is referenced, it is deleted once the reference is gone and not"
                "immediately.")
        .def("getMainScene", &Player::getMainScene,
                "getMainScene() -> scene\n"
                "Returns the main scene. This is the scene loaded using loadFile or"
                "loadString and displayed on screen.")
        .def("getScene", &Player::getScene,
                "getScene(id) -> scene\n"
                "Returns a reference to an offscreen scene - one loaded using"
                "loadSceneXxx")
        .def("play", &Player::play,
                "play()\n"
                "Opens a playback window or screen and starts playback. play returns\n"
                "when playback has ended.\n")
        .def("stop", &Player::stop,
                "stop()\n"
                "Stops playback and resets the video mode if necessary.\n")
        .def("isPlaying", &Player::isPlaying,
                "isPlaying() -> bool\n"
                "Returns True if play() is currently executing, False if not.\n")
        .def("setFramerate", &Player::setFramerate,
                "setFramerate(framerate)\n"
                "Sets the desired framerate for playback. Turns off syncronization\n"
                "to the vertical blanking interval.\n"
                "@param framerate: ")
        .def("setVBlankFramerate", &Player::setVBlankFramerate,
                "setVBlankFramerate(rate)\n"
                "Sets the desired number of monitor refreshes before the next\n"
                "frame is displayed. The resulting framerate is determined by the\n"
                "monitor refresh rate divided by the rate parameter.\n"
                "@param rate: Number of vertical blanking intervals to wait.\n")
        .def("getEffectiveFramerate", &Player::getEffectiveFramerate,
                "getEffectiveFramerate() -> framerate\n"
                "Returns the framerate that the player is actually achieving. The\n"
                "value returned is not averaged and reflects only the current frame.\n")
        .def("getTestHelper", &Player::getTestHelper,
                return_value_policy<reference_existing_object>(),
                "")
        .def("setFakeFPS", &Player::setFakeFPS,
                "setFakeFPS(fps)\n"
                "Sets a fixed number of virtual frames per second that are used as\n"
                "clock source for video playback, animations and other time-based\n"
                "actions. If a value of -1 is given as parameter, the real clock is\n"
                "used. FakeFPS can be used to get reproducible effects for recordings\n"
                "or automated tests. Setting FakeFPS has the side-effect of disabling\n"
                "audio.\n"
                "@param fps: \n")
        .def("getFrameTime", &Player::getFrameTime,
                "getFrameTime() -> time\n"
                "Returns the number of milliseconds that have elapsed since playback\n"
                "has started. Honors FakeFPS. The time returned stays constant for an\n"
                "entire frame; it is the time of the last display update.\n")
        .def("createNode", &Player::createNodeFromXmlString,
                "createNode(xml) -> node\n"
                "Creates a new Node. This node can be used as\n"
                "parameter to DivNode::appendChild() and insertChild().\n"
                "This method will create any type of node, including <div> nodes\n"
                "with children.\n"
                "@param xml: xml string in avg syntax that specifies the node to create.")
        .def("createNode", &Player::createNode,
                "createNode(type, args) -> node\n"
                "Creates a new Node. This node can be used as\n"
                "parameter to DivNode::appendChild() and insertChild().\n"
                "This method will only create one node at a time.\n"
                "@param type: type string of the node to create.\n"
                "@param args: a dictionary specifying attributes of the node.")
        .def("addTracker", &Player::addTracker,
                return_value_policy<reference_existing_object>(),
                "addTracker()\n"
                "Adds a camera-based tracker to the avg player. The tracker can be\n"
                "configured using the default config file and immediately starts\n"
                "reporting events.")
        .def("getTracker", &Player::getTracker,
                return_value_policy<reference_existing_object>(),
                "getTracker()\n"
                "returns a tracker previously created with addTracker.")
        .def("setInterval", &Player::setInterval,
                "setInterval(time, pyfunc) -> id\n"
                "Sets a python callable object that should be executed regularly.\n"
                "setInterval returns an id that can be used to\n"
                "call clearInterval() to stop the function from being called. The\n"
                "callback is called at most once per frame.\n"
                "@param time: Number of milliseconds between two calls.\n"
                "@param pyfunc: Python callable to execute.\n")
        .def("setTimeout", &Player::setTimeout, 
                "setTimeout(time, pyfunc) -> id\n"
                "Sets a python callable object that should be executed after a set\n"
                "amount of time. setTimeout returns an id that can be used to\n"
                "call clearInterval() to stop the function from being called.\n"
                "@param time: Number of milliseconds before the call.\n"
                "@param pyfunc: Python callable to execute.\n")
        .def("setOnFrameHandler", &Player::setOnFrameHandler,
                "setOnFrameHandler(pyfunc) -> id\n"
                "Sets a python callable object that should be executed once per frame.\n"
                "Returns an id that can be used to call clearInterval() to stop the\n"
                "function from being called.\n"
                "@param pyfunc: Python callable to execute.\n")
        .def("clearInterval", &Player::clearInterval,
                "clearInterval(id) -> ok\n"
                "Stops a timeout, an interval or an onFrameHandler from being called.\n"
                "Returns True if there was an interval with the given id, False if not.\n"
                "@param id: An id returned by setInterval, setTimeout or\n"
                "setOnFrameHandler.\n")
        .def("getMouseState", &Player::getMouseState,
                "getMouseState() -> event\n"
                "Returns an interface to the last mouse event.\n")
        .def("getKeyModifierState", &Player::getKeyModifierState,
                "getKeyModifierState() -> KeyModifier\n"
                "Returns the current modifier keys (shift, ctrl) pressed. The return\n"
                "value is several KeyModifier values or'ed together.\n")
        .def("screenshot", &Player::screenshot,
                "screenshot() -> bitmap\n"
                "Returns the contents of the current screen as a bitmap.\n")
        .def("stopOnEscape", &Player::setStopOnEscape,
                "stopOnEscape(stop)\n"
                "Toggles player stop upon escape keystroke.\n"
                "@param stop: True if player should stop on escape\n")
        .def("showCursor", &Player::showCursor,
                "showCursor(show)\n"
                "Shows or hides the mouse cursor.\n"
                "@param show: True if the mouse cursor should be visible.\n")
        .def("setCursor", &Player::setCursor,
                "setCursor(bitmap, hotspot)\n"
                "Sets the mouse cursor to the bitmap given. The bitmap must have a size\n"
                "divisible by 8 and an RGBA pixel format. The cursor generated is\n"
                "binary black and white with a binary transparency channel. hotspot is\n"
                "the relative position of the actual pointing coordinate in the\n"
                "bitmap.\n")
        .def("getElementByID", &Player::getElementByID,
                "getElementByID(id) -> node\n"
                "Returns an element in the avg tree.\n"
                "@param id: id attribute of the node to return.\n")
        .def("getRootNode", &Player::getRootNode,
                "getRootNode() -> node\n"
                "Returns the outermost element in the avg tree.\n")
        .def("getFramerate", &Player::getFramerate,
                "getFramerate() -> rate\n"
                "Returns the current target framerate in frames per second.\n")
        .def("getVideoRefreshRate", &Player::getVideoRefreshRate,
                "getVideoRefreshRate() -> rate\n"
                "Returns the current hardware video refresh rate in number of\n"
                "refreshes per second.\n")
        .def("setGamma", &Player::setGamma,
                "setGamma(red, green, blue)\n"
                "Sets display gamma. This is a control for overall brightness and\n"
                "contrast that leaves black and white unchanged but adjusts greyscale\n"
                "values. 1.0 is identity, higher values give a brighter image, lower\n"
                "values a darker one.\n"
                "@param red, green, blue: \n")
        .def("setMousePos", &Player::setMousePos,
                "setMousePos(pos)\n"
                "Sets the position of the mouse cursor. Generates a mouse motion event.\n"
                "@param pos: new coordinates as a Point2D.\n")
        .def("loadPlugin", &Player::loadPlugin,
                "loadPlugin(name)\n"
                "load a Plugin and extend the XML DTD.\n"
                "@param name: name of the plugin (without directory and\n"
                "file extension)\n")
        .def("setEventHook", &Player::setEventHook,
                "setEventHook(pyfunc)\n"
                "set a callable which will receive all events passing\n"
                "through Player events' sink. If the function returns True,\n"
                "the event is not propagated to the underlying listeners.\n"
                "@param pyfunc: a python callable\n")
        .add_property("pluginPath", &Player::getPluginPath, &Player::setPluginPath,
                "A colon-separated list of directories where the player\n"
                "searches for plugins when loadPlugin() is called.\n")
        .add_property("volume", &Player::getVolume, &Player::setVolume,
                "Total audio playback volume. 0 is silence, 1 passes media file\n"
                "volume through unchanged. Values higher than 1 can be used to\n"
                "amplify playback. A limiter prevents distortion when the volume\n"
                "is set to high.\n")
    ;

    class_<Scene, boost::shared_ptr<Scene>, boost::noncopyable>("Scene", 
                "A Scene is a tree of nodes. It corresponds to a scenegraph. In a libavg"
                "session, there is one main scene that gets rendered to the screen and"
                "zero or more scenes that are rendered offscreen.",
                no_init)
        .def(self == self)
        .def(self != self)
        .def("__hash__", &Scene::getHash)
        .def("getRootNode", &Scene::getRootNode,
                "getRootNode() -> node\n"
                "Returns the root of the scenegraph. For the main scene, this is an <avg>"
                "node. For an offscreen scene, this is a <scene> node.")
        .def("getElementByID", &Scene::getElementByID,
                "getElementByID(id) -> node\n"
                "Returns an element in the scene's tree."
                "@param id: id attribute of the node to return.")
/*
        .def("screenshot", &Scene::screenshot,
               "getImage() -> bitmap\n"
               "Returns the image the scene has last rendered. For the main scene, this"
               "is a real screenshot. For offscreen scenes, this is the image rendered"
               "offscreen.")
*/        
    ;

    class_<OffscreenScene, boost::shared_ptr<OffscreenScene>, bases<Scene>,
            boost::noncopyable>("OffscreenScene",
                "An OffscreenScene is a Scene that is rendered to a texture. It can be"
                "referenced in the href attribute of an image node.",
                no_init)
        .def("getID", &OffscreenScene::getID,
                "getID() -> id"
                "Returns the id of the scene. This is the same as"
                "calling scene.getRootNode().getID().")
    ;
}
