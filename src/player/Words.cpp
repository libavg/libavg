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
#include "DisplayEngine.h"
#include "ISurface.h"
#include "NodeDefinition.h"

#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/ScopeTimer.h"
#include "../base/XMLHelper.h"
#include "../base/OSHelper.h"
#include "../base/FileHelper.h"
#include "../base/StringHelper.h"
#include "../base/MathHelper.h"

#include "../graphics/Filterfill.h"

#include <pango/pangoft2.h>

#include <iostream>
#include <stdlib.h>
#include <algorithm>
#include <cctype> // for toupper

using namespace std;

namespace avg {

set<string> Words::s_sFontsNotFound;
set<pair<string, string> > Words::s_VariantsNotFound;
bool Words::s_bInitialized = false;
int Words::s_NumFontFamilies = 0;
PangoFontFamily** Words::s_ppFontFamilies = 0;
PangoContext * Words::s_pPangoContext = 0;

void GLibLogFunc(const gchar *log_domain, GLogLevelFlags log_level, 
        const gchar *message, gpointer unused_data)
{
    string s = "Pango ";
    if (log_level & G_LOG_LEVEL_ERROR) {
        s += "error: ";
    } else if (log_level & G_LOG_LEVEL_CRITICAL) {
        s += string("critical: ")+message;
        AVG_TRACE(Logger::ERROR, s);
        assert(false);
    } else if (log_level & G_LOG_LEVEL_WARNING) {
        s += "warning: ";
    } else if (log_level & G_LOG_LEVEL_MESSAGE) {
        s += "message: ";
    } else if (log_level & G_LOG_LEVEL_INFO) {
        s += "info: ";
    } else if (log_level & G_LOG_LEVEL_DEBUG) {
        s += "debug: ";
    }
    s += message;
    AVG_TRACE(Logger::WARNING, s);
}

NodeDefinition Words::getNodeDefinition()
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
            "small", "tt", "u"};
    vector<string> sChildren = vectorFromCArray(6, sChildArray); 
    return NodeDefinition("words", Node::buildNode<Words>)
        .extendDefinition(RasterNode::getNodeDefinition())
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
        .addArg(Arg<bool>("rawtextmode", false, false, offsetof(Words, m_bRawTextMode)));
}

static void
text_subst_func (FcPattern *pattern, gpointer data)
{
//  GimpText *text = GIMP_TEXT (data);

  FcPatternAddBool(pattern, FC_HINTING, true);
  FcPatternAddInteger(pattern, FC_HINT_STYLE, FC_HINT_MEDIUM);
  FcPatternAddInteger(pattern, FC_RGBA, FC_RGBA_NONE);
  FcPatternAddBool(pattern, FC_ANTIALIAS, true);
}

Words::Words (const ArgList& Args, Player * pPlayer, bool bFromXML)
    : RasterNode(pPlayer), 
      m_StringExtents(0,0),
      m_pFontDescription(0),
      m_pLayout(0),
      m_bFontChanged(true),
      m_bDrawNeeded(true)
{
    m_bParsedText = false;

    if (!s_pPangoContext) {
        pango_ft2_get_context(72, 72);
        
        PangoFT2FontMap *fontmap;
        fontmap = PANGO_FT2_FONT_MAP (pango_ft2_font_map_new());
        pango_ft2_font_map_set_resolution (fontmap, 72, 72);
        pango_ft2_font_map_set_default_substitute (fontmap, text_subst_func, 0, 0);
        s_pPangoContext = pango_ft2_font_map_create_context (fontmap);
        g_object_unref (fontmap);

        pango_context_set_language(s_pPangoContext,
                pango_language_from_string ("en_US"));
        pango_context_set_base_dir(s_pPangoContext, PANGO_DIRECTION_LTR);
    }

    Args.setMembers(this);
    setAlignment(Args.getArgVal<string>("alignment"));
    setText(Args.getArgVal<string>("text"));
    initFonts();
}

Words::~Words ()
{
    if (m_pFontDescription) {
        pango_font_description_free(m_pFontDescription);
    }
    if (m_pLayout) {
        g_object_unref(m_pLayout);
    }
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
    m_Color = colorStringToColor(m_sColorName);

    m_bFontChanged = true;
    m_bDrawNeeded = true;
    RasterNode::setRenderingEngines(pDisplayEngine, pAudioEngine);
}

void Words::disconnect()
{
    if (m_pFontDescription) {
        pango_font_description_free(m_pFontDescription);
        m_pFontDescription = 0;
    }
    RasterNode::disconnect();
}

