//
// $Id$
//

void export_raster();
void export_event();
void export_parport();

#include "../base/Logger.h"
#include "../conradrelais/ConradRelais.h"
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

    export_parport();
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

    class_<Player>("Player")
        .def("loadFile", &Player::loadFile)
        .def("play", &Player::play)
        .def("stop", &Player::stop)
        .def("createNode", &Player::createNodeFromXmlString,
                return_value_policy<manage_new_object>())
        .def("setInterval", &Player::setInterval)
        .def("setTimeout", &Player::setTimeout)
        .def("clearInterval", &Player::clearInterval)
        .def("getCurEvent", &Player::getCurEvent,
                return_value_policy<reference_existing_object>())
        .def("screenshot", &Player::screenshot)
        .def("showCursor", &Player::showCursor)
        .def("getElementByID", &Player::getElementByID,
                return_value_policy<reference_existing_object>())
        .def("getRootNode", &Player::getRootNode,
                return_value_policy<reference_existing_object>())
        .def("getFramerate", &Player::getFramerate)
    ;

}
