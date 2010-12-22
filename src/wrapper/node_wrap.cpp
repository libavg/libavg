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

void export_bitmap();
void export_fx();
void export_raster();
void export_event();
#ifndef WIN32
void export_devices();
#endif

#include "WrapHelper.h"
#include "raw_constructor.hpp"

#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../player/Player.h"
#include "../player/AVGNode.h"
#include "../player/CanvasNode.h"
#include "../player/DivNode.h"
#include "../player/PanoImageNode.h"
#include "../player/SoundNode.h"
#include "../player/LineNode.h"
#include "../player/RectNode.h"
#include "../player/CurveNode.h"
#include "../player/PolyLineNode.h"
#include "../player/PolygonNode.h"
#include "../player/CircleNode.h"
#include "../player/MeshNode.h"

#include <boost/version.hpp>
#include <boost/shared_ptr.hpp>

using namespace boost::python;
using namespace avg;

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(unlink_overloads, VisibleNode::unlink, 0, 1);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(disconnectEventHandler_overloads, 
        VisibleNode::disconnectEventHandler, 1, 2);

// These function templates essentially call functions such as AreaNode::getPos()
// and return a version of the result that don't allow setting of the individual
// elements of the DPoint returned.
// Without this stuff, python code like node.pos.x=30 would fail silently. With it,
// it at least throws an exception.
template<class CLASS, const DPoint& (CLASS::*FUNC)() const>
ConstDPoint constPointGetterRef(const CLASS& node)
{
    return (node.*FUNC)();
}

template<class CLASS, DPoint (CLASS::*FUNC)() const>
ConstDPoint constPointGetter(const CLASS& node)
{
    return (node.*FUNC)();
}

ConstDPoint AreaNode_getMediaSize(AreaNode* This)
{
    return (DPoint)(This->getMediaSize());
}

char divNodeName[] = "div";
char avgNodeName[] = "avg";
char soundNodeName[] = "sound";
char panoImageNodeName[] = "panoimage";
char lineNodeName[] = "line";
char rectNodeName[] = "rect";
char curveNodeName[] = "curve";
char polylineNodeName[] = "polyline";
char polygonNodeName[] = "polygon";
char circleNodeName[] = "circle";
char meshNodeName[] = "mesh";