string Words::getTypeStr ()
{
    return "Words";
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

double Words::getWidth() 
{
    drawString();
    return Node::getWidth();
}

double Words::getHeight()
{
    drawString();
    return Node::getHeight();
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

string Words::getAlignment() const
{
    switch(m_Alignment) {
        case PANGO_ALIGN_LEFT:
            return "left";
        case PANGO_ALIGN_CENTER:
            return "center";
        case PANGO_ALIGN_RIGHT:
            return "right";
    }
    return "";
}

const vector<string>& Words::getFontFamilies()
{
    static vector<string> sFonts;
    initFonts();
    if (s_ppFontFamilies == 0) {
        PangoFT2FontMap *fontmap;
        fontmap = PANGO_FT2_FONT_MAP (pango_ft2_font_map_new());
        pango_ft2_font_map_set_resolution (fontmap, 72, 72);
        pango_ft2_font_map_set_default_substitute (fontmap, text_subst_func, 0, 0);
        PangoContext * pContext = pango_ft2_font_map_create_context (fontmap);
        pango_context_set_language(pContext,
                pango_language_from_string("en_US"));
        PangoFontMap* pFontMap = pango_context_get_font_map(pContext);

        string sOldLang = "";
        getEnv("LC_CTYPE", sOldLang);
        setEnv("LC_CTYPE", "en-us");
        pango_font_map_list_families(pFontMap, &s_ppFontFamilies, &s_NumFontFamilies);
        setEnv("LC_CTYPE", sOldLang);
        for (int i=0; i<s_NumFontFamilies; ++i) {
            sFonts.push_back(pango_font_family_get_name(s_ppFontFamilies[i]));
        }
        sort(sFonts.begin(), sFonts.end());
    }
    return sFonts;
}

const vector<string>& Words::getFontVariants(const string& sFontName)
{
    PangoFontFamily * pCurFamily = getFontFamily(sFontName);
    PangoFontFace ** ppFaces;
    int numFaces;
    pango_font_family_list_faces (pCurFamily, &ppFaces, &numFaces);
    static vector<string> sVariants;
    for (int i=0; i<numFaces; ++i) {
        sVariants.push_back(pango_font_face_get_face_name(ppFaces[i]));
    }
    g_free(ppFaces);
    return sVariants;
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

PangoFontFamily * Words::getFontFamily(const string& sFamily)
{
    getFontFamilies();
    PangoFontFamily * pFamily = 0;
    assert(s_NumFontFamilies != 0);
    for (int i=0; i<s_NumFontFamilies; ++i) {
        if (equalIgnoreCase(pango_font_family_get_name(s_ppFontFamilies[i]), sFamily)) {
            pFamily = s_ppFontFamilies[i];
        }
    }
    if (!pFamily) {
        throw(Exception(AVG_ERR_INVALID_ARGS, 
                "getFontFamily: Font family "+sFamily+" not found."));
    }
    return pFamily;
}

void Words::parseString(PangoAttrList** ppAttrList, char** ppText)
{
    bool bOk;
    GError * pError = 0;
    bOk = (pango_parse_markup(m_sText.c_str(), int(m_sText.length()), 0,
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
    if (!m_bDrawNeeded) {
        return;
    }
    ScopeTimer Timer(DrawStringProfilingZone);
    if (m_sText.length() == 0) {
        m_StringExtents = IntPoint(0,0);
    } else {
        if (m_bFontChanged) {
            PangoFontFamily * pFamily;
            bool bFamilyFound = true;
            try {
                pFamily = getFontFamily(m_sFontName);
            } catch (Exception&) {
                if (s_sFontsNotFound.find(m_sFontName) == s_sFontsNotFound.end()) {
                    AVG_TRACE(Logger::WARNING, "Could not find font face " << 
                            m_sFontName << ". Using sans instead.");
                    s_sFontsNotFound.insert(m_sFontName);
                }
                bFamilyFound = false;
                pFamily = getFontFamily("sans");
            }
            PangoFontFace ** ppFaces;
            int numFaces;
            pango_font_family_list_faces (pFamily, &ppFaces, &numFaces);
            PangoFontFace * pFace = 0;
            if (m_sFontVariant == "") {
                pFace = ppFaces[0];
            } else {
                for (int i=0; i<numFaces; ++i) {
                    if (equalIgnoreCase(pango_font_face_get_face_name(ppFaces[i]), 
                            m_sFontVariant)) 
                    {
                        pFace = ppFaces[i];
                    }
                }
            }
            if (!pFace) {
                pFace = ppFaces[0];
                if (bFamilyFound) {
                    pair<string, string> variant(m_sFontName, m_sFontVariant);
                    if (s_VariantsNotFound.find(variant) == s_VariantsNotFound.end()) {
                        s_VariantsNotFound.insert(variant);
                        AVG_TRACE(Logger::WARNING, "Could not find font variant " 
                                << m_sFontName << ":" << m_sFontVariant << ". Using " <<
                                pango_font_face_get_face_name(pFace) << " instead.");
                    }
                }
            }
            g_free(ppFaces);

            if (m_pFontDescription) {
                pango_font_description_free(m_pFontDescription);
            }
            m_pFontDescription = pango_font_face_describe(pFace);
            pango_font_description_set_absolute_size(m_pFontDescription,
                    (int)(m_Size * PANGO_SCALE));

            m_bFontChanged = false;
        }
        pango_context_set_font_description(s_pPangoContext, m_pFontDescription);

        if (m_pLayout) {
            g_object_unref(m_pLayout);
        }
        m_pLayout = pango_layout_new (s_pPangoContext);

        if (m_bParsedText) {
            PangoAttrList * pAttrList = 0;
            char * pText = 0;
            parseString(&pAttrList, &pText);
            pango_layout_set_text (m_pLayout, pText, -1);
            pango_layout_set_attributes (m_pLayout, pAttrList);
            pango_attr_list_unref (pAttrList);
            g_free (pText);
        } else {
            pango_layout_set_text(m_pLayout, m_sText.c_str(), -1);
        }

        pango_layout_set_alignment(m_pLayout, m_Alignment);
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
        pango_layout_get_pixel_extents (m_pLayout, &ink_rect, &logical_rect);
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

            int yoffset = 0;
            if (ink_rect.y < 0) {
                yoffset = -ink_rect.y;
            }

            // Use 1 as x position here to make sure italic text is never cut off.
            pango_ft2_render_layout(&bitmap, m_pLayout, 1, yoffset);

            getDisplayEngine()->surfaceChanged(getSurface());
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
    drawString();
}

static ProfilingZone RenderProfilingZone("Words::render");

void Words::render(const DRect& Rect)
{
    ScopeTimer Timer(RenderProfilingZone);
    if (m_sText.length() != 0 && getEffectiveOpacity() > 0.001) {
        getDisplayEngine()->blta8(getSurface(), getSize(),
                getEffectiveOpacity(), m_Color, getBlendMode());
    }
}

Pixel32 Words::colorStringToColor(const string & colorString)
{
    int r,g,b;
    sscanf(colorString.c_str(), "%2x%2x%2x", &r, &g, &b);
    return Pixel32(r,g,b);
}

IntPoint Words::getMediaSize()
{
    drawString();
    return m_StringExtents;
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

void Words::checkFontError(int Ok, const string& sMsg)
{
    if (Ok == 0) {
        throw Exception(AVG_ERR_FONT_INIT_FAILED, sMsg);
    }
}

void Words::initFonts()
{
    if (!s_bInitialized) {
        g_type_init();
        std::string sFontConfPath = "/etc/fonts/fonts.conf"; 
        if (!fileExists(sFontConfPath)) {
            sFontConfPath = getAvgLibPath()+"etc/fonts/fonts.conf";
        }
        FcConfig * pConfig = FcConfigCreate();
        int Ok = (int)FcConfigParseAndLoad(pConfig, (const FcChar8 *)(sFontConfPath.c_str()), true);
        checkFontError(Ok, string("Font error: could not load config file ")+sFontConfPath);
        Ok = (int)FcConfigBuildFonts(pConfig);
        checkFontError(Ok, string("Font error: FcConfigBuildFonts failed."));
        Ok = (int)FcConfigSetCurrent(pConfig);
        checkFontError(Ok, string("Font error: FcConfigSetCurrent failed."));
        Ok = (int)FcConfigAppFontAddDir(pConfig, (const FcChar8 *)"fonts/");
        checkFontError(Ok, string("Font error: FcConfigAppFontAddDir failed."));
/*
        FcStrList * pCacheDirs = FcConfigGetCacheDirs(pConfig);
        FcChar8 * pDir;
        do {
            pDir = FcStrListNext(pCacheDirs);
            if (pDir) {
                cerr << pDir << endl;
            }
        } while (pDir);
*/
        g_log_set_default_handler(GLibLogFunc, 0);

        s_bInitialized = true;
    }
}

}

