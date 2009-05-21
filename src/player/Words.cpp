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

#include "Words.h"
#include "SDLDisplayEngine.h"
#include "OGLSurface.h"
#include "NodeDefinition.h"
#include "TextEngine.h"

#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/ScopeTimer.h"
#include "../base/XMLHelper.h"
#include "../base/StringHelper.h"
#include "../base/MathHelper.h"

#include "../graphics/Filterfill.h"

#include <pango/pangoft2.h>

#include <iostream>
#include <algorithm>

using namespace std;

namespace avg {

NodeDefinition Words::createDefinition()
{
    static const string sDTDElements = 
        "<!ELEMENT span (#PCDATA|span|b|big|i|s|sub|sup|small|tt|u)*>\n"
        "<!ATTLIST span\n"
        "  font_desc CDATA #IMPLIED\n"
        "  font_family CDATA #IMPLIED\n"
        "  face CDATA #IMPLIED\n"
        "  size CDATA #IMPLIED\n"
        "  style CDATA #IMPLIED\n"
        "  weight CDATA #IMPLIED\n"
        "  variant CDATA #IMPLIED\n"
        "  stretch CDATA #IMPLIED\n"
        "  foreground CDATA #IMPLIED\n"
        "  background CDATA #IMPLIED\n"
        "  underline CDATA #IMPLIED\n"
        "  rise CDATA #IMPLIED\n"
        "  strikethrough CDATA #IMPLIED\n"
        "  fallback CDATA #IMPLIED\n"
        "  lang CDATA #IMPLIED\n"
        "  letter_spacing CDATA #IMPLIED\n"
        "  rawtextmode CDATA #IMPLIED >\n"

        "<!ELEMENT b (#PCDATA|span|b|big|i|s|sub|sup|small|tt|u)*>\n"
        "<!ELEMENT big (#PCDATA|span|b|big|i|s|sub|sup|small|tt|u)*>\n"
        "<!ELEMENT i (#PCDATA|span|b|big|i|s|sub|sup|small|tt|u)*>\n"
        "<!ELEMENT s (#PCDATA|span|b|big|i|s|sub|sup|small|tt|u)*>\n"
        "<!ELEMENT sub (#PCDATA|span|b|big|i|s|sub|sup|small|tt|u)*>\n"
        "<!ELEMENT sup (#PCDATA|span|b|big|i|s|sub|sup|small|tt|u)*>\n"
        "<!ELEMENT small (#PCDATA|span|b|big|i|s|sub|sup|small|tt|u)*>\n"
        "<!ELEMENT tt (#PCDATA|span|b|big|i|s|sub|sup|small|tt|u)*>\n"
        "<!ELEMENT u (#PCDATA|span|b|big|i|s|sub|sup|small|tt|u)*>\n"
        "<!ELEMENT br (#PCDATA)*>\n";

    string sChildArray[] = {"#PCDATA", "span", "b", "big", "i", "s", "sup", "sub", 
            "small", "tt", "u", "br"};
    vector<string> sChildren = vectorFromCArray(sizeof(sChildArray)/sizeof(*sChildArray),
            sChildArray); 
    return NodeDefinition("words", Node::buildNode<Words>)
        .extendDefinition(RasterNode::createDefinition())
        .addChildren(sChildren)
        .addDTDElements(sDTDElements)
        .addArg(Arg<string>("font", "arial", false, offsetof(Words, m_sFontName)))
        .addArg(Arg<string>("variant", "", false, offsetof(Words, m_sFontVariant)))
        .addArg(Arg<string>("text", ""))
        .addArg(Arg<string>("color", "FFFFFF", false, offsetof(Words, m_sColorName)))
        .addArg(Arg<double>("size", 15, false, offsetof(Words, m_Size)))
        .addArg(Arg<int>("parawidth", -1, false, offsetof(Words, m_ParaWidth)))
        .addArg(Arg<int>("indent", 0, false, offsetof(Words, m_Indent)))
        .addArg(Arg<double>("linespacing", -1, false, offsetof(Words, m_LineSpacing)))
        .addArg(Arg<string>("alignment", "left"))
        .addArg(Arg<string>("wrapmode", "word"))
        .addArg(Arg<bool>("justify", false, false, offsetof(Words, m_bJustify)))
        .addArg(Arg<bool>("rawtextmode", false, false, offsetof(Words, m_bRawTextMode)))
        .addArg(Arg<double>("letterspacing", 0, false, offsetof(Words, m_LetterSpacing)))
        ;
}

Words::Words(const ArgList& Args, bool bFromXML)
    : m_StringExtents(0,0),
      m_pFontDescription(0),
      m_pLayout(0),
      m_bFontChanged(true),
      m_bDrawNeeded(true)
{
    m_bParsedText = false;

    Args.setMembers(this);
    setAlignment(Args.getArgVal<string>("alignment"));
    setWrapMode(Args.getArgVal<string>("wrapmode"));
    setText(Args.getArgVal<string>("text"));
    m_Color = colorStringToColor(m_sColorName);
    ObjectCounter::get()->incRef(&typeid(*this));
}

Words::~Words()
{
    if (m_pFontDescription) {
        pango_font_description_free(m_pFontDescription);
    }
    if (m_pLayout) {
        g_object_unref(m_pLayout);
    }
    ObjectCounter::get()->decRef(&typeid(*this));
}

void Words::setTextFromNodeValue(const string& sText)
{
//    cerr << "NODE VALUE: " << sText << "|" << endl;
    // Gives priority to Node Values only if they aren't empty
    UTF8String sTemp = removeExcessSpaces(sText);
    if (sTemp.length() != 0) {
        setText(sText);
    }
}

void Words::setRenderingEngines(DisplayEngine * pDisplayEngine, AudioEngine * pAudioEngine)
{
    m_bFontChanged = true;
    m_bDrawNeeded = true;
    RasterNode::setRenderingEngines(pDisplayEngine, pAudioEngine);
}

void Words::disconnect()
{
    if (m_pFontDescription) {
        pango_font_description_free(m_pFontDescription);
        m_pFontDescription = 0;
        m_bFontChanged = true;
    }
    RasterNode::disconnect();
}

string Words::getAlignment() const
{
    switch(m_Alignment) {
        case PANGO_ALIGN_LEFT:
            return "left";
        case PANGO_ALIGN_CENTER:
            return "center";
        case PANGO_ALIGN_RIGHT:
            return "right";
        default:
            assert(false);
            return "";
    }
}

void Words::setAlignment(const string& sAlign)
{
    if (sAlign == "left") {
        m_Alignment = PANGO_ALIGN_LEFT;
    } else if (sAlign == "center") {
        m_Alignment = PANGO_ALIGN_CENTER;
    } else if (sAlign == "right") {
        m_Alignment = PANGO_ALIGN_RIGHT;
    } else {
        throw(Exception(AVG_ERR_UNSUPPORTED, 
                "Words alignment "+sAlign+" not supported."));
    }

    m_bDrawNeeded = true;
}

bool Words::getJustify() const
{
    return m_bJustify;
}

void Words::setJustify(bool bJustify)
{
    m_bJustify = bJustify;
    m_bDrawNeeded = true;
}

double Words::getLetterSpacing() const
{
    return m_LetterSpacing;
}

void Words::setLetterSpacing(double letterSpacing)
{
    m_LetterSpacing = letterSpacing;
    m_bDrawNeeded = true;
}

double Words::getWidth() 
{
    drawString();
    return AreaNode::getWidth();
}

double Words::getHeight()
{
    drawString();
    return AreaNode::getHeight();
}

const std::string& Words::getFont() const
{
    return m_sFontName;
}

void Words::setFont(const std::string& sName)
{
    m_sFontName = sName;
    m_bFontChanged = true;
    m_bDrawNeeded = true;
}

const std::string& Words::getFontVariant() const
{
    return m_sFontVariant;
}

void Words::addFontDir(const std::string& sDir)
{
    TextEngine::get().addFontDir(sDir);
}

void Words::setFontVariant(const std::string& sVariant)
{
    m_sFontVariant = sVariant;
    m_bFontChanged = true;
    m_bDrawNeeded = true;
}

const UTF8String& Words::getText() const 
{
    return m_sRawText;
}

void Words::setText(const UTF8String& sText)
{
    if (sText.length() > 32767) {
        throw(Exception(AVG_ERR_INVALID_ARGS, 
                string("Words::setText: string too long (") + toString(sText.length()) + ")"));
    }
//    cerr << "setText(): " << sText << "|" << endl;
    if (m_sRawText != sText) {
        m_sRawText = sText;
        m_sText = m_sRawText;
        if (m_bRawTextMode) {
            m_bParsedText = false;
        } else {
            setParsedText(sText);
        }
        m_bDrawNeeded = true;
    }
}

const std::string& Words::getColor() const
{
    return m_sColorName;
}

void Words::setColor(const std::string& sColor)
{
    m_sColorName = sColor;
    m_Color = colorStringToColor(m_sColorName);
    m_bDrawNeeded = true;
}

double Words::getFontSize() const
{
    return m_Size;
}

void Words::setFontSize(double Size)
{
    m_Size = Size;
    m_bFontChanged = true;
    m_bDrawNeeded = true;
}

int Words::getParaWidth() const
{
    return m_ParaWidth;
}

void Words::setParaWidth(int ParaWidth)
{
    m_ParaWidth = ParaWidth;
    m_bDrawNeeded = true;
}

int Words::getIndent() const
{
    return m_Indent;
}

void Words::setIndent(int Indent)
{
    m_Indent = Indent;
    m_bDrawNeeded = true;
}

double Words::getLineSpacing() const
{
    return m_LineSpacing;
}

void Words::setLineSpacing(double LineSpacing)
{
    m_LineSpacing = LineSpacing;
    m_bDrawNeeded = true;
}

bool Words::getRawTextMode() const
{
    return m_bRawTextMode;
}

void Words::setRawTextMode(bool RawTextMode)
{
    if (RawTextMode != m_bRawTextMode) {
        m_sText = m_sRawText;
        if (RawTextMode) {
            m_bParsedText = false;
        } else {
            setParsedText(m_sText);
        }
        m_bRawTextMode = RawTextMode;
        m_bDrawNeeded = true;
    }
}

IntPoint Words::getGlyphPos(int i)
{
    PangoRectangle rect = getGlyphRect(i);
    return IntPoint(rect.x/PANGO_SCALE, rect.y/PANGO_SCALE);
}

IntPoint Words::getGlyphSize(int i)
{
    PangoRectangle rect = getGlyphRect(i);
    return IntPoint(rect.width/PANGO_SCALE, rect.height/PANGO_SCALE);
}

void Words::setWrapMode(const string& sWrapMode)
{
    if (sWrapMode == "word") {
        m_WrapMode = PANGO_WRAP_WORD;
    } else if (sWrapMode == "char") {
        m_WrapMode = PANGO_WRAP_CHAR;
    } else if (sWrapMode == "wordchar") {
        m_WrapMode = PANGO_WRAP_WORD_CHAR;
    } else {
        throw(Exception(AVG_ERR_UNSUPPORTED, 
                "Words wrapping mode "+sWrapMode+" not supported."));
    }

    m_bDrawNeeded = true;
}

string Words::getWrapMode() const
{
    switch(m_WrapMode) {
        case PANGO_WRAP_WORD:
            return "word";
        case PANGO_WRAP_CHAR:
            return "char";
        case PANGO_WRAP_WORD_CHAR:
            return "wordchar";
        default:
            assert(false);
            return "";
    }
}

void Words::parseString(PangoAttrList** ppAttrList, char** ppText)
{
    UTF8String sTextWithoutBreaks = applyBR(m_sText);
    bool bOk;
    GError * pError = 0;
    bOk = (pango_parse_markup(sTextWithoutBreaks.c_str(), 
            int(sTextWithoutBreaks.length()), 0,
            ppAttrList, ppText, 0, &pError) != 0);
    if (!bOk) {
        string sError;
        if (getID() != "") {
            sError = string("Can't parse string in node with id '")+getID()+"' ("+pError->message+")";
        } else {
            sError = string("Can't parse string '")+m_sRawText+"' ("+pError->message+")";
        }
        throw Exception(AVG_ERR_CANT_PARSE_STRING, sError);
    }

}

static ProfilingZone DrawStringProfilingZone("  Words::drawString");

void Words::drawString()
{
    assert (m_sText.length() < 32767);
    if (!m_bDrawNeeded) {
        return;
    }
    ScopeTimer Timer(DrawStringProfilingZone);
    if (m_sText.length() == 0) {
        m_StringExtents = IntPoint(0,0);
    } else {
        if (m_bFontChanged) {
            if (m_pFontDescription) {
                pango_font_description_free(m_pFontDescription);
            }
            m_pFontDescription = TextEngine::get().getFontDescription(m_sFontName, 
                    m_sFontVariant);
            pango_font_description_set_absolute_size(m_pFontDescription,
                    (int)(m_Size * PANGO_SCALE));

            m_bFontChanged = false;
        }
        PangoContext* pContext = TextEngine::get().getPangoContext();
        pango_context_set_font_description(pContext, m_pFontDescription);

        if (m_pLayout) {
            g_object_unref(m_pLayout);
        }
        m_pLayout = pango_layout_new(pContext);

        PangoAttrList * pAttrList = 0;
#if PANGO_VERSION > PANGO_VERSION_ENCODE(1,18,2) 
        PangoAttribute * pLetterSpacing = pango_attr_letter_spacing_new
                (int(m_LetterSpacing*1024));
#endif
        if (m_bParsedText) {
            char * pText = 0;
            parseString(&pAttrList, &pText);
#if PANGO_VERSION > PANGO_VERSION_ENCODE(1,18,2) 
            // Workaround for pango bug.
            pango_attr_list_insert_before(pAttrList, pLetterSpacing);
#endif            
            pango_layout_set_text(m_pLayout, pText, -1);
            g_free (pText);
        } else {
            pAttrList = pango_attr_list_new();
#if PANGO_VERSION > PANGO_VERSION_ENCODE(1,18,2) 
            pango_attr_list_insert_before(pAttrList, pLetterSpacing);
#endif
            pango_layout_set_text(m_pLayout, m_sText.c_str(), -1);
        }
        pango_layout_set_attributes(m_pLayout, pAttrList);
        pango_attr_list_unref(pAttrList);

        pango_layout_set_wrap(m_pLayout, m_WrapMode);
        pango_layout_set_alignment(m_pLayout, m_Alignment);
        pango_layout_set_justify(m_pLayout, m_bJustify);
        pango_layout_set_width(m_pLayout, m_ParaWidth * PANGO_SCALE);
        pango_layout_set_indent(m_pLayout, m_Indent * PANGO_SCALE);
        if (m_Indent < 0) {
            // For hanging indentation, we add a tabstop to support lists
            PangoTabArray* pTabs = pango_tab_array_new_with_positions(1, false,
                    PANGO_TAB_LEFT, -m_Indent * PANGO_SCALE);
            pango_layout_set_tabs(m_pLayout, pTabs);
            pango_tab_array_free(pTabs);
        }
        if (m_LineSpacing != -1) {
            pango_layout_set_spacing(m_pLayout, (int)(m_LineSpacing*PANGO_SCALE));
        }
        PangoRectangle logical_rect;
        PangoRectangle ink_rect;
        pango_layout_get_pixel_extents(m_pLayout, &ink_rect, &logical_rect);
        assert (logical_rect.width < 4096);
        assert (logical_rect.height < 4096);
//        cerr << "Ink: " << ink_rect.x << ", " << ink_rect.y << ", " 
//                << ink_rect.width << ", " << ink_rect.height << endl;
//        cerr << "Logical: " << logical_rect.x << ", " << logical_rect.y << ", " 
//                << logical_rect.width << ", " << logical_rect.height << endl;
        m_StringExtents.y = logical_rect.height+2;
        m_StringExtents.x = m_ParaWidth;
        if (m_ParaWidth == -1) {
            m_StringExtents.x = logical_rect.width;
            // Work around what appears to be a pango bug when computing the 
            // extents of italic text by adding an arbritary amount to the width.
            m_StringExtents.x += int(m_Size/6+1);
        }
        if (m_StringExtents.x == 0) {
            m_StringExtents.x = 1;
        }
        if (m_StringExtents.y == 0) {
            m_StringExtents.y = 1;
        }
//        cerr << "libavg Extents: " << m_StringExtents << endl;
        if (getState() == NS_CANRENDER) {
            getSurface()->create(m_StringExtents, I8, false);

            BitmapPtr pBmp = getSurface()->lockBmp();
            FilterFill<unsigned char>(0).applyInPlace(pBmp);
            FT_Bitmap bitmap;
            bitmap.rows = m_StringExtents.y;
            bitmap.width = m_StringExtents.x;
            unsigned char * pLines = pBmp->getPixels();
            bitmap.pitch = pBmp->getStride();
            bitmap.buffer = pLines;
            bitmap.num_grays = 256;
            bitmap.pixel_mode = ft_pixel_mode_grays;

            m_PosOffset = IntPoint(0,0);
            if (ink_rect.y < 0) {
                m_PosOffset.y = ink_rect.y;
            }
            if (ink_rect.x < 0) {
                m_PosOffset.x = ink_rect.x;
            }
            pango_ft2_render_layout(&bitmap, m_pLayout, -m_PosOffset.x, -m_PosOffset.y);

            getSurface()->bind();
            if (m_LineSpacing == -1) {
                m_LineSpacing = pango_layout_get_spacing(m_pLayout)/PANGO_SCALE;
            }
        }
    }
    if (getState() == NS_CANRENDER) {
        m_bDrawNeeded = false;
        setViewport(-32767, -32767, -32767, -32767);
    }
}

void Words::preRender()
{
    Node::preRender();
    drawString();
}

static ProfilingZone RenderProfilingZone("Words::render");

void Words::render(const DRect& Rect)
{
    ScopeTimer Timer(RenderProfilingZone);
    if (m_sText.length() != 0 && getEffectiveOpacity() > 0.001) {
        if (m_PosOffset != IntPoint(0,0)) {
            getDisplayEngine()->pushTransform(DPoint(m_PosOffset), 0, DPoint(0,0));
        }
        getSurface()->blta8(getSize(), getEffectiveOpacity(), m_Color, getBlendMode());
        if (m_PosOffset != IntPoint(0,0)) {
            getDisplayEngine()->popTransform();
        }
    }
}

IntPoint Words::getMediaSize()
{
    drawString();
    return m_StringExtents;
}

const vector<string>& Words::getFontFamilies()
{
    return TextEngine::get().getFontFamilies();
}

const vector<string>& Words::getFontVariants(const string& sFontName)
{
    return TextEngine::get().getFontVariants(sFontName);
}

string Words::removeExcessSpaces(const string & sText)
{
    string s = sText;
    size_t lastPos = s.npos;
    size_t pos = s.find_first_of(" \n\r");
    while (pos != s.npos) {
        s[pos] = ' ';
        if (pos == lastPos+1) {
            s.erase(pos, 1);
            pos--;
        }
        lastPos = pos;
        pos = s.find_first_of(" \n\r", pos+1);
    }
    return s;
}
        
PangoRectangle Words::getGlyphRect(int i)
{
    
    if (i >= int(g_utf8_strlen(m_sText.c_str(), -1)) || i < 0) {
        throw(Exception(AVG_ERR_INVALID_ARGS, 
                string("getGlyphRect: Index ") + toString(i) + " out of range."));
    }
    char * pChar = g_utf8_offset_to_pointer(m_sText.c_str(), i);
    int byteOffset = pChar-m_sText.c_str();
    drawString();
    PangoRectangle rect;
    
    if (m_pLayout) {
        pango_layout_index_to_pos(m_pLayout, byteOffset, &rect);
    } else {
        rect.x = 0;
        rect.y = 0;
        rect.width = 0;
        rect.height = 0;
    }
    return rect;
}

void Words::setParsedText(const UTF8String& sText)
{
    m_sText = removeExcessSpaces(sText);
    m_bDrawNeeded = true;

    // This just does a syntax check and throws an exception if appropriate.
    // The results are discarded.
    PangoAttrList * pAttrList = 0;
    char * pText = 0;
    parseString(&pAttrList, &pText);
    pango_attr_list_unref (pAttrList);
    g_free (pText);
    m_bParsedText = true;
}

UTF8String Words::applyBR(const UTF8String& sText)
{
    UTF8String sResult(sText);
    UTF8String sLowerText = tolower(sResult); 
    string::size_type pos=sLowerText.find("<br/>");
    while (pos != string::npos) {
        sResult.replace(pos, 5, "\n");
        sLowerText.replace(pos, 5, "\n");
        if (sLowerText[pos+1] == ' ') {
            sLowerText.erase(pos+1, 1);
            sResult.erase(pos+1, 1);
        }
        pos=sLowerText.find("<br/>");
    }
    return sResult;
}

}

