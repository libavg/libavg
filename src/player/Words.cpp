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

#include "Words.h"
#include "DisplayEngine.h"
#include "ISurface.h"

#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/ScopeTimer.h"
#include "../base/XMLHelper.h"

#include "../graphics/Filterfill.h"

#include <pango/pangoft2.h>

#include <iostream>
#include <stdlib.h>
#include <algorithm>
#include <cctype> // for toupper

using namespace std;

namespace avg {

Words::Words ()
    : m_FontName("arial"),
      m_Text(""),
      m_ColorName("FFFFFF"),
      m_Size(15),
      m_ParaWidth(-1),
      m_Indent(0),
      m_LineSpacing(-1),
      m_Alignment(PANGO_ALIGN_LEFT),
      m_Weight(PANGO_WEIGHT_NORMAL),
      m_bItalic(false),
      m_Stretch(PANGO_STRETCH_NORMAL),
      m_bSmallCaps(false),
      m_pSurface(0),
      m_StringExtents(0,0),
      m_pContext(0),
      m_pFontDescription(0),
      m_bFontChanged(true),
      m_bDrawNeeded(true)
{
}

Words::Words (const xmlNodePtr xmlNode, Player * pPlayer)
    : RasterNode(xmlNode, pPlayer)
{
    m_FontName = getDefaultedStringAttr (xmlNode, "font", "arial");
    m_Text = getDefaultedStringAttr (xmlNode, "text", "");
    m_ColorName = getDefaultedStringAttr (xmlNode, "color", "FFFFFF");
    m_Size = getDefaultedIntAttr (xmlNode, "size", 15);
    m_ParaWidth = getDefaultedIntAttr (xmlNode, "parawidth", -1);
    m_Indent = getDefaultedIntAttr (xmlNode, "indent", 0);
    m_LineSpacing = getDefaultedDoubleAttr (xmlNode, "linespacing", -1);
    setAlignment(getDefaultedStringAttr (xmlNode, "alignment", "left"));
    setWeight(getDefaultedStringAttr (xmlNode, "weight", "normal"));
    m_bItalic = getDefaultedBoolAttr (xmlNode, "italic", false);
    setStretch(getDefaultedStringAttr (xmlNode, "stretch", "normal"));
    m_bSmallCaps = getDefaultedBoolAttr (xmlNode, "smallcaps", false);
}

Words::~Words ()
{
    if (m_pSurface) {
        delete m_pSurface;
        m_pSurface = 0;
        g_object_unref(m_pContext);
        pango_font_description_free(m_pFontDescription);
    }
}

void Words::initText(const string& sText)
{
    string sTemp = removeExcessSpaces(sText);
    if (sText.length() != 0) {
        m_Text = sTemp;
    }
}

static void
text_subst_func (FcPattern *pattern, gpointer data)
{
//  GimpText *text = GIMP_TEXT (data);

  FcPatternAddBool (pattern, FC_HINTING,true);
  FcPatternAddBool (pattern, FC_AUTOHINT, true);
  FcPatternAddBool (pattern, FC_ANTIALIAS, true);
}

void Words::setDisplayEngine (DisplayEngine * pEngine)
{
    m_Color = colorStringToColor(m_ColorName);
    m_pSurface = pEngine->createSurface();

    pango_ft2_get_context(72, 72);
    
    PangoFT2FontMap *fontmap;
    fontmap = PANGO_FT2_FONT_MAP (pango_ft2_font_map_new ());
    pango_ft2_font_map_set_resolution (fontmap, 72, 72);
    pango_ft2_font_map_set_default_substitute (fontmap, text_subst_func, 0, 0);
    m_pContext = pango_ft2_font_map_create_context (fontmap);
    g_object_unref (fontmap);

    pango_context_set_language(m_pContext,
            pango_language_from_string ("en_US"));
    pango_context_set_base_dir(m_pContext, PANGO_DIRECTION_LTR);
    m_pFontDescription = pango_font_description_new();
    
    m_bFontChanged = true;
    m_bDrawNeeded = true;
    RasterNode::setDisplayEngine(pEngine);
}

void Words::disconnect()
{
    if (m_pSurface) {
        delete m_pSurface;
        m_pSurface = 0;
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
    invalidate();
    if (sAlign == "left") {
        m_Alignment = PANGO_ALIGN_LEFT;
    } else if (sAlign == "center") {
        m_Alignment = PANGO_ALIGN_CENTER;
    } else if (sAlign == "right") {
        m_Alignment = PANGO_ALIGN_RIGHT;
    } else {
        // TODO: Throw exception.
    }

    m_bDrawNeeded = true;
    invalidate();
}

void Words::setWeight(const string& sWeight)
{
    invalidate();
    if (sWeight == "ultralight") {
        m_Weight = PANGO_WEIGHT_ULTRALIGHT;
    } else if (sWeight == "light") {
        m_Weight = PANGO_WEIGHT_LIGHT;
    } else if (sWeight == "normal") {
        m_Weight = PANGO_WEIGHT_NORMAL;
    } else if (sWeight == "semibold") {
        m_Weight = PANGO_WEIGHT_SEMIBOLD;
    } else if (sWeight == "bold") {
        m_Weight = PANGO_WEIGHT_BOLD;
    } else if (sWeight == "ultrabold") {
        m_Weight = PANGO_WEIGHT_ULTRABOLD;
    } else if (sWeight == "heavy") {
        m_Weight = PANGO_WEIGHT_HEAVY;
    } else {
        // TODO: Throw exception.
    }
    m_bFontChanged = true;
    m_bDrawNeeded = true;
    invalidate();
}

void Words::setStretch(const string& sStretch)
{
    invalidate();
    if (sStretch == "ultracondensed") {
        m_Stretch = PANGO_STRETCH_ULTRA_CONDENSED;
    } else if (sStretch == "extracondensed") {
        m_Stretch = PANGO_STRETCH_EXTRA_CONDENSED;
    } else if (sStretch == "condensed") {
        m_Stretch = PANGO_STRETCH_CONDENSED;
    } else if (sStretch == "semicondensed") {
        m_Stretch = PANGO_STRETCH_SEMI_CONDENSED;
    } else if (sStretch == "normal") {
        m_Stretch = PANGO_STRETCH_SEMI_CONDENSED;
    } else if (sStretch == "normal") {
        m_Stretch = PANGO_STRETCH_NORMAL;
    } else if (sStretch == "semiexpanded") {
        m_Stretch = PANGO_STRETCH_SEMI_EXPANDED;
    } else if (sStretch == "expanded") {
        m_Stretch = PANGO_STRETCH_EXPANDED;
    } else if (sStretch == "extraexpanded") {
        m_Stretch = PANGO_STRETCH_EXTRA_EXPANDED;
    } else if (sStretch == "ultraexpanded") {
        m_Stretch = PANGO_STRETCH_ULTRA_EXPANDED;
    } else {
        // TODO: Throw exception.
    }
    m_bFontChanged = true;
    m_bDrawNeeded = true;
    invalidate();
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

string Words::getWeight() const
{
    switch (m_Weight) {
        case PANGO_WEIGHT_ULTRALIGHT:
            return "ultralight";
        case PANGO_WEIGHT_LIGHT:
            return "light";
        case PANGO_WEIGHT_NORMAL:
            return "normal";
        case PANGO_WEIGHT_SEMIBOLD:
            return "semibold";
        case PANGO_WEIGHT_BOLD:
            return "bold";
        case PANGO_WEIGHT_ULTRABOLD:
            return "ultrabold";
        case PANGO_WEIGHT_HEAVY:
            return "heavy";
    }
    return "";
}

string Words::getStretch() const
{
    switch (m_Stretch) {
        case PANGO_STRETCH_ULTRA_CONDENSED:
            return "ultracondensed";
        case PANGO_STRETCH_EXTRA_CONDENSED:
            return "extracondensed";
        case PANGO_STRETCH_CONDENSED:
            return "condensed";
        case PANGO_STRETCH_SEMI_CONDENSED:
            return "semicondensed";
        case PANGO_STRETCH_NORMAL:
            return "normal";
        case PANGO_STRETCH_SEMI_EXPANDED:
            return "semiexpanded";
        case PANGO_STRETCH_EXPANDED:
            return "expanded";
        case PANGO_STRETCH_EXTRA_EXPANDED:
            return "extraexpanded";
        case PANGO_STRETCH_ULTRA_EXPANDED:
            return "ultraexpanded";
    }
    return "";
}

bool equalIgnoreCase(const string& s1, const string& s2) {
    string sUpper1;
    string sUpper2;
	transform(s1.begin(), s1.end(), std::back_inserter(sUpper1), (int(*)(int)) toupper);
	transform(s2.begin(), s2.end(), std::back_inserter(sUpper2), (int(*)(int)) toupper);
    return sUpper1 == sUpper2;
}

static ProfilingZone DrawStringProfilingZone("  Words::drawString");

void Words::drawString()
{
    if (!isDisplayAvailable()) {
        return;
    }
    ScopeTimer Timer(DrawStringProfilingZone);
    if (m_Text.length() == 0) {
        m_StringExtents = DPoint(0,0);
    } else {
        if (m_bFontChanged) {
            pango_font_description_set_family(m_pFontDescription,
                    g_strdup(m_FontName.c_str()));
            pango_font_description_set_style(m_pFontDescription,
                    m_bItalic?PANGO_STYLE_ITALIC:PANGO_STYLE_NORMAL);
            pango_font_description_set_variant(m_pFontDescription,
                    m_bSmallCaps?PANGO_VARIANT_SMALL_CAPS:PANGO_VARIANT_NORMAL);
            pango_font_description_set_weight(m_pFontDescription,
                    m_Weight);
            pango_font_description_set_stretch(m_pFontDescription,
                    m_Stretch);
            pango_font_description_set_absolute_size(m_pFontDescription,
                    (int)(m_Size * PANGO_SCALE));

            pango_context_set_font_description(m_pContext, m_pFontDescription);
            m_bFontChanged = false;
            
            // Test if the font is actually available and warn if not.
            PangoFontMap* pFontMap = pango_context_get_font_map(m_pContext);
            PangoFont* pUsedFont = pango_font_map_load_font(pFontMap, m_pContext,
                    m_pFontDescription);
            PangoFontDescription * pUsedDescription = pango_font_describe(pUsedFont);
            string sUsedName = pango_font_description_get_family(pUsedDescription);
            if (!equalIgnoreCase(sUsedName, m_FontName)) {
                AVG_TRACE(Logger::WARNING, "Could not find font face " << m_FontName <<
                        ". Using " << sUsedName << " instead.");
            } else {
                if (m_Weight != pango_font_description_get_weight(m_pFontDescription)) {
                    AVG_TRACE(Logger::WARNING, "Font face " << m_FontName <<
                        " not available in " << getWeight() << ".");
                }
            }
            pango_font_description_free(pUsedDescription);
        }

        PangoLayout *layout = pango_layout_new (m_pContext);
       
        {
            bool bOk;
            PangoAttrList * pAttrList = 0;
            char * pText = 0;
            GError * pError = 0;
            bOk = pango_parse_markup(m_Text.c_str(), m_Text.length(), 0,
                    &pAttrList, &pText, 0, &pError);
            if (!bOk) {
                throw Exception(AVG_ERR_CANT_PARSE_STRING,
                        string("Can't parse string '")+m_Text+"' in node with id '"+
                            getID()+"' ("+pError->message+")");
            }
            pango_layout_set_text (layout, pText, -1);
            pango_layout_set_attributes (layout, pAttrList);
            pango_attr_list_unref (pAttrList);
            g_free (pText);

//        pango_layout_set_markup(layout, m_Text.c_str(), m_Text.length());
        }
        pango_layout_set_alignment (layout, m_Alignment);
        pango_layout_set_width (layout, m_ParaWidth * PANGO_SCALE);
        if (m_LineSpacing != -1) {
            pango_layout_set_spacing(layout, (int)(m_LineSpacing*PANGO_SCALE));
        }
        PangoRectangle logical_rect;
        pango_layout_get_pixel_extents (layout, 0, &logical_rect);
        m_StringExtents.y = logical_rect.height;
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
        m_pSurface->create(IntPoint(m_StringExtents), I8, false);

        BitmapPtr pBmp = m_pSurface->lockBmp();
        FilterFill<unsigned char>(0).applyInPlace(pBmp);
        FT_Bitmap bitmap;
        bitmap.rows = (int)m_StringExtents.y;
        bitmap.width = (int)m_StringExtents.x;
        unsigned char * pLines = pBmp->getPixels();
        bitmap.pitch = pBmp->getStride();
        bitmap.buffer = pLines;
        bitmap.num_grays = 256;
        bitmap.pixel_mode = ft_pixel_mode_grays;

        // Use 1 as x-position here to make sure italic text is never cut off.
        pango_ft2_render_layout(&bitmap, layout, 1, 0);

        getEngine()->surfaceChanged(m_pSurface);
        if (m_LineSpacing == -1) {
            m_LineSpacing = pango_layout_get_spacing(layout)/PANGO_SCALE;
        }
        g_object_unref(layout);
    }
    m_bDrawNeeded = false;
    setViewport(-32767, -32767, -32767, -32767);
}

void Words::prepareRender(int time, const DRect& parent)
{
    if (m_bDrawNeeded) {
        drawString();
    }
    Node::prepareRender(time, parent);
}

static ProfilingZone RenderProfilingZone("    Words::render");

void Words::render(const DRect& Rect)
{
    ScopeTimer Timer(RenderProfilingZone);
    if (m_Text.length() != 0 && getEffectiveOpacity() > 0.001) {
        DRect TextPos = getAbsViewport();
        TextPos.tl.x--;   // Compensate for italic hack in call to pango_ft2_render_layout
        TextPos.br.x--; 
        getEngine()->blta8(m_pSurface, &TextPos,
                getEffectiveOpacity(), m_Color, getAngle(),
                getPivot(), getBlendMode());
    }
}

Pixel32 Words::colorStringToColor(const string & colorString)
{
    int r,g,b;
    sscanf(colorString.c_str(), "%2x%2x%2x", &r, &g, &b);
    return Pixel32(r,g,b);
}

DPoint Words::getPreferredMediaSize()
{
    if (m_bDrawNeeded) {
        drawString();
    }
    return m_StringExtents;
}

string Words::removeExcessSpaces(const string & sText)
{
    string s = sText;
    unsigned lastPos = s.npos;
    unsigned pos = s.find_first_of(" \n\r");
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


}
