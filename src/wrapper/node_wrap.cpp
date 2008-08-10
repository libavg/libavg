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

#include "WrapHelper.h"

#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../player/Player.h"
#include "../player/AVGNode.h"
#include "../player/DivNode.h"
#include "../player/PanoImage.h"
#include "../player/Sound.h"

#include <boost/version.hpp>
#include <boost/shared_ptr.hpp>

using namespace boost::python;
using namespace avg;

void export_node()
{
    class_<Node, boost::shared_ptr<Node>, boost::noncopyable>("Node",
            "Base class for all elements in the avg tree.",
            no_init)
        .def(self == self)
        .def(self != self)
        .def("__hash__", &Node::getHash)
        .def("getParent", &Node::getParent,
                "getParent() -> node\n"
                "Returns the container (AVGNode or DivNode) the node is in. For\n"
                "the root node, returns None.\n")
        .def("unlink", &Node::unlink,
                "unlink() -> None\n"
                "Removes a node from it's parent container. Equivalent to\n"
                "node.getParent().removeChild(node.getParent().indexOf(node)).")
        .def("setEventCapture", &Node::setMouseEventCapture,
                "setEventCapture(cursorid)\n"
                "Sets up event capturing so that cursor events are sent to this node\n"
                "regardless of the cursor position. cursorid is optional; if left out,\n"
                "the mouse cursor is captured. If not, events from a specific tracker\n"
                "cursor are captured. If the node doesn't handle the event, it\n"
                "propagates to its parent normally. This function is useful for the\n"
                "implementation of user interface elements such as scroll bars. Only one\n"
                "node can capture a cursor at any one time. Normal operation can\n"
                "be restored by calling releaseEventCapture().\n"
                "@param cursorid: The id of the tracker cursor to capture (optional).\n")
        .def("setEventCapture", &Node::setEventCapture)
        .def("releaseEventCapture", &Node::releaseMouseEventCapture,
                "releaseEventCapture(cursorid)\n"
                "Restores normal mouse operation after a call to setEventCapture().\n"
                "@param cursorid: The id of the tracker cursor to release (optional).\n")
        .def("releaseEventCapture", &Node::releaseEventCapture)
        .def("setEventHandler", &Node::setEventHandler,
                "setEventHandler(type, source, pyfunc)\n"
                "Sets a callback function that is invoked whenever an event of the\n"
                "specified type from the specified source occurs. This function is\n"
                "similar to the event handler node attributes (e.g. oncursordown).\n"
                "It is more specific since it takes the event source as a parameter\n"
                "and allows the use of any python callable as callback function.\n"
                "@param type: One of the event types KEYUP, KEYDOWN, CURSORMOTION,\n"
                "CURSORUP, CURSORDOWN, CURSOROVER, CURSOROUT, RESIZE or QUIT.\n"
                "@param source: MOUSE for mouse events, TOUCH for multitouch touch\n"
                "events, TRACK for multitouch track events or other tracking, \n"
                "NONE for keyboard events.\n"
                "@param pyfunc: The python callable to invoke.\n")
        .def("getAbsPos", &Node::getAbsPos,
                "getAbsPos(relpos) -> abspos\n"
                "Transforms a position in coordinates relative to the node to a\n"
                "position in window coordinates.\n"
                "@param relpos: Relative coordinate to transform.")
        .def("getRelPos", &Node::getRelPos,
                "getRelPos(abspos) -> relpos\n"
                "Transforms a position in window coordinates to a position\n"
                "in coordinates relative to the node.\n"
                "@param abspos: Absolute coordinate to transform.")
        .add_property("id", make_function(&Node::getID,
                return_value_policy<copy_const_reference>()), &Node::setID,
                "A unique identifier that can be used to reference the node.\n")
        .add_property("x", &Node::getX, &Node::setX,
                "The position of the node's left edge relative to it's parent node.\n")
        .add_property("y", &Node::getY, &Node::setY,
                "The position of the node's top edge relative to it's parent node.\n")
        .add_property("width", &Node::getWidth, &Node::setWidth)
        .add_property("height", &Node::getHeight, &Node::setHeight)
        .add_property("angle", &Node::getAngle, &Node::setAngle,
                "The angle that the node is rotated to in radians. 0 is\n"
                "unchanged, 3.14 is upside-down.\n")
        .add_property("pivotx", &Node::getPivotX, &Node::setPivotX,
                "x coordinate of the point that the node is rotated around.\n"
                "Default is the center of the node.\n")
        .add_property("pivoty", &Node::getPivotY, &Node::setPivotY,
                "y coordinate of the point that the node is rotated around.\n"
                "Default is the center of the node.\n")
        .add_property("opacity", &Node::getOpacity, &Node::setOpacity,
                      "A measure of the node's transparency. 0.0 is completely\n"
                      "transparent, 1.0 is completely opaque. Opacity is relative to\n"
                      "the parent node's opacity.\n")
        .add_property("active", &Node::getActive, &Node::setActive,
                      "If this attribute is true, the node behaves as usual. If not, it\n"
                      "is neither drawn nor does it react to events. Videos are paused.\n")
        .add_property("sensitive", &Node::getSensitive, &Node::setSensitive,
                      "A node only reacts to events if sensitive is true.")
    ;

    export_bitmap();
    export_raster();
   
    class_<DivNode, bases<Node>, boost::noncopyable>("DivNode", 
            "A div node is a node that groups other nodes logically and visually.\n"
            "Its upper left corner is used as point of origin for the coordinates\n"
            "of its child nodes. Its extents are used to clip the children. Its\n"
            "opacity is used as base opacity for the child nodes' opacities.\n"
            "The children of a div node are drawn in the order they are found\n"
            "in the avg file.",
            no_init)
        .add_property("mediadir", make_function(&DivNode::getMediaDir,
                return_value_policy<copy_const_reference>()), &DivNode::setMediaDir,
                "The directory that the media files for the children of this node are in.\n")
        .def("getNumChildren", &DivNode::getNumChildren,
                "getNumChildren() -> numchildren\n"
                "Returns the number of immediate children that this div contains.")
        .def("getChild", &DivNode::getChild, 
                "getChild(pos) -> node\n"
                "Returns the child at position pos.")
        .def("appendChild", &DivNode::appendChild,
                "appendChild(node)\n"
                "Adds a new child to the container behind the last existing child.")
        .def("insertChildBefore", &DivNode::insertChildBefore,
                "insertChildBefore(newNode, oldChild)\n"
                "Adds a new child to the container in front of the existing node oldChild.")
        .def("insertChild", &DivNode::insertChild,
                "insertChild(node, pos)\n"
                "Adds a new child to the container at position pos.")
        .def("removeChild", (void (DivNode::*)(NodePtr))(&DivNode::removeChild),
                "removeChild(node)\n"
                "Removes the child given by pNode.")
        .def("removeChild", (void (DivNode::*)(unsigned))(&DivNode::removeChild),
                "removeChild(pos)\n"
                "Removes the child at index pos.")
        .def("reorderChild", (void (DivNode::*)(unsigned, unsigned))(&DivNode::reorderChild),
                "reorderChild(oldPos, newPos)\n"
                "Moves the child at index pos so it's at index newPos. This function\n"
                "can be used to change the order in which the children are drawn.")
        .def("reorderChild", (void (DivNode::*)(NodePtr, unsigned))(&DivNode::reorderChild),
                "reorderChild(node, newPos)\n"
                "Moves the child node so it's at index newPos. This function\n"
                "can be used to change the order in which the children are drawn.")
        .def("indexOf", &DivNode::indexOf,
                "indexOf(childnode)\n"
                "Returns the index of the child given or -1 if childnode isn't a\n"
                "child of the container. This function does a linear search through\n"
                "the list of children until the child is found.")
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

    class_<Sound, bases<Node> >("Sound",
            "A sound played from a file.\n",
            no_init)
        .def("play", &Sound::play,
                "play()\n"
                "Starts audio playback.")
        .def("stop", &Sound::stop,
                "stop()\n"
                "Stops audio playback. Closes the object and 'rewinds' the playback\n"
                "cursor.")
        .def("pause", &Sound::pause,
                "pause()\n"
                "Stops audio playback but doesn't close the object. The playback\n"
                "cursor stays at the same position.")
        .def("setEOFCallback", &Sound::setEOFCallback,
                "setEOFCallback(pyfunc)\n"
                "Sets a python callable to be invoked when the video reaches end of file.")
        .add_property("href", make_function(&Sound::getHRef, 
                return_value_policy<copy_const_reference>()), &Sound::setHRef,
                "The source filename of the sound.\n")
        .add_property("loop", &Sound::getLoop,
                "Whether to start the sound again when it has ended (ro).\n")
    ;
    class_<PanoImage, bases<Node> >("PanoImage",
            "A panorama image displayed in cylindrical projection.\n",
            no_init)
        .def("getScreenPosFromPanoPos", &PanoImage::getScreenPosFromPanoPos,
                "getScreenPosFromPanoPos(panoPos) -> pos\n"
                "Converts a position in panorama image pixels to pixels in coordinates\n"
                "relative to the node, taking into account the current rotation angle.\n")
        .def("getScreenPosFromAngle", &PanoImage::getScreenPosFromAngle,
                "getScreenPosFromAngle(angle) -> pos\n"
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
