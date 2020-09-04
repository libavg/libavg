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

void export_raster2();

#include "WrapHelper.h"
#include "raw_constructor.hpp"

#include "../graphics/Bitmap.h"
#include "../player/ImageNode.h"
#include "../player/FontStyle.h"
#include "../player/WordsNode.h"

using namespace boost::python;
using namespace avg;
using namespace std;

char imageNodeName[] = "image";
char fontStyleName[] = "fontstyle";
char wordsNodeName[] = "words";

void export_raster()
{
    scope mainScope;

    scope rasterScope = class_<RasterNode, bases<AreaNode>, 
            boost::noncopyable>("RasterNode", no_init) 
        .def("getOrigVertexCoords", &RasterNode::getOrigVertexCoords)
        .def("getWarpedVertexCoords", &RasterNode::getWarpedVertexCoords)
        .def("setWarpedVertexCoords", &RasterNode::setWarpedVertexCoords)
        .def("setMaskBitmap", &RasterNode::setMaskBitmap)
        .def("setMirror", &RasterNode::setMirror)
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

    enum_<RasterNode::MirrorType>("MirrorType")
        .value("HORIZONTAL", RasterNode::HORIZONTAL)
        .value("VERTICAL", RasterNode::VERTICAL)
        .export_values()
    ;

    scope oldScope1(mainScope);

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

    export_raster2();
}
