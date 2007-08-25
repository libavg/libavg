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

void export_bitmap();
void export_raster();
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

#include <boost/python.hpp>
#include <boost/version.hpp>
#include <boost/shared_ptr.hpp>

using namespace boost::python;
using namespace avg;

void export_node()
{
    class_<Node, boost::shared_ptr<Node>, boost::noncopyable>("Node",
            "Base class for all elements in the avg tree.\n",
            no_init)
        .def("getParent", &Node::getParent,
                "getParent() -> Node\n\n"
                "Returns the container (AVGNode or DivNode) the node is in. For\n"
                "the root node, returns None.\n")
        .def("setEventCapture", &Node::setMouseEventCapture,
                "setEventCapture() -> None\n\n")
        .def("setEventCapture", &Node::setEventCapture,
                "setEventCapture(CursorID) -> None\n\n"
                "Sets up event capturing so that all mouse events are sent to this node\n"
                "regardless of the mouse cursor position. If the node doesn't handle the\n"
                "event, it propagates to its parent normally. Useful for the\n"
                "implementation of user interface elements such as scroll bars. Only one\n"
                "node can capture the mouse at any one time. Normal mouse operation can\n"
                "be restored by calling releaseEventCapture().\n")
        .def("releaseEventCapture", &Node::releaseMouseEventCapture,
                "releaseEventCapture() -> None\n\n")
        .def("releaseEventCapture", &Node::releaseEventCapture,
                "releaseEventCapture() -> None\n\n"
                "Restores normal nouse operation after a call to setEventCapture().\n")
        .def("setEventHandler", &Node::setEventHandler,
                "setEventHandler(Type, Source, pyfunc) -> None\n\n"
                "Sets a callback function that is invoked whenever an event of the\n"
                "specified Type from the specified Source occurs. This function is\n"
                "similar to the event handler node attributes (e.g. oncursordown).\n"
                "It is more specific since it takes the event source as a parameter\n"
                "and allows the use of any python callable as callback function.\n")
        .def("getRelXPos", &Node::getRelXPos,
                "getRelXPos(absx) -> relx\n\n"
                "Transforms an x-coordinate in screen coordinates to an x-coordinate\n"
                "in coordinates relative to the node.\n")
        .def("getRelYPos", &Node::getRelYPos,
                "getRelYPos(absy) -> rely\n\n"
                "Transforms an y-coordinate in screen coordinates to an y-coordinate\n"
                "in coordinates relative to the node.\n")
        .add_property("id", make_function(&Node::getID,
                return_value_policy<copy_const_reference>()), &Node::setID,
                "A unique identifier that can be used to reference the node.\n")
        .add_property("x", &Node::getX, &Node::setX,
                "The position of the node's left edge relative to it's parent node.\n")
        .add_property("y", &Node::getY, &Node::setY,
                "The position of the node's top edge relative to it's parent node.\n")
        .add_property("width", &Node::getWidth, &Node::setWidth)
        .add_property("height", &Node::getHeight, &Node::setHeight)
        .add_property("opacity", &Node::getOpacity, &Node::setOpacity,
                      "A measure of the node's transparency. 0.0 is completely\n"
                      "transparent, 1.0 is completely opaque. Opacity is relative to\n"
                      "the parent node's opacity.\n")
        .add_property("active", &Node::getActive, &Node::setActive,
                      "If this attribute is true, the node behaves as usual. If not, it\n"
                      "is neither drawn nor does it react to events. Videos are paused.\n")
        .add_property("sensitive", &Node::getSensitive, &Node::setSensitive,
                      "sensitive: A node only reacts to events if sensitive is true.")
    ;

    export_bitmap();
    export_raster();
    
    class_<DivNode, bases<Node>, boost::noncopyable>("DivNode", 
            "A div node is a node that groups other nodes logically and visually.\n"
            "Its upper left corner is used as point of origin for the coordinates\n"
            "of its child nodes. Its extents are used to clip the children. Its\n"
            "opacity is used as base opacity for the child nodes' opacities.\n",
            no_init)
        .def("getNumChildren", &DivNode::getNumChildren,
                "getNumChildren() -> numChildren\n\n")
        .def("getChild", &DivNode::getChild, 
                "getChild(i) -> Node\n\n"
                "Returns the ith child in z-order.")
        .def("appendChild", &DivNode::appendChild,
                "appendChild(Node) -> None\n\n"
                "Adds a new child to the container behind the last existing child.")
        .def("insertChildBefore", &DivNode::insertChildBefore,
                "insertChildBefore(Node, pos) -> None\n\n"
                "Adds a new child to the container at position pos.")
        .def("removeChild", &DivNode::removeChild,
                "removeChild(i) -> None\n\n"
                "Removes the child at index i.")
        .def("indexOf", &DivNode::indexOf,
                "indexOf(childNode) -> i\n\n"
                "Returns the index of the child given or -1 if childNode isn't a\n"
                "child of the container.")
    ;
    
    class_<AVGNode, bases<DivNode> >("AVGNode",
            "Root node of any avg tree. Defines the properties of the display and\n"
            "handles key press events. The AVGNode's width and height define the\n"
            "coordinate system for the display and are the default for the window\n"
            "size used (i.e. by default, the coordinate system is pixel-based.)\n",
            no_init)
        .def("getCropSetting", &AVGNode::getCropSetting,
                "getCropSetting() -> isCropActive\n\n"
                "Returns true if cropping is active. Cropping can be turned off globally\n"
                "in the avg file. (Deprecated. This attribute is only nessesary because\n"
                "of certain buggy display drivers that don't work with cropping.)")
    ;

    class_<PanoImage, bases<Node> >("PanoImage",
            "A panorama image.\n",
            no_init)
        .def("getScreenPosFromPanoPos", &PanoImage::getScreenPosFromPanoPos,
                "getScreenPosFromPanoPos(panoPos) -> pos\n\n"
                "Converts a position in panorama image pixels to pixels in coordinates\n"
                "relative to the node, taking into account the current rotation angle.\n")
        .def("getScreenPosFromAngle", &PanoImage::getScreenPosFromAngle,
                "getScreenPosFromAngle(angle) -> pos\n\n"
                "Converts panorama angle to pixels in coordinates\n"
                "relative to the node, taking into account the current rotation angle.\n")
        .add_property("href", make_function(&PanoImage::getHRef, 
                return_value_policy<copy_const_reference>()), &PanoImage::setHRef,
                "The source filename of the image.\n")
        .add_property("sensorwidth", &PanoImage::getSensorWidth, 
                &PanoImage::setSensorWidth,
                "The width of the sensor used to make the image. This value\n"
                "is used together with sensorheight and focallength to\n"
                "determine the projection to use.\n")
        .add_property("sensorheight", &PanoImage::getSensorHeight, 
                &PanoImage::setSensorHeight,
                "The height of the sensor used to make the image.\n")
        .add_property("focallength", &PanoImage::getFocalLength, 
                &PanoImage::setFocalLength,
                "The focal length of the lens in millimeters.\n")
        .add_property("hue", &PanoImage::getHue,
                "A hue to color the image in. (ro, deprecated)\n")
        .add_property("saturation", &PanoImage::getSaturation,
                "The saturation the image should have. (ro, deprecated)\n")
        .add_property("rotation", &PanoImage::getRotation, &PanoImage::setRotation,
                "The current angle the viewer is looking at in radians.\n")
        .add_property("maxrotation", &PanoImage::getMaxRotation,
                "The maximum angle the viewer can look at.\n")
    ;

}
