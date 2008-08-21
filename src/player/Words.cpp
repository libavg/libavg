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

void GLibLogFunc(const gchar *log_domain, GLogLevelFlags log_level, 
        const gchar *message, gpointer unused_data)
{
    string s = "Pango ";
    if (log_level & G_LOG_LEVEL_ERROR) {
        s += "error: ";
    } else if (log_level & G_LOG_LEVEL_CRITICAL) {
        s += "critical: ";
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
    static const string sChildren = "(#PCDATA|span|b|big|i|s|sub|sup|small|tt|u|br)*";
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
        "  lang CDATA #IMPLIED >\n"

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
        .addArg(Arg<string>("alignment", "left"));
}

Words::Words (const ArgList& Args, Player * pPlayer, bool bFromXML)
    : RasterNode(pPlayer), 
      m_StringExtents(0,0),
      m_pContext(0), 
      m_pFontDescription(0),
      m_bFontChanged(true),
      m_bDrawNeeded(true),
      m_LastCharPos(0,0)
{
    m_bParsedText = false;
    Args.setMembers(this);
    setAlignment(Args.getArgVal<string>("alignment"));
    if (bFromXML) {
        m_sText = Args.getArgVal<string>("text");
    } else {
        initText(Args.getArgVal<string>("text"));
    }
    initFonts();
}

Words::~Words ()
{
    if (m_pContext) {
        g_object_unref(m_pContext);
        m_pContext = 0;
        pango_font_description_free(m_pFontDescription);
    }
}

void Words::initText(const std::string& sText)
{
    string sTemp = removeExcessSpaces(sText);
    if (sText.length() != 0) {
        setParsedText(sTemp);
    }
}


static void
text_subst_func (FcPattern *pattern, gpointer data)
{
//  GimpText *text = GIMP_TEXT (data);

  FcPatternAddBool (pattern, FC_HINTING, true);
  FcPatternAddBool (pattern, FC_AUTOHINT, true);
  FcPatternAddBool (pattern, FC_ANTIALIAS, true);
}

void Words::setRenderingEngines(DisplayEngine * pDisplayEngine, AudioEngine * pAudioEngine)
{
    m_Color = colorStringToColor(m_sColorName);

    pango_ft2_get_context(72, 72);
    
    PangoFT2FontMap *fontmap;
    fontmap = PANGO_FT2_FONT_MAP (pango_ft2_font_map_new());
    pango_ft2_font_map_set_resolution (fontmap, 72, 72);
    pango_ft2_font_map_set_default_substitute (fontmap, text_subst_func, 0, 0);
    m_pContext = pango_ft2_font_map_create_context (fontmap);
    g_object_unref (fontmap);

    pango_context_set_language(m_pContext,
            pango_language_from_string ("en_US"));
    pango_context_set_base_dir(m_pContext, PANGO_DIRECTION_LTR);

    m_bFontChanged = true;
    m_bDrawNeeded = true;
    RasterNode::setRenderingEngines(pDisplayEngine, pAudioEngine);
}

void Words::disconnect()
{
    if (m_pContext) {
        g_object_unref(m_pContext);
        m_pContext = 0;
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

double Words::getLastCharX() const
{
    return m_LastCharPos.x;
}

double Words::getLastCharY() const
{
    return m_LastCharPos.y;
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

const std::string& Words::getText() const 
{
    return m_sText;
}

void Words::setText(const std::string& sText)
{
    if (m_sText != sText) {
        setParsedText(sText);
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

double Words::getSize() const
{
    return m_Size;
}

void Words::setSize(double Size)
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

bool equalIgnoreCase(const string& s1, const string& s2) {
    string sUpper1;
    string sUpper2;
    transform(s1.begin(), s1.end(), std::back_inserter(sUpper1), (int(*)(int)) toupper);
    transform(s2.begin(), s2.end(), std::back_inserter(sUpper2), (int(*)(int)) toupper);
    return sUpper1 == sUpper2;
}

static ProfilingZone FontFamilyProfilingZone("  Words::getFontFamily");

PangoFontFamily * Words::getFontFamily(const string& sFamily)
{
    ScopeTimer Timer(FontFamilyProfilingZone);
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
    string sTextWithoutBreaks = applyBR(m_sText);
    bool bOk;
    GError * pError = 0;
    bOk = (pango_parse_markup(sTextWithoutBreaks.c_str(), 
            int(sTextWithoutBreaks.length()), 0,
            ppAttrList, ppText, 0, &pError) != 0);
    if (!bOk) {
        throw Exception(AVG_ERR_CANT_PARSE_STRING,
                string("Can't parse string in node with id '")+
                getID()+"' ("+pError->message+")");
    }

}

static ProfilingZone DrawStringProfilingZone("  Words::drawString");
static ProfilingZone OpenFontProfilingZone("    Words::open font");

void Words::drawString()
{
    if (!m_bDrawNeeded || !isDisplayAvailable()) {
        return;
    }
    ScopeTimer Timer(DrawStringProfilingZone);
    if (m_sText.length() == 0) {
        m_StringExtents = IntPoint(0,0);
    } else {
        if (m_bFontChanged) {
            ScopeTimer Timer(OpenFontProfilingZone);
            AVG_TRACE(Logger::MEMORY, "Opening font " << m_sFontName);
            PangoFontFamily * pFamily;
            bool bFamilyFound = true;
            try {
                pFamily = getFontFamily(m_sFontName);
            } catch (Exception& ex) {
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

            pango_context_set_font_description(m_pContext, m_pFontDescription);
            m_bFontChanged = false;
        }

        PangoLayout *pLayout = pango_layout_new (m_pContext);

        if (m_bParsedText) {
            PangoAttrList * pAttrList = 0;
            char * pText = 0;
            parseString(&pAttrList, &pText);
            pango_layout_set_text (pLayout, pText, -1);
            pango_layout_set_attributes (pLayout, pAttrList);
            pango_attr_list_unref (pAttrList);
            g_free (pText);
        } else {
            pango_layout_set_text(pLayout, m_sText.c_str(), -1);
        }

        pango_layout_set_alignment(pLayout, m_Alignment);
        pango_layout_set_width(pLayout, m_ParaWidth * PANGO_SCALE);
        pango_layout_set_indent(pLayout, m_Indent * PANGO_SCALE);
        if (m_Indent < 0) {
            // For hanging indentation, we add a tabstop to support lists
            PangoTabArray* pTabs = pango_tab_array_new_with_positions(1, false,
                    PANGO_TAB_LEFT, -m_Indent * PANGO_SCALE);
            pango_layout_set_tabs(pLayout, pTabs);
            pango_tab_array_free(pTabs);
        }
        if (m_LineSpacing != -1) {
            pango_layout_set_spacing(pLayout, (int)(m_LineSpacing*PANGO_SCALE));
        }
        PangoRectangle logical_rect;
        PangoRectangle ink_rect;
        pango_layout_get_pixel_extents (pLayout, &ink_rect, &logical_rect);
//        cerr << "Ink: " << ink_rect.x << ", " << ink_rect.y << ", " 
//                << ink_rect.width << ", " << ink_rect.height << endl;
//        cerr << "Logical: " << logical_rect.x << ", " << logical_rect.y << ", " 
//                << logical_rect.width << ", " << logical_rect.height << endl;

        PangoRectangle lastPos;
        pango_layout_index_to_pos(pLayout, (int)m_sText.length(), &lastPos);
//        cerr << "ITOPOS: " << lastPos.x / PANGO_SCALE << ", " << lastPos.y / PANGO_SCALE << endl;
        m_LastCharPos.x = lastPos.x / PANGO_SCALE;
        m_LastCharPos.y = lastPos.y / PANGO_SCALE;
        
        m_StringExtents.y = logical_rect.height+2;
        m_StringExtents.x = m_ParaWidth;
        if (m_ParaWidth == -1) {
            m_StringExtents.x = logical_rect.width;
            // Work around what appears to be a pango bug when computing the 
            // extents of italic text by adding an arbritary amount to the width.
            m_StringExtents.x += m_Size/6+1;
        }
        if (m_StringExtents.x == 0) {
            m_StringExtents.x = 1;
        }
        if (m_StringExtents.y == 0) {
            m_StringExtents.y = 1;
        }
//        cerr << "libavg Extents: " << m_StringExtents << endl;
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
        pango_ft2_render_layout(&bitmap, pLayout, 1, yoffset);

        getDisplayEngine()->surfaceChanged(getSurface());
        if (m_LineSpacing == -1) {
            m_LineSpacing = pango_layout_get_spacing(pLayout)/PANGO_SCALE;
        }
        g_object_unref(pLayout);
        
    }
    m_bDrawNeeded = false;

    setViewport(-32767, -32767, -32767, -32767);
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
        getDisplayEngine()->blta8(getSurface(), getRelSize(),
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

void Words::setParsedText(const std::string& sText)
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

string Words::applyBR(const string& sText)
{
    string sResult(sText);
    string sLowerText = tolower(sResult); 
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

