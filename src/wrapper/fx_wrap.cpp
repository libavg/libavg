//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2014 Ulrich von Zadow
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

#include "../player/BlurFXNode.h"
#include "../player/ChromaKeyFXNode.h"
#include "../player/FXNode.h"
#include "../player/HueSatFXNode.h"
#include "../player/InvertFXNode.h"
#include "../player/NullFXNode.h"
#include "../player/ShadowFXNode.h"

#include <boost/shared_ptr.hpp>

using namespace boost::python;
using namespace avg;

namespace bp = boost::python;

void export_fx()
{

    class_<FXNode, boost::shared_ptr<FXNode>, boost::noncopyable>("FXNode", no_init)
        ;

    class_<BlurFXNode, bases<FXNode>, boost::shared_ptr<BlurFXNode>,
            boost::noncopyable>("BlurFXNode", init<optional<float> >(
                        (bp::arg("radius")=1.f)
                        ))
        .add_property("radius", &BlurFXNode::getRadius,
                &BlurFXNode::setRadius)
        ;

    class_<ChromaKeyFXNode, bases<FXNode>, boost::shared_ptr<ChromaKeyFXNode>,
            boost::noncopyable>("ChromaKeyFXNode")
        .add_property("color",
                make_function(&ChromaKeyFXNode::getColor,
                        return_value_policy<copy_const_reference>()),
                &ChromaKeyFXNode::setColor)
        .add_property("htolerance", &ChromaKeyFXNode::getHTolerance,
                &ChromaKeyFXNode::setHTolerance)
        .add_property("stolerance", &ChromaKeyFXNode::getSTolerance,
                &ChromaKeyFXNode::setSTolerance)
        .add_property("ltolerance", &ChromaKeyFXNode::getLTolerance,
                &ChromaKeyFXNode::setLTolerance)
        .add_property("softness", &ChromaKeyFXNode::getSoftness,
                &ChromaKeyFXNode::setSoftness)
        .add_property("erosion", &ChromaKeyFXNode::getErosion,
                &ChromaKeyFXNode::setErosion)
        .add_property("spillthreshold", &ChromaKeyFXNode::getSpillThreshold,
                &ChromaKeyFXNode::setSpillThreshold)
        ;

    class_<HueSatFXNode, bases<FXNode>, boost::shared_ptr<HueSatFXNode>,
            boost::noncopyable > ("HueSatFXNode",
                    init<optional<float, float, float, bool> >(
                        (bp::arg("hue")=0, bp::arg("saturation")=0,
                         bp::arg("lightness")=0, bp::arg("colorize")=false)
                    ))
        .add_property("hue", &HueSatFXNode::getHue,
                &HueSatFXNode::setHue)
        .add_property("saturation", &HueSatFXNode::getSaturation,
                &HueSatFXNode::setSaturation)
        .add_property("lightness", &HueSatFXNode::getLightnessOffset,
                &HueSatFXNode::setLightnessOffset)
        .add_property("colorize", &HueSatFXNode::isColorizing,
                &HueSatFXNode::setColorizing)
        .def("__repr__", &HueSatFXNode::toString)
        ;

    class_<InvertFXNode, bases<FXNode>, boost::shared_ptr<InvertFXNode>, 
            boost::noncopyable>("InvertFXNode")
        ;

    class_<NullFXNode, bases<FXNode>, boost::shared_ptr<NullFXNode>, boost::noncopyable>(
            "NullFXNode")
        ;

    class_<ShadowFXNode, bases<FXNode>, boost::shared_ptr<ShadowFXNode>,
            boost::noncopyable>("ShadowFXNode", 
            init<optional<glm::vec2, float, float, std::string> >(
                (bp::arg("offset")=glm::vec2(0,0), bp::arg("radius")=1.f,
                 bp::arg("opacity")=1.f, bp::arg("color")=std::string("FFFFFF"))
            ))
        .add_property("offset", &ShadowFXNode::getOffset, &ShadowFXNode::setOffset)
        .add_property("radius", &ShadowFXNode::getRadius, &ShadowFXNode::setRadius)
        .add_property("opacity", &ShadowFXNode::getOpacity, &ShadowFXNode::setOpacity)
        .add_property("color", &ShadowFXNode::getColor, &ShadowFXNode::setColor)
        ;
}