void export_node()
{

    class_<Node, boost::shared_ptr<Node>, boost::noncopyable>("Node", no_init)
        .def(self == self)
        .def(self != self)
        .def("__hash__", &Node::getHash)
        .add_property("id", make_function(&Node::getID,
                return_value_policy<copy_const_reference>()),  &Node::setID)
        ;

    class_<VisibleNode, bases<Node>, boost::shared_ptr<VisibleNode>, boost::noncopyable>(
            "VisibleNode", no_init)
        .def("getParent", &VisibleNode::getParent)
        .def("unlink", &VisibleNode::unlink, unlink_overloads(args("bKill")))
        .def("setEventCapture", &VisibleNode::setMouseEventCapture)
        .def("setEventCapture", &VisibleNode::setEventCapture)
        .def("releaseEventCapture", &VisibleNode::releaseMouseEventCapture)
        .def("releaseEventCapture", &VisibleNode::releaseEventCapture)
        .def("setEventHandler", &VisibleNode::setEventHandler)
        .def("connectEventHandler", &VisibleNode::connectEventHandler)
        .def("disconnectEventHandler", &VisibleNode::disconnectEventHandler,
                disconnectEventHandler_overloads(args("pFunc")))
        .def("getAbsPos", &VisibleNode::getAbsPos)
        .def("getRelPos", &VisibleNode::getRelPos)
        .def("getElementByPos", &VisibleNode::getElementByPos)
        .add_property("active", &VisibleNode::getActive, &VisibleNode::setActive)
        .add_property("sensitive", &VisibleNode::getSensitive, &VisibleNode::setSensitive)
        .add_property("opacity", &VisibleNode::getOpacity, &VisibleNode::setOpacity)
        ;

    class_<AreaNode, boost::shared_ptr<AreaNode>, bases<VisibleNode>, boost::noncopyable>(
            "AreaNode", 
            no_init)
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
        ;
    export_bitmap();
    export_fx();
    export_raster();
  
    class_<DivNode, bases<AreaNode>, boost::noncopyable>("DivNode", no_init)
        .def("__init__", raw_constructor(createNode<divNodeName>))
        .add_property("crop", &DivNode::getCrop, &DivNode::setCrop)
        .add_property("elementoutlinecolor",
                make_function(&DivNode::getElementOutlineColor,
                        return_value_policy<copy_const_reference>()),
                make_function(&DivNode::setElementOutlineColor,
                        return_value_policy<copy_const_reference>()))
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

    class_<PanoImageNode, bases<AreaNode> >("PanoImageNode", no_init)
        .def("__init__", raw_constructor(createNode<panoImageNodeName>))
        .def("getScreenPosFromPanoPos", &PanoImageNode::getScreenPosFromPanoPos)
        .def("getScreenPosFromAngle", &PanoImageNode::getScreenPosFromAngle)
        .add_property("href", make_function(&PanoImageNode::getHRef, 
                return_value_policy<copy_const_reference>()), &PanoImageNode::setHRef)
        .add_property("sensorwidth", &PanoImageNode::getSensorWidth, 
                &PanoImageNode::setSensorWidth)
        .add_property("sensorheight", &PanoImageNode::getSensorHeight, 
                &PanoImageNode::setSensorHeight)
        .add_property("focallength", &PanoImageNode::getFocalLength, 
                &PanoImageNode::setFocalLength)
        .add_property("rotation", &PanoImageNode::getRotation, 
                &PanoImageNode::setRotation)
        .add_property("maxrotation", &PanoImageNode::getMaxRotation)
    ;

    class_<VectorNode, bases<VisibleNode>, boost::noncopyable>("VectorNode", 
            no_init)
        .add_property("strokewidth", &VectorNode::getStrokeWidth, 
                &VectorNode::setStrokeWidth)
        .add_property("color", make_function(&VectorNode::getColor,
                return_value_policy<copy_const_reference>()), &VectorNode::setColor)
        .add_property("texhref", make_function(&VectorNode::getTexHRef,
                return_value_policy<copy_const_reference>()), &VectorNode::setTexHRef)
        .add_property("blendmode", 
                make_function(&VectorNode::getBlendModeStr, 
                        return_value_policy<copy_const_reference>()),
                &VectorNode::setBlendModeStr)
        .def("setBitmap", &VectorNode::setBitmap)
    ;

    class_<FilledVectorNode, bases<VectorNode>, boost::noncopyable>("FilledVectorNode", 
            no_init)
        .add_property("filltexhref", make_function(&FilledVectorNode::getFillTexHRef,
                return_value_policy<copy_const_reference>()), 
                &FilledVectorNode::setFillTexHRef)
        .add_property("fillcolor", make_function(&FilledVectorNode::getFillColor,
                return_value_policy<copy_const_reference>()), 
                &FilledVectorNode::setFillColor)
        .add_property("fillopacity", &FilledVectorNode::getFillOpacity, 
                &FilledVectorNode::setFillOpacity)
        .add_property("filltexcoord1", make_function(&FilledVectorNode::getFillTexCoord1,
               return_value_policy<copy_const_reference>()), 
               &FilledVectorNode::setFillTexCoord1)
        .add_property("filltexcoord2", make_function(&FilledVectorNode::getFillTexCoord2,
               return_value_policy<copy_const_reference>()), 
               &FilledVectorNode::setFillTexCoord2)
        .def("setFillBitmap", &FilledVectorNode::setFillBitmap) 
    ;

    class_<LineNode, bases<VectorNode>, boost::noncopyable>("LineNode", 
            no_init)
        .def("__init__", raw_constructor(createNode<lineNodeName>))
        .add_property("pos1", &constPointGetterRef<LineNode, &LineNode::getPos1>,
                &LineNode::setPos1)
        .add_property("pos2", &constPointGetterRef<LineNode, &LineNode::getPos2>,
                &LineNode::setPos2)
        .add_property("x1", &deprecatedGet<LineNode>, &deprecatedSet<LineNode>)
        .add_property("y1", &deprecatedGet<LineNode>, &deprecatedSet<LineNode>)
        .add_property("x2", &deprecatedGet<LineNode>, &deprecatedSet<LineNode>)
        .add_property("y2", &deprecatedGet<LineNode>, &deprecatedSet<LineNode>)        
        .add_property("texcoord1", &LineNode::getTexCoord1, &LineNode::setTexCoord1)
        .add_property("texcoord2", &LineNode::getTexCoord2, &LineNode::setTexCoord2)
    ;

    class_<RectNode, bases<FilledVectorNode>, boost::noncopyable>("RectNode", 
            no_init)
        .def("__init__", raw_constructor(createNode<rectNodeName>))
        .add_property("pos", &constPointGetterRef<RectNode, &RectNode::getPos>, 
                &RectNode::setPos)
        .add_property("size", &constPointGetter<RectNode, &RectNode::getSize>,
                &RectNode::setSize)
        .add_property("x", &deprecatedGet<RectNode>, &deprecatedSet<RectNode>)
        .add_property("y", &deprecatedGet<RectNode>, &deprecatedSet<RectNode>)
        .add_property("width", &deprecatedGet<RectNode>, &deprecatedSet<RectNode>)
        .add_property("height", &deprecatedGet<RectNode>, &deprecatedSet<RectNode>) 
        .add_property("texcoords", make_function(&RectNode::getTexCoords, 
                return_value_policy<copy_const_reference>()), &RectNode::setTexCoords)
        .add_property("angle", &RectNode::getAngle, &RectNode::setAngle)
    ;
    
    class_<CurveNode, bases<VectorNode>, boost::noncopyable>("CurveNode", 
            no_init)
        .def("__init__", raw_constructor(createNode<curveNodeName>))
        .add_property("pos1", &constPointGetterRef<CurveNode, &CurveNode::getPos1>,
               &CurveNode::setPos1)
        .add_property("pos2", &constPointGetterRef<CurveNode, &CurveNode::getPos2>,
               &CurveNode::setPos2)
        .add_property("pos3", &constPointGetterRef<CurveNode, &CurveNode::getPos3>,
               &CurveNode::setPos3)
        .add_property("pos4", &constPointGetterRef<CurveNode, &CurveNode::getPos4>,
               &CurveNode::setPos4)
        .add_property("x1", &deprecatedGet<CurveNode>, &deprecatedSet<CurveNode>)
        .add_property("y1", &deprecatedGet<CurveNode>, &deprecatedSet<CurveNode>)
        .add_property("x2", &deprecatedGet<CurveNode>, &deprecatedSet<CurveNode>)
        .add_property("y2", &deprecatedGet<CurveNode>, &deprecatedSet<CurveNode>)
        .add_property("x3", &deprecatedGet<CurveNode>, &deprecatedSet<CurveNode>)
        .add_property("y3", &deprecatedGet<CurveNode>, &deprecatedSet<CurveNode>)
        .add_property("x4", &deprecatedGet<CurveNode>, &deprecatedSet<CurveNode>)
        .add_property("y4", &deprecatedGet<CurveNode>, &deprecatedSet<CurveNode>)
        .add_property("texcoord1", &CurveNode::getTexCoord1, &CurveNode::setTexCoord1)
        .add_property("texcoord2", &CurveNode::getTexCoord2, &CurveNode::setTexCoord2)
    ;

    class_<PolyLineNode, bases<VectorNode>, boost::noncopyable>("PolyLineNode", no_init)
        .def("__init__", raw_constructor(createNode<polylineNodeName>))
        .add_property("pos", make_function(&PolyLineNode::getPos, 
                return_value_policy<copy_const_reference>()), &PolyLineNode::setPos)
        .add_property("texcoords", make_function(&PolyLineNode::getTexCoords, 
                return_value_policy<copy_const_reference>()), &PolyLineNode::setTexCoords)
        .add_property("linejoin", &PolyLineNode::getLineJoin, &PolyLineNode::setLineJoin)
    ;

    class_<PolygonNode, bases<FilledVectorNode>, boost::noncopyable>("PolygonNode", 
            no_init)
        .def("__init__", raw_constructor(createNode<polygonNodeName>))
        .add_property("pos", make_function(&PolygonNode::getPos, 
                return_value_policy<copy_const_reference>()), &PolygonNode::setPos)
        .add_property("texcoords", make_function(&PolygonNode::getTexCoords, 
                return_value_policy<copy_const_reference>()), &PolygonNode::setTexCoords)
        .add_property("linejoin", &PolygonNode::getLineJoin, &PolygonNode::setLineJoin)
    ;

    class_<CircleNode, bases<FilledVectorNode>, boost::noncopyable>("CircleNode", 
            no_init)
        .def("__init__", raw_constructor(createNode<circleNodeName>))
        .add_property("pos", &constPointGetterRef<CircleNode, &CircleNode::getPos>,
               &CircleNode::setPos)
        .add_property("x", &deprecatedGet<CircleNode>, &deprecatedSet<CircleNode>)
        .add_property("y", &deprecatedGet<CircleNode>, &deprecatedSet<CircleNode>)
        .add_property("r", &CircleNode::getR, &CircleNode::setR)
        .add_property("texcoord1", &CircleNode::getTexCoord1, &CircleNode::setTexCoord1)
        .add_property("texcoord2", &CircleNode::getTexCoord2, &CircleNode::setTexCoord2)
    ;
    
    class_<MeshNode, bases<VectorNode>, boost::noncopyable>("MeshNode", no_init)
        .def("__init__", raw_constructor(createNode<meshNodeName>))
        .add_property("vertexcoords", make_function(&MeshNode::getVertexCoords,
                return_value_policy<copy_const_reference>()), &MeshNode::setVertexCoords)
        .add_property("texcoords", make_function(&MeshNode::getTexCoords,
                return_value_policy<copy_const_reference>()), &MeshNode::setTexCoords)
        .add_property("triangles", make_function(&MeshNode::getTriangles,
                return_value_policy<copy_const_reference>()), &MeshNode::setTriangles)
    ;
    
}
