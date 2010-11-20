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
        .def("getBitmap", &RasterNode::getBitmap)
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
        .add_property("intensity", &RasterNode::getIntensity)
        .add_property("contrast", &RasterNode::getContrast, &RasterNode::setContrast)
    ;

    class_<ImageNode, bases<RasterNode> >("ImageNode", no_init)
        .def("__init__", raw_constructor(createNode<imageNodeName>))
        .def("setBitmap", &ImageNode::setBitmap)
        .add_property("href", 
                make_function(&ImageNode::getHRef,
                        return_value_policy<copy_const_reference>()),
                make_function(&ImageNode::setHRef,
                        return_value_policy<copy_const_reference>()))
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
        .def("getWhitebalanceU", &CameraNode::getWhitebalanceU)
        .def("getWhitebalanceV", &CameraNode::getWhitebalanceV)
        .def("setWhitebalance", &CameraNode::setWhitebalance)
        .def("doOneShotWhitebalance", &CameraNode::doOneShotWhitebalance)
        .def("isAvailable", &CameraNode::isAvailable)
        .def("dumpCameras", make_function(&CameraNode::dumpCameras))
        .staticmethod("dumpCameras")
        .def("resetFirewireBus", &CameraNode::resetFirewireBus)
        .staticmethod("resetFirewireBus")
    ;
        
    class_<VideoNode, bases<RasterNode> >("VideoNode",
            "Video nodes display a video file. Video formats and codecs supported\n"
            "are all formats that ffmpeg/libavcodec supports.\n",
            no_init)
        .def("__init__", raw_constructor(createNode<videoNodeName>))
        .def("play", &VideoNode::play,
                "play()\n"
                "Starts video playback.")
        .def("stop", &VideoNode::stop,
                "stop()\n"
                "Stops video playback. Closes the file and 'rewinds' the playback\n"
                "cursor.")
        .def("pause", &VideoNode::pause,
                "pause()\n"
                "Stops video playback but doesn't close the object. The playback\n"
                "cursor stays at the same position.")
        .def("getNumFrames", &VideoNode::getNumFrames,
                "getNumFrames()")
        .def("getNumFramesQueued", &VideoNode::getNumFramesQueued,
                "getNumFramesQueued()\n"
                "Returns the number of frames already decoded and waiting for playback.")
        .def("getCurFrame", &VideoNode::getCurFrame,
                "getCurFrame()\n"
                "Returns the video frame currently playing.")
        .def("seekToFrame", &VideoNode::seekToFrame,
                "seekToFrame(num)\n"
                "Moves the playback cursor to the frame given.")
        .def("getStreamPixelFormat", &VideoNode::getStreamPixelFormat,
                "getStreamPixelFormat() -> string\n"
                "Returns the pixel format of the video file as a string. Possible\n"
                "pixel formats are described in\n"
                "http://cekirdek.pardus.org.tr/~ismail/ffmpeg-docs/ffmpeg-r_2libavutil_2avutil_8h.html\n")
        .def("getDuration", &VideoNode::getDuration,
                "getDuration() -> duration\n"
                "Returns the duration of the video in milliseconds./n")
        .def("getBitrate", &VideoNode::getBitrate,
                "getBitrate() -> bitrate\n"
                "Returns the number of bits in the file per second./n")
        .def("getVideoCodec", &VideoNode::getVideoCodec,
                "getVideoCodec() -> vcodec\n"
                "Returns the video codec used as a string such as 'mpeg4'.\n")
        .def("getAudioCodec", &VideoNode::getAudioCodec,
                "getAudioCodec() -> acodec\n"
                "Returns the audio codec used as a string such as 'mp2'\n")
        .def("getAudioSampleRate", &VideoNode::getAudioSampleRate,
                "getAudioSampleRate() -> samplerate\n"
                "Returns the sample rate in samples per second (for example, 44100).\n")
        .def("getNumAudioChannels", &VideoNode::getNumAudioChannels,
                "getNumAudioChannels() -> numchannels\n"
                "Returns the number of audio channels. 2 for stereo, etc.\n")
        .def("getCurTime", &VideoNode::getCurTime,
                "getCurTime()\n"
                "Returns milliseconds of playback time since video start.")
        .def("seekToTime", &VideoNode::seekToTime,
                "seekToTime(millisecs)\n"
                "Moves the playback cursor to the time given.")
        .def("hasAudio", &VideoNode::hasAudio,
                "hasAudio() -> bool\n"
                "Returns true if the video contains an audio stream. Throws an\n"
                "exception if the video has not been opened yet.\n")
        .def("hasAlpha", &VideoNode::hasAlpha,
                "hasAlpha() -> bool\n"
                "Returns true if the video contains an alpha (transparency) channel."
                "Throws an exception if the video has not been opened yet.")
        .def("setEOFCallback", &VideoNode::setEOFCallback,
                "setEOFCallback(pyfunc)\n"
                "Sets a python callable to be invoked when the video reaches end of\n"
                "file.")
        .add_property("fps", &VideoNode::getFPS,
                "The nominal frames per second the object should display at. Can only be"
                "set at node construction.")
        .add_property("queuelength", &VideoNode::getQueueLength,
                "The length of the decoder queue in video frames. This is the number of"
                "frames that can be decoded before the first one is displayed. A higher"
                "number increases memory consumption but also resilience against"
                "data source latency. Can only be set at node construction. Can't be set"
                "if threaded=False, since there is no queue in that case.")
        .add_property("href", 
                make_function(&VideoNode::getHRef,
                        return_value_policy<copy_const_reference>()),
                make_function(&VideoNode::setHRef,
                        return_value_policy<copy_const_reference>()),
                "The source filename of the video.\n")
        .add_property("loop", &VideoNode::getLoop,
                "Whether to start the video again when it has ended Can only be"
                "set at node construction.\n")
        .add_property("volume", &VideoNode::getVolume, &VideoNode::setVolume,
                "Audio playback volume for this video. 0 is silence, 1 passes media\n"
                "file volume through unchanged. Values higher than 1 can be used to\n"
                "amplify sound if the sound file doesn't use the complete dynamic\n"
                "range. If there is no audio track, the call is ignored.\n")
        .add_property("threaded", &VideoNode::isThreaded,
                "Whether to use separate threads to decode the video. The default is"
                "True. Setting this attribute to False makes seeking much quicker. It"
                "also disables audio and prevents libavg from distributing the CPU"
                "load over several cores of a multi-core computer.")
    ;

    class_<WordsNode, bases<RasterNode> >("WordsNode",
            "A words node displays formatted text. Rendering is done by pango. All\n"
            "properties are set in pixels. International and multi-byte character\n"
            "sets are fully supported. Words nodes should use UTF-8 to encode\n"
            "international characters. The pos attribute of a words node is the\n"
            "logical top left of the first character for left-aligned text. For\n"
            "centered and right-aligned text, it is the top center and right of the\n"
            "first line, respectively. For latin text, the logical top usually\n"
            "corresponds to the height of the ascender. There may be cases where\n"
            "portions of the text are rendered to the left of or above the position,\n"
            "for instance when italics are used.\n",
            no_init)
        .def("__init__", raw_constructor(createNode<wordsNodeName>))
        .add_property("font", 
                make_function(&WordsNode::getFont,
                        return_value_policy<copy_const_reference>()),
                make_function(&WordsNode::setFont,
                        return_value_policy<copy_const_reference>()),
                "The family name of the truetype font to use. This font must\n"
                "be installed in the system (for instance using the installfonts.sh\n"
                "script in the main libavg source directory) or available in a fonts/\n"
                "subdirectory of the current directory.\n")
        .add_property("variant", 
                make_function(&WordsNode::getFontVariant,
                        return_value_policy<copy_const_reference>()),
                make_function(&WordsNode::setFontVariant,
                        return_value_policy<copy_const_reference>()),
                "The variant (bold, italic, etc.) of the truetype font to use. To\n"
                "figure out which variants are available, use the avg_showfonts.py\n"
                "utility.\n")
        .add_property("text", 
                make_function(&WordsNode::getText,
                        return_value_policy<copy_const_reference>()),
                make_function(&WordsNode::setText,
                        return_value_policy<copy_const_reference>()),
                "The string to display. In the avg file, this is either the\n"
                "text attribute of the words node or the content of the words\n"
                "node itself. In the second case, the string can be formatted\n"
                "using the pango text attribute markup language described at\n"
                "U{http://library.gnome.org/devel/pango/unstable/PangoMarkupFormat.html}.\n"
                "Markup can also be used if the text is set using python.\n"
                "Markup parsing is controlled with 'rawtextmode' property.\n")
        .add_property("color", 
                make_function(&WordsNode::getColor,
                        return_value_policy<copy_const_reference>()),
                make_function(&WordsNode::setColor,
                        return_value_policy<copy_const_reference>()),
                "The color of the text in standard html color notation:\n" 
                "FF0000 is red, 00FF00 green, etc.\n")
        .add_property("fontsize", &WordsNode::getFontSize, &WordsNode::setFontSize,
                "The font size in pixels. Fractional sizes are supported.\n")
        .add_property("parawidth", &deprecatedGet<WordsNode>, &deprecatedSet<WordsNode>)
        .add_property("indent", &WordsNode::getIndent, &WordsNode::setIndent,
                "The indentation of the first line of the paragraph.\n")
        .add_property("linespacing", &WordsNode::getLineSpacing, 
                &WordsNode::setLineSpacing,
                "The number of pixels between different lines of a\n"
                "paragraph.\n")
        .add_property("alignment", &WordsNode::getAlignment, &WordsNode::setAlignment,
                "The paragraph alignment. Possible values are 'left',\n"
                "'center' and 'right'.\n")
        .add_property("wrapmode", &WordsNode::getWrapMode, &WordsNode::setWrapMode,
                "Paragraph's wrap behaviour. Possible values are\n"
                "'word' (break to the nearest space, default), 'char' (break\n"
                "at any position) and 'wordchar' (break words but fall back"
                "to char mode if there is no free space for a full word).")
        .add_property("justify", &WordsNode::getJustify, &WordsNode::setJustify,
                "Whether each complete line should be stretched to fill\n"
                "the entire width of the layout. Default is false.\n")
        .add_property("rawtextmode", &WordsNode::getRawTextMode, 
                &WordsNode::setRawTextMode,
                "Sets whether the text should be parsed (False, default) or\n"
                "interpreted as raw string (True).\n")
        .add_property("letterspacing", &WordsNode::getLetterSpacing, 
                &WordsNode::setLetterSpacing,
                "The amount of space between the idividual glyphs of the text in\n"
                "pixels, with 0 being standard spacing and negative values indicating\n"
                "packed text (less letter spacing than normal). Only active when text\n"
                "attribute markup is not being used.\n")
        .add_property("hint", &WordsNode::getHint, &WordsNode::setHint,
                "Whether or not hinting (http://en.wikipedia.org/wiki/Font_hinting)\n"
                "should be used when rendering the text. Unfortunately, this setting\n"
                "does not override the fontconfig settings in\n"
                "/etc/fonts/conf.d/*-hinting.conf or other fontconfig configuration\n"
                "files.\n")
        .def("getGlyphPos", &WordsNode::getGlyphPos,
                "getGlyphPos(i)->pos\n"
                "Returns the position of the glyph at byte index i in the layout.\n"
                "The position is a Point2D, in pixels, and relative to the words\n"
                "node. Formatting html-syntax as <b> or <i> is treated as zero chars,\n"
                "<br/> is treatet as one char.\n")
        .def("getGlyphSize", &WordsNode::getGlyphSize,
                "getGlyphSize(i)->pos\n"
                "Returns the size of the glyph at byte index i in the layout.\n"
                "The position is a Point2D, in pixels. Formatting html-syntax\n"
                "as <b> or <i> is treated as zero chars, <br/> is treatet as one char.\n")
        .def("getNumLines", &WordsNode::getNumLines,
                "getNumLines()\n"
                "Returns the number of lines\n")
        .def("getCharIndexFromPos", &WordsNode::getCharIndexFromPos,
                "getCharIndexFromPos(p)->index\n"
                "Returns the number of chars to the position p, or none\n"
                "if no char is clicked. p is relative to the WordsNode.\n"
                "Formatting html-syntax as <b> or <i> is treated as zero chars,\n"
                "<br/> is treatet as one char. To get the text matched to this\n"
                "use getTextAsDisplayed()\n")
        .def("getTextAsDisplayed", &WordsNode::getTextAsDisplayed,
                "Returns the text without text attribute markup language, <br/>\n"
                "is replaced by \\n \n")
        .def("getLineExtents", &WordsNode::getLineExtents,
                "getLineExtents(line)->(width,height)\n"
                "Returns the Width and Height of the specified line \n"
                "The width and height is a Point2D, in pixels.\n")    
        .def("getFontFamilies", make_function(&WordsNode::getFontFamilies, 
                return_value_policy<copy_const_reference>()))
        .staticmethod("getFontFamilies")
        .def("getFontVariants", make_function(&WordsNode::getFontVariants, 
                return_value_policy<copy_const_reference>()))
        .staticmethod("getFontVariants")
        .def("addFontDir", &WordsNode::addFontDir,
                "addFontDir(s)\n"
                "Adds a directory to be searched for fonts."
                "This only works before Player.play().")
        .staticmethod("addFontDir")
    ;
}
