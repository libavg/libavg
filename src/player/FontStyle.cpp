//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2013 Ulrich von Zadow
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

#include "FontStyle.h"

#include "../base/ObjectCounter.h"
#include "../base/Exception.h"

#include "TypeDefinition.h"
#include "Arg.h"

using namespace std;

namespace avg {

void FontStyle::registerType()
{
    TypeDefinition def = TypeDefinition("fontstyle", "",
            ExportedObject::buildObject<FontStyle>)
        .addArg(Arg<string>("font", "sans", false, offsetof(FontStyle, m_sName)))
        .addArg(Arg<string>("variant", "", false, offsetof(FontStyle, m_sVariant)))
        .addArg(Arg<string>("color", "FFFFFF", false, offsetof(FontStyle, m_sColorName)))
        .addArg(Arg<float>("aagamma", 1.0f, false, offsetof(FontStyle, m_AAGamma)))
        .addArg(Arg<float>("fontsize", 15, false, offsetof(FontStyle, m_Size)))
        .addArg(Arg<int>("indent", 0, false, offsetof(FontStyle, m_Indent)))
        .addArg(Arg<float>("linespacing", 0, false, offsetof(FontStyle, m_LineSpacing)))
        .addArg(Arg<string>("alignment", "left"))
        .addArg(Arg<string>("wrapmode", "word"))
        .addArg(Arg<bool>("justify", false, false, offsetof(FontStyle, m_bJustify)))
        .addArg(Arg<float>("letterspacing", 0, false,
                offsetof(FontStyle, m_LetterSpacing)))
        .addArg(Arg<bool>("hint", true, false, offsetof(FontStyle, m_bHint)))
        .addArg(Arg<FontStylePtr>("basestyle", FontStylePtr()))
        ;
    TypeRegistry::get()->registerType(def);
}

FontStyle::FontStyle(const ArgList& args)
{
    args.setMembers(this);
    setAlignment(args.getArgVal<string>("alignment"));
    setWrapMode(args.getArgVal<string>("wrapmode"));
    m_Color = colorStringToColor(m_sColorName);
    if (args.getArgVal<FontStylePtr>("basestyle") != 0) {
        applyBaseStyle(*(args.getArgVal<FontStylePtr>("basestyle")), args);
    }
}

FontStyle::FontStyle()
{
    const ArgList& args = TypeRegistry::get()->getTypeDef("fontstyle").getDefaultArgs();
    args.setMembers(this);
    setAlignment(args.getArgVal<string>("alignment"));
    setWrapMode(args.getArgVal<string>("wrapmode"));
    m_Color = colorStringToColor(m_sColorName);
}

FontStyle::~FontStyle()
{
}

template<class ATTR>
void setDefaultedAttr(ATTR& member, const string& sName, const ArgList& args,
        const ATTR& attr)
{
    if (args.getArg(sName)->isDefault()) {
        member = attr;
    }
}

void FontStyle::applyBaseStyle(const FontStyle& baseStyle, const ArgList& args)
{
    setDefaultedAttr(m_sName, "font", args, baseStyle.getFont());
    setDefaultedAttr(m_sVariant, "variant", args, baseStyle.getFontVariant());
    setDefaultedAttr(m_sColorName, "color", args, baseStyle.getColor());
    m_Color = colorStringToColor(m_sColorName);
    setDefaultedAttr(m_AAGamma, "aagamma", args, baseStyle.getAAGamma());
    setDefaultedAttr(m_Size, "fontsize", args, baseStyle.getFontSize());
    setDefaultedAttr(m_Indent, "indent", args, baseStyle.getIndent());
    setDefaultedAttr(m_LineSpacing, "linespacing", args, baseStyle.getLineSpacing());
    setDefaultedAttr(m_Alignment, "alignment", args, baseStyle.getAlignmentVal());
    setDefaultedAttr(m_WrapMode, "wrapmode", args, baseStyle.getWrapModeVal());
    setDefaultedAttr(m_bJustify, "justify", args, baseStyle.getJustify());
    setDefaultedAttr(m_LetterSpacing, "letterspacing", args,
            baseStyle.getLetterSpacing());
    setDefaultedAttr(m_bHint, "hint", args, baseStyle.getHint());
}

template<class ARG>
void setDefaultedArg(ARG& member, const string& sName, const ArgList& args)
{
    if (!args.getArg(sName)->isDefault()) {
        member = args.getArgVal<ARG>(sName);
    }
}

void FontStyle::setDefaultedArgs(const ArgList& args)
{
    // Warning: The ArgList here contains args that are for a different class originally,
    // so the member offsets are wrong.
    setDefaultedArg(m_sName, "font", args);
    setDefaultedArg(m_sVariant, "variant", args);
    setDefaultedArg(m_sColorName, "color", args);
    setColor(m_sColorName);
    setDefaultedArg(m_AAGamma, "aagamma", args);
    setDefaultedArg(m_Size, "fontsize", args);
    setDefaultedArg(m_Indent, "indent", args);
    setDefaultedArg(m_LineSpacing, "linespacing", args);
    string s = getAlignment();
    setDefaultedArg(s, "alignment", args);
    setAlignment(s);
    s = getWrapMode();
    setDefaultedArg(s, "wrapmode", args);
    setWrapMode(s);
    setDefaultedArg(m_bJustify, "justify", args);
    setDefaultedArg(m_LetterSpacing, "letterspacing", args);
    setDefaultedArg(m_bHint, "hint", args);
}

const std::string& FontStyle::getFont() const
{
    return m_sName;
}

void FontStyle::setFont(const string& sName)
{
    m_sName = sName;
}

const std::string& FontStyle::getFontVariant() const
{
    return m_sVariant;
}

void FontStyle::setFontVariant(const std::string& sVariant)
{
    m_sVariant = sVariant;
}

const std::string& FontStyle::getColor() const
{
    return m_sColorName;
}

void FontStyle::setColor(const string& sColor)
{
    m_sColorName = sColor;
    m_Color = colorStringToColor(m_sColorName);
}

float FontStyle::getAAGamma() const
{
    return m_AAGamma;
}

void FontStyle::setAAGamma(float gamma)
{
    m_AAGamma = gamma;
}

float FontStyle::getFontSize() const
{
    return m_Size;
}

void FontStyle::setFontSize(float size)
{
    if (size <= 1) {
        throw Exception(AVG_ERR_INVALID_ARGS, "Font size < 1 is illegal.");
    } 
    m_Size = size;
}

int FontStyle::getIndent() const
{
    return m_Indent;
}

void FontStyle::setIndent(int indent)
{
    m_Indent = indent;
}

float FontStyle::getLineSpacing() const
{
    return m_LineSpacing;
}

void FontStyle::setLineSpacing(float lineSpacing)
{
    m_LineSpacing = lineSpacing;
}

string FontStyle::getAlignment() const
{
    switch(m_Alignment) {
        case PANGO_ALIGN_LEFT:
            return "left";
        case PANGO_ALIGN_CENTER:
            return "center";
        case PANGO_ALIGN_RIGHT:
            return "right";
        default:
            AVG_ASSERT(false);
            return "";
    }
}

void FontStyle::setAlignment(const string& sAlign)
{
    if (sAlign == "left") {
        m_Alignment = PANGO_ALIGN_LEFT;
    } else if (sAlign == "center") {
        m_Alignment = PANGO_ALIGN_CENTER;
    } else if (sAlign == "right") {
        m_Alignment = PANGO_ALIGN_RIGHT;
    } else {
        throw(Exception(AVG_ERR_UNSUPPORTED, 
                "Alignment "+sAlign+" not supported."));
    }
}

void FontStyle::setWrapMode(const string& sWrapMode)
{
    if (sWrapMode == "word") {
        m_WrapMode = PANGO_WRAP_WORD;
    } else if (sWrapMode == "char") {
        m_WrapMode = PANGO_WRAP_CHAR;
    } else if (sWrapMode == "wordchar") {
        m_WrapMode = PANGO_WRAP_WORD_CHAR;
    } else {
        throw(Exception(AVG_ERR_UNSUPPORTED, 
                "FontStyle wrapping mode "+sWrapMode+" not supported."));
    }
}

string FontStyle::getWrapMode() const
{
    switch(m_WrapMode) {
        case PANGO_WRAP_WORD:
            return "word";
        case PANGO_WRAP_CHAR:
            return "char";
        case PANGO_WRAP_WORD_CHAR:
            return "wordchar";
        default:
            AVG_ASSERT(false);
            return "";
    }
}

bool FontStyle::getJustify() const
{
    return m_bJustify;
}

void FontStyle::setJustify(bool bJustify)
{
    m_bJustify = bJustify;
}

float FontStyle::getLetterSpacing() const
{
    return m_LetterSpacing;
}

void FontStyle::setLetterSpacing(float letterSpacing)
{
    m_LetterSpacing = letterSpacing;
}

bool FontStyle::getHint() const
{
    return m_bHint;
}

void FontStyle::setHint(bool bHint)
{
    m_bHint = bHint;
}

PangoAlignment FontStyle::getAlignmentVal() const
{
    return m_Alignment;
}

PangoWrapMode FontStyle::getWrapModeVal() const
{
    return m_WrapMode;
}

Pixel32 FontStyle::getColorVal() const
{
    return m_Color;
}

}
