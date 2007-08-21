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

#include "WrapHelper.h"

#include "../player/CameraNode.h"
#include "../player/Image.h"
#include "../player/Video.h"
#include "../player/Words.h"

#include <boost/python.hpp>

using namespace boost::python;
using namespace avg;
using namespace std;

void export_raster()
{
    boost::python::to_python_converter<vector<DPoint>, 
        to_list<vector<DPoint> > >();    
    boost::python::to_python_converter<VertexGrid, 
        to_list<VertexGrid> >();    
   
    from_python_sequence<vector<DPoint>, variable_capacity_policy>();
    from_python_sequence<VertexGrid, variable_capacity_policy>();

 
    class_<RasterNode, bases<Node>, boost::noncopyable>("RasterNode",
            "Base class for all nodes that have a direct 2d raster representation.\n"
            "This includes Image, Word, Camera, and Video nodes. RasterNodes can\n"
            "be rotated. Warping of RasterNodes is implemented using a grid of\n"
            "reference points. The position of each of these points can be changed.\n",
            no_init) 
        .def("getOrigVertexCoords", &RasterNode::getOrigVertexCoords,
                "getOrigVertexCoords() -> GridPoints\n\n"
                "Returns the unwarped coordinate of all vertices.")
        .def("getWarpedVertexCoords", &RasterNode::getWarpedVertexCoords,
                "getWarpedVertexCoords() -> GridPoints\n\n"
                "Returnes the current coordinate of all vertices.")
        .def("setWarpedVertexCoords", &RasterNode::setWarpedVertexCoords,
                "setWarpedVertexCoords(Grid) -> None\n\n"
                "Changes the current coordinates of all vertices.")
        .def("getBitmap", &RasterNode::getBitmap,
                return_value_policy<manage_new_object>(),
                "getBitmap() -> Bitmap\n\n"
                "Returns a copy of the bitmap that the node contains.")
        .add_property("angle", &RasterNode::getAngle, &RasterNode::setAngle,
                "The angle that the node is rotated to in radians. 0 is\n"
                "unchanged, 3.14 is upside-down.\n")
        .add_property("pivotx", &RasterNode::getPivotX, &RasterNode::setPivotX,
                "x coordinate of the point that the node is rotated around.\n"
                "Default is the center of the node.\n")
        .add_property("pivoty", &RasterNode::getPivotY, &RasterNode::setPivotY,
                "y coordinate of the point that the node is rotated around.\n"
                "Default is the center of the node.\n")
        .add_property("maxtilewidth", &RasterNode::getMaxTileWidth,
                "The maximum width and height of the\n"
                "tiles used for warping. The effective tile size is also\n"
                "dependent on hardware and driver limits. (ro)\n")
        .add_property("maxtileheight", &RasterNode::getMaxTileHeight,
                "The maximum width and height of the\n"
                "tiles used for warping. The effective tile size is also\n"
                "dependent on hardware and driver limits. (ro)\n")
        .add_property("blendmode", 
                make_function(&RasterNode::getBlendModeStr, 
                        return_value_policy<copy_const_reference>()),
                &RasterNode::setBlendModeStr,
                "The method of compositing the node with the nodes under\n"
                "it. Valid values are 'blend', 'add', 'min' and 'max'.")
    ;

    class_<Image, bases<RasterNode> >("Image",
            "A static raster image on the screen. Images are loaded from files on\n"
            "program start. Alpha channels of the image files are used as\n"
            "transparency information.\n",
            no_init)
        .def("setBitmap", &Image::setBitmap, 
                "setBitmap(Bitmap)-> None\n"
                "Sets the bitmap pixels of the image.")
        .add_property("href", 
                make_function(&Image::getHRef,
                        return_value_policy<copy_const_reference>()),
                make_function(&Image::setHRef,
                        return_value_policy<copy_const_reference>()),
                "The source filename of the image.\n")
        .add_property("hue", &Image::getHue,
                "A hue to color the image in. (ro, deprecated)\n")
        .add_property("saturation", &Image::getSaturation,
                "The saturation the image should have. (ro, deprecated)\n")
    ;

    class_<VideoBase, bases<RasterNode>, boost::noncopyable>("VideoBase", 
            "Base class for video and camera image nodes.",
            no_init)
        .def("play", &VideoBase::play,
                "play() -> None\n\n"
                "Starts video playback.")
        .def("stop", &VideoBase::stop,
                "stop() -> None\n\n"
                "Stops video playback. Closes the file and 'rewinds' the playback\n"
                "cursor.")
        .def("pause", &VideoBase::pause,
                "pause() -> None\n\n"
                "Stops video playback but doesn't close the file. The playback\n"
                "cursor stays at the same position.")
        .def("getFPS", &VideoBase::getFPS)
    ;  

    class_<CameraNode, bases<VideoBase> >("Camera",
            "A node that displays the image of a firewire camera.\n"
            "    brightness, exposure, sharpness, saturation, gamma, shutter, gain, whitebalance",
            no_init)
        .add_property("device", make_function(&CameraNode::getDevice,
                return_value_policy<copy_const_reference>()))
        .add_property("drivername", make_function(&CameraNode::getDriverName,
                return_value_policy<copy_const_reference>()))
        .add_property("framerate", &CameraNode::getFrameRate)
        .add_property("framenum", &CameraNode::getFrameNum)
        .add_property("brightness", &CameraNode::getBrightness, &CameraNode::setBrightness)
        .add_property("exposure", &CameraNode::getExposure, &CameraNode::setExposure)
        .add_property("sharpness", &CameraNode::getSharpness, &CameraNode::setSharpness)
        .add_property("saturation", &CameraNode::getSaturation, &CameraNode::setSaturation)
        .add_property("gamma", &CameraNode::getGamma, &CameraNode::setGamma)
        .add_property("shutter", &CameraNode::getShutter, &CameraNode::setShutter)
        .add_property("gain", &CameraNode::getGain, &CameraNode::setGain)
        .add_property("whitebalance", &CameraNode::getWhiteBalance, 
                &CameraNode::setWhiteBalance)
    ;
        
    class_<Video, bases<VideoBase> >("Video",
            "Video nodes display a video file. Video formats and codecs supported\n"
            "are all formats that ffmpeg/libavcodec supports.\n",
            no_init)
        .def("getNumFrames", &Video::getNumFrames,
                "getNumFrames() -> num\n\n")
        .def("getCurFrame", &Video::getCurFrame,
                "getCurFrame() -> num\n\n"
                "Returns the video frame currently playing.")
        .def("seekToFrame", &Video::seekToFrame,
                "seekToFrame(num) -> None\n\n"
                "Moves the playback cursor to the frame given.")
        .def("setEOFCallback", &Video::setEOFCallback,
                "setEOFCallback(pyfunc) -> None\n\n"
                "Sets a python function to be called when the video reaches end of file.")
        .add_property("href", 
                make_function(&Video::getHRef,
                        return_value_policy<copy_const_reference>()),
                make_function(&Video::setHRef,
                        return_value_policy<copy_const_reference>()),
                "The source filename of the video. (ro)\n")
        .add_property("loop", &Video::getLoop,
                "Whether to start the video again when it has ended. (ro)\n")
    ;

    class_<Words, bases<RasterNode> >("Words",
            "A words node displays formatted text. Rendering is done by pango. All\n"
            "properties are set in pixels. International and multi-byte character\n"
            "sets are fully supported. Words nodes should use UTF-8 to encode\n"
            "international characters.\n",
            no_init)
        .add_property("font", 
                make_function(&Words::getFont,
                        return_value_policy<copy_const_reference>()),
                make_function(&Words::setFont,
                        return_value_policy<copy_const_reference>()),
                "The family name of the truetype font to use. This font must\n"
                "be installed in the system, for instance using the installfonts.sh\n"
                "script in the main libavg source directory.\n")
        .add_property("text", 
                make_function(&Words::getText,
                        return_value_policy<copy_const_reference>()),
                make_function(&Words::setText,
                        return_value_policy<copy_const_reference>()),
                "The string to display. In the avg file, this is either the\n"
                "text attribute of the words node or the content of the words\n"
                "node itself. In the second case, the string can be formatted\n"
                "using the pango text attribute markup language described at\n"
                "http://developer.gnome.org/doc/API/2.0/pango/PangoMarkupFormat.html.\n"
                "Markup can also be used if the text is set using python.\n")
        .add_property("color", 
                make_function(&Words::getColor,
                        return_value_policy<copy_const_reference>()),
                make_function(&Words::setColor,
                        return_value_policy<copy_const_reference>()),
                "The color of the text in standard html color notation:\n" 
                "FF0000 is red, 00FF00 green, etc.\n")
        .add_property("size", &Words::getSize, &Words::setSize,
                "The font size in pixels. Fractional sizes are supported.\n")
        .add_property("parawidth", &Words::getParaWidth, &Words::setParaWidth,
                "The width at which to word-wrap.\n")
        .add_property("indent", &Words::getIndent, &Words::setIndent,
                "The indentation of the first line of the paragraph.\n")
        .add_property("linespacing", &Words::getLineSpacing, &Words::setLineSpacing,
                "The number of pixels between different lines of a\n"
                "paragraph.\n")
        .add_property("alignment", &Words::getAlignment, &Words::setAlignment,
                "The paragraph alignment. Possible values are 'left',\n"
                "'center' and 'right'.\n")
        .add_property("weight", &Words::getWeight, &Words::setWeight,
                "Sets the character weight. Possible values are\n"
                "'ultralight', 'light', 'normal', 'semibold', 'bold', 'ultrabold'\n"
                "and 'heavy'. This value is mapped to the weights implemented\n"
                "in the font chosen. In most cases, working weights are limited\n"
                "to 'normal' and 'bold'.\n")
        .add_property("italic", &Words::getItalic, &Words::setItalic,
                "Boolean value that determines if the text is displayed in italic\n")
        .add_property("stretch", &Words::getStretch,&Words::setStretch,
                "One of 'ultracondensed', 'extracondensed', 'condensed',\n"
                "'semicondensed', 'normal', 'semiexpanded', 'expanded',\n"
                "'extraexpanded' and 'ultraexpanded'. Not implemented in most\n"
                "fonts.\n")
        .add_property("smallcaps", &Words::getSmallCaps, &Words::setSmallCaps)
    ;
}
