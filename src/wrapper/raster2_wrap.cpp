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

#include "../player/CameraNode.h"
#include "../player/VideoNode.h"

using namespace boost::python;
using namespace avg;
using namespace std;

char cameraNodeName[] = "camera";
char videoNodeName[] = "video";

long long getDurationDeprecated(VideoNode* pNode)
{
    avgDeprecationWarning("1.9.0", "VideoNode.getDuration()", "VideoNode.duration");
    return pNode->getDuration();
}

void export_raster2()
{
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

    to_python_converter<CamerasInfosVector, to_list<CamerasInfosVector> >();
    from_python_sequence<CamerasInfosVector>();

    to_python_converter<CameraImageFormatsVector, to_list<CameraImageFormatsVector> >();
    from_python_sequence<CameraImageFormatsVector>();

    to_python_converter<CameraControlsVector, to_list<CameraControlsVector> >();
    from_python_sequence<CameraControlsVector>();

    to_python_converter<FrameratesVector, to_list<FrameratesVector> >();
    from_python_sequence<FrameratesVector>();

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
        .def("getDuration", &getDurationDeprecated)
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
        .def("isSeeking", &VideoNode::isSeeking)
        .def("hasAudio", &VideoNode::hasAudio)
        .def("hasAlpha", &VideoNode::hasAlpha)
        .def("setEOFCallback", &VideoNode::setEOFCallback)
        .add_property("fps", &VideoNode::getFPS)
        .add_property("queuelength", &VideoNode::getQueueLength)
        .add_property("href", 
                make_function(&VideoNode::getHRef,
                        return_value_policy<copy_const_reference>()),
                &VideoNode::setHRef)
        .add_property("loop", &VideoNode::getLoop)
        .add_property("volume", &VideoNode::getVolume, &VideoNode::setVolume)
        .add_property("threaded", &VideoNode::isThreaded)
        .add_property("duration", &VideoNode::getDuration)
    ;
}
