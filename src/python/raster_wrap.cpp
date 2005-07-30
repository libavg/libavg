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
    class_<DPoint>("Point",
            "A point is the basic coordinate in avg. Points are usually expressed\n"
            "in floating-point coordinates.")
        .def(init<double, double>())
        .def(init<DPoint>())
        .def_readwrite("x", &DPoint::x)
        .def_readwrite("y", &DPoint::y)
    ;

    class_<RasterNode, bases<Node>, boost::noncopyable>("RasterNode",
            "Base class for all nodes that have a direct 2d raster representation.\n"
            "This includes Image, Word, Camera, and Video nodes. RasterNodes can\n"
            "be rotated. Warping of RasterNodes is implemented using a grid of\n"
            "reference points. The position of each of these points can be changed.\n"
            "Properties:\n"
            "    angle: The angle that the node is rotated to in radians. 0 is\n"
            "        unchanged, 3.14 is upside-down.\n"
            "    pivotx, pivoty: The point that the node is rotated around. Default\n"
            "        is the center of the node.\n"
            "    maxtilewidth, maxtileheight: The maximum width and height of the\n"
            "        tiles used for warping. The effective tile size is also\n"
            "        dependent on hardware and driver limits. (ro)\n"
            "    blendmode: The method of compositing the node with the nodes under\n"
            "        it. Valid values are 'blend', 'add', 'min' and 'max'.",
            no_init) 
        .def("getNumVerticesX", &RasterNode::getNumVerticesX,
                "getNumVerticesX() -> x\n\n"
                "Returns the number of horizontal grid points.")
        .def("getNumVerticesY", &RasterNode::getNumVerticesY,
                "getNumVerticesY() -> y\n\n"
                "Returns the number of vertical grid points.")
        .def("getOrigVertexCoord", &RasterNode::getOrigVertexCoord,
                "getOrigVertexCoord(x,y) -> Point\n\n"
                "Returns the unwarped coordinate of a vertex.")
        .def("getWarpedVertexCoord", &RasterNode::getWarpedVertexCoord,
                "getWarpedVertexCoord(x,y) -> Point\n\n"
                "Returnes the current coordinate of a vertex.")
        .def("setWarpedVertexCoord", &RasterNode::setWarpedVertexCoord,
                "setWarpedVertexCoord(x,y, Point) -> None\n\n"
                "Changes the current coordinate of a vertex.")
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
    
    class_<Image, bases<RasterNode> >("Image",
            "A static raster image on the screen. Images are loaded from files on\n"
            "program start. Alpha channels of the image files are used as\n"
            "transparency information.\n"
            "Properties:\n"
            "    href: The source filename of the image.\n"
            "    hue: A hue to color the image in. (ro, deprecated)\n"
            "    saturation: The saturation the image should have. (ro, deprecated)\n")
        .add_property("href", make_function(&Image::getHRef,
                return_value_policy<copy_const_reference>()))
        .add_property("hue", &Image::getHue)
        .add_property("saturation", &Image::getSaturation)
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
                "Stpos video playback but doesn't close the file. The playback\n"
                "cursor stays at the same position.")
        .def("getFPS", &VideoBase::getFPS)
    ;  

    class_<Camera, bases<VideoBase> >("Camera"
            "A node that displays the image of a firewire camera.\n"
            "Properties:\n"
            "    device (ro)\n"
            "    framerate (ro)\n"
            "    mode (ro)\n"
            "    brightness, exposure, sharpness, saturation, gamma, shutter, gain")
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
        
    class_<Video, bases<VideoBase> >("Video"
            "Video nodes display a video file. Video formats and codecs supported\n"
            "are all formats that ffmpeg/libavcodec supports.\n"
            "Properties:\n"
            "    href: The source filename of the video. (ro)\n"
            "    loop: Whether to start the video again when it has ended. (ro)\n")
        .def("getNumFrames", &Video::getNumFrames,
                "getNumFrames() -> num\n\n")
        .def("getCurFrame", &Video::getCurFrame,
                "getCurFrame() -> num\n\n"
                "Returns the video frame currently playing.")
        .def("seekToFrame", &Video::seekToFrame,
                "seekToFrame(num) -> None\n\n"
                "Moves the playback cursor to the frame given.")
        .add_property("href", make_function(&Video::getHRef,
                return_value_policy<copy_const_reference>()))
        .add_property("loop", &Video::getLoop)
    ;

    class_<Words, bases<RasterNode> >("Words"
            "A words node displays formatted text. Rendering is done by pango. All\n"
            "properties are set in pixels. International and multi-byte character\n"
            "sets are fully supported. Words nodes should use UTF-8 to encode\n"
            "international characters.\n"
            "Properties:\n"
            "    font: The name of the truetype font to use. This font must be\n"
            "        installed in the system, for instance using the installfonts.sh\n"
            "        script in the main libavg source directory.\n"
            "    text: The string to display. In the avg file, this is either the\n"
            "        text attribute of the words node or the content of the words\n"
            "        node itself. In the second case, the string can be formatted\n"
            "        using the pango text attribute markup language described at\n"
            "        http://developer.gnome.org/doc/API/2.0/pango/PangoMarkupFormat.html.\n"
            "        Markup can also be used if the text is set using python.\n"
            "    color: The color of the text in standard html color notation:\n" 
            "        FF0000 is red, 00FF00 green, etc.\n"
            "    size: The font size.\n"
            "    parawidth: The width at which to word-wrap.\n"
            "    indent: The indentation of the first line of the paragraph.\n"
            "    linespacing: The number of pixels between different lines of a\n"
            "        paragraph.\n"
            "    alignment: The paragraph alignment. Possible values are 'left',\n"
            "        'center' and 'right'.\n"
            "    weight: Sets the character weight. Possible values are\n"
            "        'ultralight', 'light', 'normal', 'semibold', 'bold', 'ultrabold'"
            "        and 'heavy'. This value is mapped to the weights implemented\n"
            "        in the font chosen. In most cases, working weights are limited\n"
            "        to 'normal' and 'bold'.\n"
            "    italic: Boolean value that determines if the text is displayed in\n"
            "        italic\n"
            "    stretch: One of 'ultracondensed', 'extracondensed', 'condensed',\n"
            "        'semicondensed', 'normal', 'semiexpanded', 'expanded',\n"
            "        'extraexpanded' and 'ultraexpanded'. Not implemented in most\n"
            "        fonts.\n"
            "    smallcaps\n"
            )
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
