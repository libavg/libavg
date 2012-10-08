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

#include "FontStyle.h"

#include "../base/ObjectCounter.h"
#include "../base/Exception.h"

using namespace std;

namespace avg {


FontStyle::FontStyle(const string& sName, const string& sVariant, 
        const string& sColorName, float aaGamma, float size, int indent,
        float lineSpacing, const string& sAlign, const string& sWrapMode, bool bJustify, 
        float letterSpacing, bool bHint, FontStylePtr pBaseStyle)
    : m_sName(sName),
      m_sVariant(sVariant),
      m_sColorName(sColorName),
      m_AAGamma(aaGamma),
      m_Size(size),
      m_Indent(indent),
      m_LineSpacing(lineSpacing),
      m_bJustify(bJustify),
      m_LetterSpacing(letterSpacing),
      m_bHint(bHint)
{
    m_Color = colorStringToColor(m_sColorName);
    setAlignment(sAlign);
    setWrapMode(sWrapMode);

    // TODO: Support base style

    ObjectCounter::get()->incRef(&typeid(*this));
}

FontStyle::~FontStyle()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

const std::string& FontStyle::getName() const
{
    return m_sName;
}

void FontStyle::setName(const string& sName)
{
    m_sName = sName;
}

const std::string& FontStyle::getVariant() const
{
    return m_sVariant;
}

void FontStyle::setVariant(const std::string& sVariant)
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

float FontStyle::getSize() const
{
    return m_Size;
}

void FontStyle::setSize(float size)
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
                "WordsNode wrapping mode "+sWrapMode+" not supported."));
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

}
