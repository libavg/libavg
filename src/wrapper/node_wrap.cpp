//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2020 Ulrich von Zadow
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
void export_fx();
void export_raster();
void export_vector();
void export_event();

#include "WrapHelper.h"
#include "raw_constructor.hpp"

#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../player/Player.h"
#include "../player/AVGNode.h"
#include "../player/CanvasNode.h"
#include "../player/DivNode.h"
#include "../player/SoundNode.h"

#include <boost/version.hpp>
#include <boost/shared_ptr.hpp>
#include <string>

using namespace boost::python;
using namespace avg;
using namespace std;

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(unlink_overloads, Node::unlink, 0, 1);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(disconnectEventHandler_overloads, 
        Node::disconnectEventHandler, 1, 2);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(dumpNodeTree_overloads, DivNode::dump, 0, 1);

ConstVec2 AreaNode_getMediaSize(AreaNode* This)
{
    return (glm::vec2)(This->getMediaSize());
}

char divNodeName[] = "div";
char avgNodeName[] = "avg";
char soundNodeName[] = "sound";

void export_node()
{
    object nodeClass = class_<Node, bases<Publisher>, boost::noncopyable>("Node", no_init)
        .add_property("id", make_function(&Node::getID,
                return_value_policy<copy_const_reference>()), &Node::setID)
        .def("registerInstance", &Node::registerInstance)
        .def("getParent", &Node::getParent)
        .def("unlink", &Node::unlink, unlink_overloads(args("bKill")))
        .def("setEventCapture", &Node::setMouseEventCapture)
        .def("setEventCapture", &Node::setEventCapture)
        .def("releaseEventCapture", &Node::releaseMouseEventCapture)
        .def("releaseEventCapture", &Node::releaseEventCapture)
        .def("setEventHandler", &Node::setEventHandler)
        .def("connectEventHandler", &Node::connectEventHandler)
        .def("disconnectEventHandler", &Node::disconnectEventHandler,
                disconnectEventHandler_overloads(args("pFunc")))
        .def("getAbsPos", &Node::getAbsPos)
        .def("getRelPos", &Node::getRelPos)
        .def("getElementByPos", &Node::getElementByPos)
        .add_property("parent", &Node::getParent)
        .add_property("active", &Node::getActive, &Node::setActive)
        .add_property("sensitive", &Node::getSensitive, &Node::setSensitive)
        .add_property("opacity", &Node::getOpacity, &Node::setOpacity)
        ;
    exportMessages(nodeClass, "Node");

    class_<AreaNode, boost::shared_ptr<AreaNode>, bases<Node>,
            boost::noncopyable>("AreaNode", no_init)
        .def("getMediaSize", &AreaNode_getMediaSize)
        .add_property("x", &AreaNode::getX, &AreaNode::setX)
        .add_property("y", &AreaNode::getY, &AreaNode::setY)
        .add_property("pos", &constPointGetterRef<AreaNode, &AreaNode::getPos>, 
                &AreaNode::setPos)
        .add_property("width", &AreaNode::getWidth, &AreaNode::setWidth)
        .add_property("height", &AreaNode::getHeight, &AreaNode::setHeight)
        .add_property("angle", &AreaNode::getAngle, &AreaNode::setAngle)
        .add_property("size", &constPointGetter<AreaNode, &AreaNode::getSize>, 
                &AreaNode::setSize)
        .add_property("pivot",  &constPointGetter<AreaNode, &AreaNode::getPivot>, 
                &AreaNode::setPivot)
        .add_property("pivotx", &deprecatedGet<AreaNode>, &deprecatedSet<AreaNode>)
        .add_property("pivoty", &deprecatedGet<AreaNode>, &deprecatedSet<AreaNode>)
        .add_property("elementoutlinecolor",
                make_function(&AreaNode::getElementOutlineColor,
                        return_value_policy<copy_const_reference>()),
                make_function(&AreaNode::setElementOutlineColor,
                        return_value_policy<copy_const_reference>()))
        ;

    export_bitmap();
    export_fx();
    export_raster();
    export_vector();
  
    class_<DivNode, bases<AreaNode>, boost::noncopyable>("DivNode", no_init)
        .def("__init__", raw_constructor(createNode<divNodeName>))
        .add_property("crop", &DivNode::getCrop, &DivNode::setCrop)
        .def("getNumChildren", &DivNode::getNumChildren)
        .def("getChild", make_function(&DivNode::getChild,
                return_value_policy<copy_const_reference>()))
        .def("appendChild", &DivNode::appendChild)
        .def("insertChildBefore", &DivNode::insertChildBefore)
        .def("insertChildAfter", &DivNode::insertChildAfter)
        .def("insertChild", &DivNode::insertChild)
        .def("removeChild", (void (DivNode::*)(NodePtr))(&DivNode::removeChild))
        .def("removeChild", (void (DivNode::*)(unsigned))(&DivNode::removeChild))
        .def("reorderChild", (void (DivNode::*)(unsigned, unsigned))
                (&DivNode::reorderChild))
        .def("reorderChild", (void (DivNode::*)(NodePtr, unsigned))
                (&DivNode::reorderChild))
        .def("indexOf", &DivNode::indexOf)
        .def("getEffectiveMediaDir", &DivNode::getEffectiveMediaDir)
        .def("dumpNodeTree", &DivNode::dump, dumpNodeTree_overloads(args("indent")))
        .add_property("mediadir", make_function(&DivNode::getMediaDir,
                return_value_policy<copy_const_reference>()), &DivNode::setMediaDir)
    ;

    class_<CanvasNode, bases<DivNode> >("CanvasNode",
            no_init)
    ;

    class_<AVGNode, bases<CanvasNode> >("AVGNode", no_init)
    ;

    class_<SoundNode, bases<AreaNode> >("SoundNode", no_init)
        .def("__init__", raw_constructor(createNode<soundNodeName>))
        .def("play", &SoundNode::play)
        .def("stop", &SoundNode::stop)
        .def("pause", &SoundNode::pause)
        .def("setEOFCallback", &SoundNode::setEOFCallback)
        .def("getAudioCodec", &SoundNode::getAudioCodec)
        .def("getAudioSampleRate", &SoundNode::getAudioSampleRate)
        .def("getNumAudioChannels", &SoundNode::getNumAudioChannels)
        .def("seekToTime", &SoundNode::seekToTime)
        .def("getCurTime", &SoundNode::getCurTime)
        .add_property("href", make_function(&SoundNode::getHRef, 
                return_value_policy<copy_const_reference>()), &SoundNode::setHRef)
        .add_property("loop", &SoundNode::getLoop)
        .add_property("duration", &SoundNode::getDuration)
        .add_property("volume", &SoundNode::getVolume, &SoundNode::setVolume)
    ;
}
