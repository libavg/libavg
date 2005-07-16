//
// $Id$
//

#include "../player/Point.h"
#include "../player/Camera.h"
#include "../player/Image.h"
#include "../player/Video.h"
#include "../player/Words.h"

#include <boost/python.hpp>

using namespace boost::python;
using namespace avg;

void export_raster()
{
    class_<DPoint>("Point")
        .def(init<double, double>())
        .def(init<DPoint>())
        .def_readwrite("x", &DPoint::x)
        .def_readwrite("y", &DPoint::y)
    ;

    class_<RasterNode, bases<Node>, boost::noncopyable>("RasterNode", no_init)
        .def("getNumVerticesX", &RasterNode::getNumVerticesX)
        .def("getNumVerticesY", &RasterNode::getNumVerticesY)
        .def("getOrigVertexCoord", &RasterNode::getOrigVertexCoord)
        .def("getWarpedVertexCoord", &RasterNode::getWarpedVertexCoord)
        .def("setWarpedVertexCoord", &RasterNode::setWarpedVertexCoord)
        .add_property("angle", &RasterNode::getAngle, &RasterNode::setAngle)
        .add_property("pivotx", &RasterNode::getPivotX, &RasterNode::setPivotX)
        .add_property("pivoty", &RasterNode::getPivotY, &RasterNode::setPivotY)
        .add_property("maxtilewidth", &RasterNode::getMaxTileWidth)
        .add_property("maxtileheight", &RasterNode::getMaxTileHeight)
        .add_property("blendmode", 
                make_function(&RasterNode::getBlendModeStr, 
                        return_value_policy<copy_const_reference>()),
                &RasterNode::setBlendModeStr)
    ;
    
    class_<Image, bases<RasterNode> >("Image")
        .add_property("href", make_function(&Image::getHRef,
                return_value_policy<copy_const_reference>()))
        .add_property("hue", &Image::getHue)
        .add_property("saturation", &Image::getSaturation)
    ;

    class_<VideoBase, bases<RasterNode>, boost::noncopyable>("VideoBase", no_init)
        .def("play", &VideoBase::play)
        .def("stop", &VideoBase::stop)
        .def("pause", &VideoBase::pause)
        .def("getFPS", &VideoBase::getFPS)
    ;  

    class_<Camera, bases<VideoBase> >("Camera")
        .add_property("device", make_function(&Camera::getDevice,
                return_value_policy<copy_const_reference>()))
        .add_property("framerate", &Camera::getFrameRate)
        .add_property("mode", make_function(&Camera::getMode,
                return_value_policy<copy_const_reference>()))
        .add_property("brightness", &Camera::getBrightness, &Camera::setBrightness)
        .add_property("exposure", &Camera::getExposure, &Camera::setExposure)
        .add_property("sharpness", &Camera::getSharpness, &Camera::setSharpness)
        .add_property("saturation", &Camera::getSaturation, &Camera::setSaturation)
        .add_property("gamma", &Camera::getGamma, &Camera::setGamma)
        .add_property("shutter", &Camera::getShutter, &Camera::setShutter)
        .add_property("gain", &Camera::getGain, &Camera::setGain)
    ;
        
    class_<Video, bases<VideoBase> >("Video")
        .def("getNumFrames", &Video::getNumFrames)
        .def("getCurFrame", &Video::getCurFrame)
        .def("seekToFrame", &Video::seekToFrame)
        .add_property("href", make_function(&Video::getHRef,
                return_value_policy<copy_const_reference>()))
        .add_property("loop", &Video::getLoop)
    ;

    class_<Words, bases<RasterNode> >("Words")
        .add_property("font", 
                make_function(&Words::getFont,
                        return_value_policy<copy_const_reference>()),
                make_function(&Words::setFont,
                        return_value_policy<copy_const_reference>()))
        .add_property("text", 
                make_function(&Words::getText,
                        return_value_policy<copy_const_reference>()),
                make_function(&Words::setText,
                        return_value_policy<copy_const_reference>()))
        .add_property("color", 
                make_function(&Words::getColor,
                        return_value_policy<copy_const_reference>()),
                make_function(&Words::setColor,
                        return_value_policy<copy_const_reference>()))
        .add_property("size", &Words::getSize, &Words::setSize)
        .add_property("parawidth", &Words::getParaWidth, &Words::setParaWidth)
        .add_property("indent", &Words::getIndent, &Words::setIndent)
        .add_property("linespacing", &Words::getLineSpacing, &Words::setLineSpacing)
        .add_property("alignment", &Words::getAlignment, &Words::setAlignment)
        .add_property("weight", &Words::getWeight, &Words::setWeight)
        .add_property("italic", &Words::getItalic, &Words::setItalic)
        .add_property("stretch", &Words::getStretch,&Words::setStretch)
        .add_property("smallcaps", &Words::getSmallCaps, &Words::setSmallCaps)
    ;
    
}
