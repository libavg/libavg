//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2011 Ulrich von Zadow
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

#include "../player/CameraNode.h"
#include "../player/ImageNode.h"
#include "../player/VideoNode.h"
#include "../player/WordsNode.h"

using namespace boost::python;
using namespace avg;
using namespace std;

char imageNodeName[] = "image";
char cameraNodeName[] = "camera";
char cameraInfoName[] = "info";
char intPointName[] = "point";
char camImageFormatName[] = "format";
char cameraControlName[] = "control";
char videoNodeName[] = "video";
char wordsNodeName[] = "words";

void export_raster()
{
    to_python_converter<VertexGrid, to_list<VertexGrid> >();    
    from_python_sequence<VertexGrid, variable_capacity_policy>();

    class_<RasterNode, bases<AreaNode>, boost::noncopyable>("RasterNode", no_init) 
        .def("getOrigVertexCoords", &RasterNode::getOrigVertexCoords)
        .def("getWarpedVertexCoords", &RasterNode::getWarpedVertexCoords)
        .def("setWarpedVertexCoords", &RasterNode::setWarpedVertexCoords)
        .def("setEffect", &RasterNode::setEffect)
        .add_property("maxtilewidth", &RasterNode::getMaxTileWidth)
        .add_property("maxtileheight", &RasterNode::getMaxTileHeight)
        .add_property("blendmode", 
                make_function(&RasterNode::getBlendModeStr, 
                        return_value_policy<copy_const_reference>()),
                &RasterNode::setBlendModeStr)
        .add_property("maskhref", 
                make_function(&RasterNode::getMaskHRef,
                        return_value_policy<copy_const_reference>()),
                &RasterNode::setMaskHRef)
        .add_property("maskpos",
                make_function(&RasterNode::getMaskPos,
                        return_value_policy<copy_const_reference>()),
                &RasterNode::setMaskPos)
        .add_property("masksize",
                make_function(&RasterNode::getMaskSize,
                        return_value_policy<copy_const_reference>()),
                &RasterNode::setMaskSize)
        .add_property("mipmap", &RasterNode::getMipmap)
        .add_property("gamma", &RasterNode::getGamma, &RasterNode::setGamma)
        .add_property("intensity", &RasterNode::getIntensity, &RasterNode::setIntensity)
        .add_property("contrast", &RasterNode::getContrast, &RasterNode::setContrast)
    ;

    class_<ImageNode, bases<RasterNode> >("ImageNode", no_init)
        .def("__init__", raw_constructor(createNode<imageNodeName>))
        .def("getBitmap", &ImageNode::getBitmap)
        .def("setBitmap", &ImageNode::setBitmap)
        .add_property("href", 
                make_function(&ImageNode::getHRef,
                        return_value_policy<copy_const_reference>()),
                &ImageNode::setHRef)
        .add_property("compression",
                &ImageNode::getCompression)
    ;

    class_<CameraNode, bases<RasterNode> >("CameraNode", no_init)
        .def("__init__", raw_constructor(createNode<cameraNodeName>))
        .add_property("device", make_function(&CameraNode::getDevice,
                return_value_policy<copy_const_reference>()))
        .add_property("driver", make_function(&CameraNode::getDriverName,
                return_value_policy<copy_const_reference>()))
        .add_property("framerate", &CameraNode::getFrameRate)
        .add_property("framenum", &CameraNode::getFrameNum)
        .add_property("brightness", &CameraNode::getBrightness, 
                &CameraNode::setBrightness)
        .add_property("sharpness", &CameraNode::getSharpness, &CameraNode::setSharpness)
        .add_property("saturation", &CameraNode::getSaturation, 
                &CameraNode::setSaturation)
        .add_property("camgamma", &CameraNode::getCamGamma, &CameraNode::setCamGamma)
        .add_property("shutter", &CameraNode::getShutter, &CameraNode::setShutter)
        .add_property("gain", &CameraNode::getGain, &CameraNode::setGain)
        .add_property("strobeduration", &CameraNode::getStrobeDuration, 
                &CameraNode::setStrobeDuration)
        .def("play", &CameraNode::play)
        .def("stop", &CameraNode::stop)
        .def("getBitmap", &CameraNode::getBitmap)
        .def("getWhitebalanceU", &CameraNode::getWhitebalanceU)
        .def("getWhitebalanceV", &CameraNode::getWhitebalanceV)
        .def("setWhitebalance", &CameraNode::setWhitebalance)
        .def("doOneShotWhitebalance", &CameraNode::doOneShotWhitebalance)
        .def("isAvailable", &CameraNode::isAvailable)
        .def("dumpCameras", make_function(&CameraNode::dumpCameras))
        .staticmethod("dumpCameras")
        .def("listCameraInfo", make_function(&CameraNode::listCameraInfo))
        .staticmethod("listCameraInfo")
        .def("resetFirewireBus", &CameraNode::resetFirewireBus)
        .staticmethod("resetFirewireBus")
    ;

    class_<IntPoint>("IntPoint", no_init)
        .def("__init__", raw_constructor(createNode<intPointName>))
        .def_readonly("x", &IntPoint::x)
        .def_readonly("y", &IntPoint::y)
        .def("getNormalized", &IntPoint::getNormalized)
        .def("safeGetNormalized", &IntPoint::safeGetNormalized)
        .def("getNorm", &IntPoint::getNorm)
        .def("isNaN", &IntPoint::isNaN)
        .def("isInf", &IntPoint::isInf)
        .def("getRotated", &IntPoint::getRotated)
        .def("getRotatedPivot", &IntPoint::getRotatedPivot)
        .def("fromPolar", &IntPoint::fromPolar)
        .staticmethod("fromPolar")
        .def("getAngle", &IntPoint::getAngle)
    ;

    //Wrap std::vector from CameraInfo to Pyhton list
    to_python_converter<CamInfoList, to_list<CamInfoList> >();
    from_python_sequence<CamInfoList, variable_capacity_policy>();

    to_python_converter<CamImageFormatsList, to_list<CamImageFormatsList> >();
    from_python_sequence<CamImageFormatsList, variable_capacity_policy>();

    to_python_converter<CamControlsList, to_list<CamControlsList> >();
    from_python_sequence<CamControlsList, variable_capacity_policy>();

    to_python_converter<FramerateList, to_list<FramerateList> >();
    from_python_sequence<FramerateList, variable_capacity_policy>();

    enum_<PixelFormat>("PixelFormat")
        .value("B5G6R5",B5G6R5)
        .value("B8G8R8", B8G8R8)
        .value("B8G8R8A8", B8G8R8A8)
        .value("B8G8R8X8", B8G8R8X8)
        .value("A8B8G8R8", A8B8G8R8)
        .value("X8B8G8R8", X8B8G8R8)
        .value("R5G6B5", R5G6B5)
        .value("R8G8B8", R8G8B8)
        .value("R8G8B8A8", R8G8B8A8)
        .value("R8G8B8X8", R8G8B8X8)
        .value("A8R8G8B8", A8R8G8B8)
        .value("X8R8G8B8", X8R8G8B8)
        .value("I8", I8)
        .value("I16", I16)
        .value("A8", A8)
        .value("YCbCr411", YCbCr411)
        .value("YCbCr422", YCbCr422)
        .value("YUYV422", YUYV422)
        .value("YCbCr420p", YCbCr420p)
        .value("YCbCrJ420p", YCbCrJ420p)
        .value("YCbCrA420p", YCbCrA420p)
        .value("BAYER8", BAYER8)
        .value("BAYER8_RGGB", BAYER8_RGGB)
        .value("BAYER8_GBRG", BAYER8_GBRG)
        .value("BAYER8_GRBG", BAYER8_GRBG)
        .value("BAYER8_BGGR", BAYER8_BGGR)
        .value("R32G32B32A32F", R32G32B32A32F)
        .value("I32F", I32F)
        .value("NO_PIXELFORMAT", NO_PIXELFORMAT)
        .export_values()
    ;

    class_<CamImageFormat>("CamImageFormat", no_init)
        .def("__init__", raw_constructor(createNode<camImageFormatName>))
        .def_readonly("size", &CamImageFormat::size)
        .def_readonly("pixelformat", &CamImageFormat::pixelformat)
        .def("getFramerates", &CamImageFormat::getFramerates)
    ;

    class_<CamControl>("CamControl", no_init)
        .def("__init__", raw_constructor(createNode<cameraControlName>))
        .def_readonly("controlName", &CamControl::sControlName)
        .def_readonly("min", &CamControl::min)
        .def_readonly("max", &CamControl::max)
        .def_readonly("defaultValue", &CamControl::defaultValue)
    ;

    class_<CameraInfo>("CameraInfo", no_init)
        .def("__init__", raw_constructor(createNode<cameraInfoName>))
        .def("getDriver", &CameraInfo::getDriver)
        .def("getDeviceID", &CameraInfo::getDeviceID)
        .def("getImageFormats", &CameraInfo::getImageFormats)
        .def("getControls", &CameraInfo::getControls)
    ;
        
    enum_<VideoNode::VideoAccelType>("VideoAccelType")
        .value("NO_ACCELERATION", VideoNode::NONE)
        .value("VDPAU", VideoNode::VDPAU)
        .export_values()
    ;

    class_<VideoNode, bases<RasterNode> >("VideoNode", no_init)
        .def("__init__", raw_constructor(createNode<videoNodeName>))
        .def("play", &VideoNode::play)
        .def("stop", &VideoNode::stop)
        .def("pause", &VideoNode::pause)
        .def("getNumFrames", &VideoNode::getNumFrames)
        .def("getNumFramesQueued", &VideoNode::getNumFramesQueued)
        .def("getCurFrame", &VideoNode::getCurFrame)
        .def("seekToFrame", &VideoNode::seekToFrame)
        .def("getStreamPixelFormat", &VideoNode::getStreamPixelFormat)
        .def("getDuration", &VideoNode::getDuration)
        .def("getBitrate", &VideoNode::getBitrate)
        .def("getVideoCodec", &VideoNode::getVideoCodec)
        .def("getAudioCodec", &VideoNode::getAudioCodec)
        .def("getAudioSampleRate", &VideoNode::getAudioSampleRate)
        .def("getNumAudioChannels", &VideoNode::getNumAudioChannels)
        .def("getCurTime", &VideoNode::getCurTime)
        .def("seekToTime", &VideoNode::seekToTime)
        .def("hasAudio", &VideoNode::hasAudio)
        .def("hasAlpha", &VideoNode::hasAlpha)
        .def("setEOFCallback", &VideoNode::setEOFCallback)
        .def("getVideoAccelConfig", &VideoNode::getVideoAccelConfig)
        .staticmethod("getVideoAccelConfig")
        .add_property("fps", &VideoNode::getFPS)
        .add_property("queuelength", &VideoNode::getQueueLength)
        .add_property("href", 
                make_function(&VideoNode::getHRef,
                        return_value_policy<copy_const_reference>()),
                &VideoNode::setHRef)
        .add_property("loop", &VideoNode::getLoop)
        .add_property("volume", &VideoNode::getVolume, &VideoNode::setVolume)
        .add_property("threaded", &VideoNode::isThreaded)
        .add_property("accelerated", &VideoNode::isAccelerated)
    ;

    class_<WordsNode, bases<RasterNode> >("WordsNode", no_init)
        .def("__init__", raw_constructor(createNode<wordsNodeName>))
        .add_property("font", 
                make_function(&WordsNode::getFont,
                        return_value_policy<copy_const_reference>()),
                make_function(&WordsNode::setFont,
                        return_value_policy<copy_const_reference>()))
        .add_property("variant", 
                make_function(&WordsNode::getFontVariant,
                        return_value_policy<copy_const_reference>()),
                make_function(&WordsNode::setFontVariant,
                        return_value_policy<copy_const_reference>()))
        .add_property("text", 
                make_function(&WordsNode::getText,
                        return_value_policy<copy_const_reference>()),
                make_function(&WordsNode::setText,
                        return_value_policy<copy_const_reference>()))
        .add_property("color", 
                make_function(&WordsNode::getColor,
                        return_value_policy<copy_const_reference>()),
                make_function(&WordsNode::setColor,
                        return_value_policy<copy_const_reference>()))
        .add_property("fontsize", &WordsNode::getFontSize, &WordsNode::setFontSize)
        .add_property("parawidth", &deprecatedGet<WordsNode>, &deprecatedSet<WordsNode>)
        .add_property("indent", &WordsNode::getIndent, &WordsNode::setIndent)
        .add_property("linespacing", &WordsNode::getLineSpacing, 
                &WordsNode::setLineSpacing)
        .add_property("alignment", &WordsNode::getAlignment, &WordsNode::setAlignment)
        .add_property("wrapmode", &WordsNode::getWrapMode, &WordsNode::setWrapMode)
        .add_property("justify", &WordsNode::getJustify, &WordsNode::setJustify)
        .add_property("rawtextmode", &WordsNode::getRawTextMode, 
                &WordsNode::setRawTextMode)
        .add_property("letterspacing", &WordsNode::getLetterSpacing, 
                &WordsNode::setLetterSpacing)
        .add_property("hint", &WordsNode::getHint, &WordsNode::setHint)
        .def("getGlyphPos", &WordsNode::getGlyphPos)
        .def("getGlyphSize", &WordsNode::getGlyphSize)
        .def("getNumLines", &WordsNode::getNumLines)
        .def("getCharIndexFromPos", &WordsNode::getCharIndexFromPos)
        .def("getTextAsDisplayed", &WordsNode::getTextAsDisplayed)
        .def("getLineExtents", &WordsNode::getLineExtents)
        .def("getFontFamilies", make_function(&WordsNode::getFontFamilies, 
                return_value_policy<copy_const_reference>()))
        .staticmethod("getFontFamilies")
        .def("getFontVariants", make_function(&WordsNode::getFontVariants, 
                return_value_policy<copy_const_reference>()))
        .staticmethod("getFontVariants")
        .def("addFontDir", &WordsNode::addFontDir)
        .staticmethod("addFontDir")
    ;
}
