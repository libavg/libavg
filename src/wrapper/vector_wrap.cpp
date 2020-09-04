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

#include "WrapHelper.h"
#include "raw_constructor.hpp"

#include "../player/LineNode.h"
#include "../player/RectNode.h"
#include "../player/CurveNode.h"
#include "../player/PolyLineNode.h"
#include "../player/PolygonNode.h"
#include "../player/CircleNode.h"
#include "../player/MeshNode.h"

using namespace boost::python;
using namespace avg;
using namespace std;

char lineNodeName[] = "line";
char rectNodeName[] = "rect";
char curveNodeName[] = "curve";
char polylineNodeName[] = "polyline";
char polygonNodeName[] = "polygon";
char circleNodeName[] = "circle";
char meshNodeName[] = "mesh";

void export_vector()
{
    // vector< vector<vec2> > PolygonNode
    to_python_converter<VectorVec2Vector, to_list<VectorVec2Vector> >();
    from_python_sequence<VectorVec2Vector>();

    class_<VectorNode, bases<Node>, boost::noncopyable>("VectorNode", 
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
        .add_property("length", &CurveNode::getCurveLen)
        .def("getPtOnCurve", &CurveNode::getPtOnCurve)
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
        .add_property("backfacecull", &MeshNode::getBackfaceCull,
                &MeshNode::setBackfaceCull)
    ;    
}
