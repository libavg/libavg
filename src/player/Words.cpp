//
// $Id$
//

#include "Words.h"
#include "IDisplayEngine.h"
#include "ISurface.h"

#include "../base/Exception.h"
#include "../base/ScopeTimer.h"
#include "../base/XMLHelper.h"

#include "../graphics/Filterfill.h"

#include <pango/pangoft2.h>

#include <iostream>
#include <stdlib.h>

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
      m_bFontChanged(true)
{
}

Words::Words (const xmlNodePtr xmlNode, Container * pParent)
    : RasterNode(xmlNode, pParent)
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

void Words::init (IDisplayEngine * pEngine, Container * pParent,
           Player * pPlayer)
{
    Node::init(pEngine, pParent, pPlayer);
    m_Color = colorStringToColor(m_ColorName);
    m_pSurface = getEngine()->createSurface();
    m_pContext = pango_ft2_get_context(72, 72);

    pango_context_set_language(m_pContext,
            pango_language_from_string ("en_US"));
    pango_context_set_base_dir(m_pContext, PANGO_DIRECTION_LTR);
    m_pFontDescription = pango_font_description_new();
}

void Words::initVisible ()
{
    Node::initVisible();
    m_bFontChanged = true;
    drawString();
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
    drawString();
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
    drawString();
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
    drawString();
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

static ProfilingZone DrawStringProfilingZone("  Words::drawString");

void Words::drawString()
{
    if (!isInitialized()) {
        return;
    }
    ScopeTimer Timer(DrawStringProfilingZone);
    if (m_Text.length() == 0) {
        m_StringExtents = DPoint(0,0);
    } else {
        if (m_bFontChanged) {
            // TODO: check if the family exists (via pango_font_map_list_families ()?)
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
            pango_font_description_set_size(m_pFontDescription,
                    (int)(m_Size * PANGO_SCALE));

            pango_context_set_font_description(m_pContext, m_pFontDescription);
            m_bFontChanged = false;
        }

        PangoRectangle logical_rect;
        PangoLayout *layout = pango_layout_new (m_pContext);
        pango_layout_set_markup(layout, m_Text.c_str(), m_Text.length());

        pango_layout_set_alignment (layout, m_Alignment);
        pango_layout_set_width (layout, m_ParaWidth * PANGO_SCALE);

        if (m_LineSpacing != -1) {
            pango_layout_set_spacing(layout, (int)(m_LineSpacing*PANGO_SCALE));
        }
        pango_layout_get_extents (layout, 0, &logical_rect);
        m_StringExtents.y = PANGO_PIXELS (logical_rect.height);
        m_StringExtents.x = m_ParaWidth;
        if (m_ParaWidth == -1) {
            m_StringExtents.x = PANGO_PIXELS(logical_rect.width);
        }
        m_pSurface->create(IntPoint(m_StringExtents), I8);

        BitmapPtr pBmp = m_pSurface->getBmp();
        FilterFill<unsigned char>(0).applyInPlace(pBmp);
        FT_Bitmap bitmap;
        bitmap.rows = (int)m_StringExtents.y;
        bitmap.width = (int)m_StringExtents.x;
        unsigned char * pLines = pBmp->getPixels();
        bitmap.pitch = pBmp->getStride();
        bitmap.buffer = pLines;
        bitmap.num_grays = 256;
        bitmap.pixel_mode = ft_pixel_mode_grays;

        pango_ft2_render_layout(&bitmap, layout, 0, 0);
        getEngine()->surfaceChanged(m_pSurface);
        if (m_LineSpacing == -1) {
            m_LineSpacing = pango_layout_get_spacing(layout)/PANGO_SCALE;
        }
        g_object_unref(layout);
    }
    setViewport(-32767, -32767, m_StringExtents.x, m_StringExtents.y);
}

static ProfilingZone RenderProfilingZone("    Words::render");

void Words::render(const DRect& Rect)
{
    ScopeTimer Timer(RenderProfilingZone);
    if (m_Text.length() != 0 && getEffectiveOpacity() > 0.001) {
//        bool bVisible = getEngine()->pushClipRect(getVisibleRect(), false);
//        if (bVisible) {
            getEngine()->blta8(m_pSurface, &getAbsViewport(),
                    getEffectiveOpacity(), m_Color, getAngle(),
                    getPivot(), getBlendMode());
//        }
//        getEngine()->popClipRect();
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
