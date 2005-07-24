//
// $Id$
//

void export_raster();
void export_event();
void export_devices();

#include "../base/Logger.h"
#include "../player/Player.h"
#include "../player/AVGNode.h"
#include "../player/DivNode.h"
#include "../player/Excl.h"
#include "../player/PanoImage.h"

#include <boost/python.hpp>

using namespace boost::python;
using namespace avg;

BOOST_PYTHON_MODULE(avg)
{
    class_<Logger>("Logger", no_init)
        .def("get", &Logger::get, 
                return_value_policy<reference_existing_object>())
        .staticmethod("get")
        .def("setDestination", &Logger::setDestination)
        .def("setCategories", &Logger::setCategories)
        .def("trace", &Logger::trace)
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

    export_devices();
    export_event();
    
    class_<Node, boost::noncopyable>("Node", no_init)
        .def("getParent", &Node::getParent,
                return_internal_reference<>())
        .add_property("id", make_function(&Node::getID, 
                return_value_policy<copy_const_reference>()))
        .add_property("x", &Node::getX, &Node::setX)
        .add_property("y", &Node::getY, &Node::setY)
        .add_property("width", &Node::getWidth, &Node::setWidth)
        .add_property("height", &Node::getHeight, &Node::setHeight)
        .add_property("opacity", &Node::getOpacity, &Node::setOpacity)
    ;

    export_raster();
    
    class_<Container, bases<Node>, boost::noncopyable>("Container", no_init)
        .def("getNumChildren", &Container::getNumChildren)
        .def("getChild", &Container::getChild, 
                return_value_policy<reference_existing_object>())
        .def("addChild", &Container::addChild)
        .def("removeChild", &Container::removeChild)
        .def("indexOf", &Container::indexOf)
    ;
    
    class_<DivNode, bases<Container>, boost::noncopyable>("DivNode", no_init)
    ;
    
    class_<AVGNode, bases<DivNode> >("AVGNode")
        .def("getCropSetting", &AVGNode::getCropSetting)
        .add_property("onkeydown", make_function(&AVGNode::getOnKeyDown,
                return_value_policy<copy_const_reference>()))
        .add_property("onkeyup", make_function(&AVGNode::getOnKeyUp,
                return_value_policy<copy_const_reference>()))
    ;

    class_<Excl, bases<Container> >("Excl")
        .add_property("activechild", &Excl::getActiveChild, &Excl::setActiveChild)
    ;

    class_<PanoImage, bases<Node> >("PanoImage")
        .add_property("href", make_function(&PanoImage::getFilename,
                return_value_policy<copy_const_reference>()))
        .add_property("sensorwidth", &PanoImage::getSensorWidth)
        .add_property("sensorheight", &PanoImage::getSensorHeight)
        .add_property("focallength", &PanoImage::getFocalLength)
        .add_property("hue", &PanoImage::getHue)
        .add_property("saturation", &PanoImage::getSaturation)
        .add_property("rotation", &PanoImage::getRotation, &PanoImage::setRotation)
        .add_property("maxrotation", &PanoImage::getMaxRotation)
    ;

    enum_<Player::DisplayEngineType>("DisplayEngineType")
        .value("DFB", Player::DFB)
        .value("OGL", Player::OGL)
        .export_values()
    ;

    class_<Player>("Player", 
                "class Player\n\n"
                "The class used to load and play avg files.")
        .def("setDisplayEngine", &Player::setDisplayEngine,
                "setDisplayEngine(engine) -> None\n\n"
                "Determines which display backend to use. Parameter can be either\n"
                "avg.DFB (for DirectFB rendering) or avg.OGL (for OpenGL rendering).\n"
                "Must be called before loadFile.")
        .def("setResolution", &Player::setResolution,
                "setResolution(fullscreen, width, height, bpp) -> None\n\n",
                "Sets display engine parameters. width and height set the window size\n"
                "(if fullscreen is false) or screen resolution (if fullscreen is true).\n"
                "bpp is the number of bits per pixel to use. Must be called before\n"
                "loadFile.")
        .def("loadFile", &Player::loadFile,
                "loadFile(fileName) -> None\n\n"
                "Loads the avg file specified in fileName. Returns false if the file\n"
                "could not be opened.")
        .def("play", &Player::play,
                "play(framerate) -> None\n\n"
                "Opens a playback window or screen and starts playback. framerate is\n"
                "the number of frames per second that should be displayed. play returns\n"
                "when playback has ended.")
        .def("stop", &Player::stop,
                "stop() -> None\n\n"
                "Stops playback and resets the video mode if nessesary.")
        .def("createNode", &Player::createNodeFromXmlString,
                return_value_policy<manage_new_object>(),
                "createNode(xml) -> Node\n\n"
                "Creates a new Node from an xml string. This node can be used as\n"
                "parameter to Container::addChild().")
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
        .def("screenshot", &Player::screenshot,
                "screenshot(filename) -> ok\n\n"
                "Saves the contents of the current screen in a png file. Returns\n"
                "1 on success, 0 if the screen couldn't be saved.\n")
        .def("showCursor", &Player::showCursor,
                "showCursor(show) -> None\n\n"
                "Shows or hides the mouse cursor. (Currently, this only works for\n"
                "OpenGL. Showing the DirectFB mouse cursor seems to expose some\n"
                "issue with DirectFB.)")
        .def("getElementByID", &Player::getElementByID,
                return_value_policy<reference_existing_object>(),
                "getElementByID(id) -> Node\n\n"
                "Returns an element in the avg tree. The id corresponds to the id\n"
                "attribute of the node.")
        .def("getRootNode", &Player::getRootNode,
                return_value_policy<reference_existing_object>(),
                "getRootNode() -> AVGNode\n\n"
                "Returns the outermost element in the avg tree.")
        .def("getFramerate", &Player::getFramerate,
                "getFramerate() -> framerate\n\n"
                "Returns the current framerate in frames per second.")
    ;

}
