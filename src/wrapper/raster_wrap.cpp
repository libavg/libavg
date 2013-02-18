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
#include "../player/FontStyle.h"
#include "../player/WordsNode.h"

using namespace boost::python;
using namespace avg;
using namespace std;

char imageNodeName[] = "image";
char cameraNodeName[] = "camera";
char videoNodeName[] = "video";
char fontStyleName[] = "fontstyle";
char wordsNodeName[] = "words";

void export_raster()
{

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
        .def("getCamerasInfos", make_function(&CameraNode::getCamerasInfos))
        .staticmethod("getCamerasInfos")
        .def("resetFirewireBus", &CameraNode::resetFirewireBus)
        .staticmethod("resetFirewireBus")
    ;

    //Wrap std::vector from CameraInfo to Pyhton list
    to_python_converter<CamerasInfosVector, to_list<CamerasInfosVector> >();
    from_python_sequence<CamerasInfosVector, variable_capacity_policy>();

    to_python_converter<CameraImageFormatsVector, to_list<CameraImageFormatsVector> >();
    from_python_sequence<CameraImageFormatsVector, variable_capacity_policy>();

    to_python_converter<CameraControlsVector, to_list<CameraControlsVector> >();
    from_python_sequence<CameraControlsVector, variable_capacity_policy>();

    to_python_converter<FrameratesVector, to_list<FrameratesVector> >();
    from_python_sequence<FrameratesVector, variable_capacity_policy>();

    class_<CameraImageFormat>("CameraImageFormat", no_init)
        .add_property("size", &CameraImageFormat::getSize)
        .add_property("pixelFormat", &CameraImageFormat::getPixelFormat)
        .add_property("framerates", &CameraImageFormat::getFramerates)
    ;

    class_<CameraControl>("CameraControl", no_init)
        .add_property("controlName", &CameraControl::getControlName)
        .add_property("min", &CameraControl::getMin)
        .add_property("max", &CameraControl::getMax)
        .add_property("default", &CameraControl::getDefault)
    ;

    class_<CameraInfo>("CameraInfo", no_init)
        .add_property("driver", &CameraInfo::getDriver)
        .add_property("deviceID", &CameraInfo::getDeviceID)
        .add_property("imageFormats", &CameraInfo::getImageFormats)
        .add_property("controls", &CameraInfo::getControls)
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
        .def("getVideoDuration", &VideoNode::getVideoDuration)
        .def("getAudioDuration", &VideoNode::getAudioDuration)
        .def("getBitrate", &VideoNode::getBitrate)
        .def("getContainerFormat", &VideoNode::getContainerFormat)
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

    class_<FontStyle, bases<ExportedObject> >("FontStyle", no_init)
        .def("__init__", raw_constructor(createExportedObject<fontStyleName>))
        .def("__copy__", copyObject<FontStyle>)
        .add_property("font", 
                make_function(&FontStyle::getFont,
                        return_value_policy<copy_const_reference>()),
                make_function(&FontStyle::setFont,
                        return_value_policy<copy_const_reference>()))
        .add_property("variant", 
                make_function(&FontStyle::getFontVariant,
                        return_value_policy<copy_const_reference>()),
                make_function(&FontStyle::setFontVariant,
                        return_value_policy<copy_const_reference>()))
        .add_property("color", 
                make_function(&FontStyle::getColor,
                        return_value_policy<copy_const_reference>()),
                make_function(&FontStyle::setColor,
                        return_value_policy<copy_const_reference>()))
        .add_property("aagamma", &FontStyle::getAAGamma, &FontStyle::setAAGamma)
        .add_property("fontsize", &FontStyle::getFontSize, &FontStyle::setFontSize)
        .add_property("indent", &FontStyle::getIndent, &FontStyle::setIndent)
        .add_property("linespacing", &FontStyle::getLineSpacing, 
                &FontStyle::setLineSpacing)
        .add_property("alignment", &FontStyle::getAlignment, &FontStyle::setAlignment)
        .add_property("wrapmode", &FontStyle::getWrapMode, &FontStyle::setWrapMode)
        .add_property("justify", &FontStyle::getJustify, &FontStyle::setJustify)
        .add_property("letterspacing", &FontStyle::getLetterSpacing, 
                &FontStyle::setLetterSpacing)
        .add_property("hint", &FontStyle::getHint, &FontStyle::setHint)
    ;

    class_<WordsNode, bases<RasterNode> >("WordsNode", no_init)
        .def("__init__", raw_constructor(createNode<wordsNodeName>))
        .add_property("fontstyle",
                make_function(&WordsNode::getFontStyle,
                        return_value_policy<copy_const_reference>()),
                &WordsNode::setFontStyle)
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
                &WordsNode::setText)
        .add_property("color", 
                make_function(&WordsNode::getColor,
                        return_value_policy<copy_const_reference>()),
                make_function(&WordsNode::setColor,
                        return_value_policy<copy_const_reference>()))
        .add_property("aagamma", &WordsNode::getAAGamma, &WordsNode::setAAGamma)
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
