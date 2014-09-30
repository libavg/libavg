//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2014 Ulrich von Zadow
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

#include "WordsNode.h"
#include "OGLSurface.h"
#include "TypeDefinition.h"
#include "TextEngine.h"
#include "Canvas.h"

#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/ScopeTimer.h"
#include "../base/XMLHelper.h"
#include "../base/StringHelper.h"
#include "../base/MathHelper.h"
#include "../base/ObjectCounter.h"

#include "../graphics/Filterfill.h"
#include "../graphics/GLContext.h"
#include "../graphics/GLContextManager.h"
#include "../graphics/GLTexture.h"
#include "../graphics/TextureMover.h"

#include <pango/pangoft2.h>

#include <iostream>
#include <algorithm>

using namespace std;
using namespace boost;

namespace avg {

void WordsNode::registerType()
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
    TypeDefinition def = TypeDefinition("words", "rasternode", 
            ExportedObject::buildObject<WordsNode>)
        .addChildren(sChildren)
        .addDTDElements(sDTDElements)
        .addArg(Arg<string>("font", "sans"))
        .addArg(Arg<string>("variant", ""))
        .addArg(Arg<UTF8String>("text", ""))
        .addArg(Arg<string>("color", "FFFFFF"))
        .addArg(Arg<float>("aagamma", 1.0f))
        .addArg(Arg<float>("fontsize", 15))
        .addArg(Arg<int>("indent", 0, false))
        .addArg(Arg<float>("linespacing", 0))
        .addArg(Arg<string>("alignment", "left"))
        .addArg(Arg<string>("wrapmode", "word"))
        .addArg(Arg<bool>("justify", false))
        .addArg(Arg<bool>("rawtextmode", false, false, 
                offsetof(WordsNode, m_bRawTextMode)))
        .addArg(Arg<float>("letterspacing", 0))
        .addArg(Arg<bool>("hint", true))
        .addArg(Arg<FontStyle>("fontstyle", FontStyle()))
        ;
    TypeRegistry::get()->registerType(def);
}

WordsNode::WordsNode(const ArgList& args)
    : m_LogicalSize(0,0),
      m_pFontDescription(0),
      m_pLayout(0),
      m_bRenderNeeded(true)
{
    m_bParsedText = false;
    args.setMembers(this);

    m_FontStyle = args.getArgVal<FontStyle>("fontstyle");
    m_FontStyle.setDefaultedArgs(args);
#ifdef _WIN32
    if (m_FontStyle.getFont() == "sans") {
        m_FontStyle.setFont("Arial");
        m_FontStyle.setFontVariant("Regular");
    }
#endif
    updateFont();
    setText(args.getArgVal<UTF8String>("text"));
    
    ObjectCounter::get()->incRef(&typeid(*this));
}

WordsNode::~WordsNode()
{
    if (m_pFontDescription) {
        pango_font_description_free(m_pFontDescription);
    }
    if (m_pLayout) {
        g_object_unref(m_pLayout);
    }
    ObjectCounter::get()->decRef(&typeid(*this));
}

void WordsNode::setTextFromNodeValue(const string& sText)
{
    // Gives priority to Node Values only if they aren't empty
    UTF8String sTemp = removeExcessSpaces(sText);
    if (sTemp.length() != 0) {
        setText(sText);
    }
}

void WordsNode::connectDisplay()
{
    RasterNode::connectDisplay();
    getSurface()->setAlphaGamma(m_FontStyle.getAAGamma());
}

void WordsNode::connect(CanvasPtr pCanvas)
{
    RasterNode::connect(pCanvas);
    checkReload();
}

void WordsNode::disconnect(bool bKill)
{
    if (m_pFontDescription) {
        pango_font_description_free(m_pFontDescription);
        m_pFontDescription = 0;
        updateFont();
    }
    RasterNode::disconnect(bKill);
}

string WordsNode::getAlignment() const
{
    return m_FontStyle.getAlignment();
}

void WordsNode::setAlignment(const string& sAlign)
{
    m_FontStyle.setAlignment(sAlign);
    updateLayout();
}

bool WordsNode::getJustify() const
{
    return m_FontStyle.getJustify();
}

void WordsNode::setJustify(bool bJustify)
{
    m_FontStyle.setJustify(bJustify);
    updateLayout();
}

float WordsNode::getLetterSpacing() const
{
    return m_FontStyle.getLetterSpacing();
}

void WordsNode::setLetterSpacing(float letterSpacing)
{
    m_FontStyle.setLetterSpacing(letterSpacing);
    updateLayout();
}

bool WordsNode::getHint() const
{
    return m_FontStyle.getHint();
}

void WordsNode::setHint(bool bHint)
{
    m_FontStyle.setHint(bHint);
    updateLayout();
}

float WordsNode::getWidth() const
{
    return AreaNode::getWidth();
}

void WordsNode::setWidth(float width)
{
    AreaNode::setWidth(width);
    updateLayout();
}

float WordsNode::getHeight() const
{
    return AreaNode::getHeight();
}

void WordsNode::setHeight(float width)
{
    AreaNode::setHeight(width);
    updateLayout();
}

glm::vec2 WordsNode::getSize() const
{
    return AreaNode::getSize();
}

void WordsNode::setSize(const glm::vec2& pt)
{
    AreaNode::setSize(pt);
    updateLayout();
}

glm::vec2 WordsNode::toLocal(const glm::vec2& globalPos) const
{
    glm::vec2 localPos = globalPos - getRelViewport().tl - glm::vec2(m_AlignOffset, 0);
    return getRotatedPivot(localPos, -getAngle(), getPivot());
}

glm::vec2 WordsNode::toGlobal(const glm::vec2& localPos) const
{
    glm::vec2 alignPos = localPos + glm::vec2(m_AlignOffset, 0);
    glm::vec2 globalPos = getRotatedPivot(alignPos, getAngle(), getPivot());
    return globalPos + getRelViewport().tl;
}

const FontStyle& WordsNode::getFontStyle() const
{
    return m_FontStyle;
}

void WordsNode::setFontStyle(const FontStyle& fontStyle)
{
    m_FontStyle = fontStyle;
    updateFont();
}

const std::string& WordsNode::getFont() const
{
    return m_FontStyle.getFont();
}

void WordsNode::setFont(const std::string& sName)
{
    m_FontStyle.setFont(sName);
    updateFont();
}

const std::string& WordsNode::getFontVariant() const
{
    return m_FontStyle.getFontVariant();
}

void WordsNode::addFontDir(const std::string& sDir)
{
    TextEngine::get(true).addFontDir(sDir);
    TextEngine::get(false).addFontDir(sDir);
}

void WordsNode::setFontVariant(const std::string& sVariant)
{
    m_FontStyle.setFontVariant(sVariant);
    updateFont();
}

const UTF8String& WordsNode::getText() const 
{
    return m_sRawText;
}

void WordsNode::setText(const UTF8String& sText)
{
    if (sText.length() > 32767) {
        throw(Exception(AVG_ERR_INVALID_ARGS, 
                string("WordsNode::setText: string too long (") 
                        + toString(sText.length()) + ")"));
    }
    if (m_sRawText != sText) {
        m_sRawText = sText;
        m_sText = m_sRawText;
        if (m_bRawTextMode) {
            m_bParsedText = false;
            updateLayout();
        } else {
            setParsedText(sText);
        }
    }
}

const std::string& WordsNode::getColor() const
{
    return m_FontStyle.getColor();
}

void WordsNode::setColor(const string& sColor)
{
    m_FontStyle.setColor(sColor);
    updateLayout();
}

float WordsNode::getAAGamma() const
{
    return m_FontStyle.getAAGamma();
}

void WordsNode::setAAGamma(float gamma)
{
    m_FontStyle.setAAGamma(gamma);
    if (getState() == Node::NS_CANRENDER) {
        getSurface()->setAlphaGamma(gamma);
    }
    updateLayout();
}

float WordsNode::getFontSize() const
{
    return m_FontStyle.getFontSize();
}

void WordsNode::setFontSize(float size)
{
    m_FontStyle.setFontSize(size);
    updateFont();
}

int WordsNode::getIndent() const
{
    return m_FontStyle.getIndent();
}

void WordsNode::setIndent(int indent)
{
    m_FontStyle.setIndent(indent);
    updateLayout();
}

float WordsNode::getLineSpacing() const
{
    return m_FontStyle.getLineSpacing();
}

void WordsNode::setLineSpacing(float lineSpacing)
{
    m_FontStyle.setLineSpacing(lineSpacing);
    updateLayout();
}

bool WordsNode::getRawTextMode() const
{
    return m_bRawTextMode;
}

void WordsNode::setRawTextMode(bool rawTextMode)
{
    if (rawTextMode != m_bRawTextMode) {
        m_sText = m_sRawText;
        if (rawTextMode) {
            m_bParsedText = false;
        } else {
            setParsedText(m_sText);
        }
        m_bRawTextMode = rawTextMode;
        updateLayout();
    }
}

glm::vec2 WordsNode::getGlyphPos(int i)
{
    PangoRectangle rect = getGlyphRect(i);
    return glm::vec2(float(rect.x)/PANGO_SCALE, float(rect.y)/PANGO_SCALE);
}

glm::vec2 WordsNode::getGlyphSize(int i)
{
    PangoRectangle rect = getGlyphRect(i);
    return glm::vec2(float(rect.width)/PANGO_SCALE, float(rect.height)/PANGO_SCALE);
}

int WordsNode::getNumLines()
{
    if(m_sText.length() != 0) {
        return pango_layout_get_line_count(m_pLayout);
    }
    return 0;
}

PyObject* WordsNode::getCharIndexFromPos(glm::vec2 p)
{
    int index;
    int trailing;
    gboolean bXyToIndex = pango_layout_xy_to_index(m_pLayout,
                int(p.x*PANGO_SCALE), int(p.y*PANGO_SCALE), &index, &trailing);
    if (bXyToIndex) {
        const char* pText = pango_layout_get_text(m_pLayout);
        return Py_BuildValue("l",(g_utf8_pointer_to_offset(pText,pText+index)));
    } else {
        return Py_BuildValue("");
    }
}

std::string WordsNode::getTextAsDisplayed()
{
    return pango_layout_get_text(m_pLayout);
}

glm::vec2 WordsNode::getLineExtents(int line)
{
    if (line < 0 || line >= getNumLines()) {
        throw Exception(AVG_ERR_OUT_OF_RANGE, "WordsNode.getLineExtents: line index "
                +toString(line)+" is out of range.");
    }
    PangoRectangle logical_rect;
    PangoRectangle ink_rect;
    PangoLayoutLine *layoutLine = pango_layout_get_line_readonly(m_pLayout, line);
    pango_layout_line_get_pixel_extents(layoutLine, &ink_rect, &logical_rect);
    return glm::vec2(float(logical_rect.width), float(logical_rect.height));
}

void WordsNode::setWrapMode(const string& sWrapMode)
{
    m_FontStyle.setWrapMode(sWrapMode);
    updateLayout();
}

string WordsNode::getWrapMode() const
{
    return m_FontStyle.getWrapMode();
}

void WordsNode::parseString(PangoAttrList** ppAttrList, char** ppText)
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
            sError = string("Can't parse string in node with id '")+getID()+"' ("
                    +pError->message+")";
        } else {
            sError = string("Can't parse string '")+m_sRawText+"' ("+pError->message+")";
        }
        throw Exception(AVG_ERR_CANT_PARSE_STRING, sError);
    }

}

void WordsNode::calcMaskCoords()
{
    // Calculate texture coordinates for the mask texture, normalized to
    // the extents of the text.
    glm::vec2 normMaskSize;
    glm::vec2 normMaskPos;
    glm::vec2 mediaSize = glm::vec2(getMediaSize());
    glm::vec2 effMaskPos = getMaskPos()-glm::vec2(m_InkOffset);
    glm::vec2 maskSize = getMaskSize();
    
    if (maskSize == glm::vec2(0,0)) {
        normMaskSize = glm::vec2(getSize().x/mediaSize.x, getSize().y/mediaSize.y);
        normMaskPos = glm::vec2(effMaskPos.x/getSize().x, effMaskPos.y/getSize().y);
    } else {
        normMaskSize = glm::vec2(maskSize.x/mediaSize.x, maskSize.y/mediaSize.y);
        normMaskPos = glm::vec2(effMaskPos.x/getMaskSize().x, 
                effMaskPos.y/getMaskSize().y);
    }
/*    
    cerr << "calcMaskCoords" << endl;
    cerr << "  mediaSize: " << getMediaSize() << endl;
    cerr << "  effMaskPos: " << effMaskPos << endl;
    cerr << "  m_AlignOffset: " << m_AlignOffset << endl;
    cerr << "  maskSize: " << maskSize << endl;
    cerr << "  normMaskSize: " << normMaskSize << endl;
    cerr << "  normMaskPos: " << normMaskPos << endl;
*/    
    getSurface()->setMaskCoords(normMaskPos, normMaskSize);
}

static ProfilingZoneID UpdateFontProfilingZone("WordsNode: Update font");

void WordsNode::updateFont()
{
    {
        ScopeTimer timer(UpdateFontProfilingZone);

        if (m_pFontDescription) {
            pango_font_description_free(m_pFontDescription);
        }
        TextEngine& engine = TextEngine::get(m_FontStyle.getHint());
        m_pFontDescription = engine.getFontDescription(m_FontStyle.getFont(), 
                m_FontStyle.getFontVariant());
        pango_font_description_set_absolute_size(m_pFontDescription,
                (int)(m_FontStyle.getFontSize() * PANGO_SCALE));
    }
    updateLayout();
}

static ProfilingZoneID UpdateLayoutProfilingZone("WordsNode: Update layout");

void WordsNode::updateLayout()
{
    ScopeTimer timer(UpdateLayoutProfilingZone);

    if (m_sText.length() == 0) {
        m_LogicalSize = IntPoint(0,0);
        m_bRenderNeeded = true;
    } else {
        TextEngine& engine = TextEngine::get(m_FontStyle.getHint());
        PangoContext* pContext = engine.getPangoContext();
        pango_context_set_font_description(pContext, m_pFontDescription);

        if (m_pLayout) {
            g_object_unref(m_pLayout);
        }
        m_pLayout = pango_layout_new(pContext);

        PangoAttrList * pAttrList = 0;
#if PANGO_VERSION > PANGO_VERSION_ENCODE(1,18,2) 
        PangoAttribute * pLetterSpacing = pango_attr_letter_spacing_new
            (int(m_FontStyle.getLetterSpacing()*1024));
#endif
        if (m_bParsedText) {
            char * pText = 0;
            parseString(&pAttrList, &pText);
#if PANGO_VERSION > PANGO_VERSION_ENCODE(1,18,2) 
            // Workaround for pango bug.
            pango_attr_list_insert_before(pAttrList, pLetterSpacing);
#endif            
            pango_layout_set_text(m_pLayout, pText, -1);
            g_free(pText);
        } else {
            pAttrList = pango_attr_list_new();
#if PANGO_VERSION > PANGO_VERSION_ENCODE(1,18,2) 
            pango_attr_list_insert_before(pAttrList, pLetterSpacing);
#endif
            pango_layout_set_text(m_pLayout, m_sText.c_str(), -1);
        }
        pango_layout_set_attributes(m_pLayout, pAttrList);
        pango_attr_list_unref(pAttrList);

        pango_layout_set_wrap(m_pLayout, m_FontStyle.getWrapModeVal());
        pango_layout_set_alignment(m_pLayout, m_FontStyle.getAlignmentVal());
        pango_layout_set_justify(m_pLayout, m_FontStyle.getJustify());
        if (getUserSize().x != 0) {
            pango_layout_set_width(m_pLayout, int(getUserSize().x * PANGO_SCALE));
        }
        int indent = m_FontStyle.getIndent() * PANGO_SCALE;
        pango_layout_set_indent(m_pLayout, indent);
        if (indent < 0) {
            // For hanging indentation, we add a tabstop to support lists
            PangoTabArray* pTabs = pango_tab_array_new_with_positions(1, false,
                    PANGO_TAB_LEFT, -indent);
            pango_layout_set_tabs(m_pLayout, pTabs);
            pango_tab_array_free(pTabs);
        }
        pango_layout_set_spacing(m_pLayout, 
                (int)(m_FontStyle.getLineSpacing()*PANGO_SCALE));
        PangoRectangle logical_rect;
        PangoRectangle ink_rect;
        pango_layout_get_pixel_extents(m_pLayout, &ink_rect, &logical_rect);

        /*        
                  cerr << getID() << endl;
                  cerr << "Ink: " << ink_rect.x << ", " << ink_rect.y << ", " 
                  << ink_rect.width << ", " << ink_rect.height << endl;
                  cerr << "Logical: " << logical_rect.x << ", " << logical_rect.y << ", " 
                  << logical_rect.width << ", " << logical_rect.height << endl;
                  cerr << "User Size: " << getUserSize() << endl;
                  */        
        m_InkSize.y = ink_rect.height;
        if (getUserSize().x == 0) {
            m_InkSize.x = ink_rect.width;
        } else {
            m_InkSize.x = int(getUserSize().x);
        }
        if (m_InkSize.x == 0) {
            m_InkSize.x = 1;
        }
        if (m_InkSize.y == 0) {
            m_InkSize.y = 1;
        }
        m_LogicalSize.y = logical_rect.height;
        m_LogicalSize.x = logical_rect.width;
        m_InkOffset = IntPoint(ink_rect.x-logical_rect.x, ink_rect.y-logical_rect.y);
        m_bRenderNeeded = true;
        setViewport(-32767, -32767, -32767, -32767);
    }
}

static ProfilingZoneID RenderTextProfilingZone("WordsNode: render text");

void WordsNode::renderText()
{
    AVG_ASSERT(m_sText.length() < 32767);

    if (!(getState() == NS_CANRENDER)) {
        return;
    }
    if (m_bRenderNeeded) {
        if (m_sText.length() != 0) {
            ScopeTimer timer(RenderTextProfilingZone);
            TextEngine& engine = TextEngine::get(m_FontStyle.getHint());
            PangoContext* pContext = engine.getPangoContext();
            pango_context_set_font_description(pContext, m_pFontDescription);
            int maxTexSize = GLContext::getCurrent()->getMaxTexSize();
            if (m_InkSize.x > maxTexSize || m_InkSize.y > maxTexSize) {
                throw Exception(AVG_ERR_UNSUPPORTED, 
                        "WordsNode size exceeded maximum (Size=" 
                        + toString(m_InkSize) + ", max=" + toString(maxTexSize) + ")");
            }

            BitmapPtr pBmp(new Bitmap(m_InkSize, A8));
            FilterFill<unsigned char>(0).applyInPlace(pBmp);
            FT_Bitmap bitmap;
            bitmap.rows = m_InkSize.y;
            bitmap.width = m_InkSize.x;
            unsigned char * pLines = pBmp->getPixels();
            bitmap.pitch = pBmp->getStride();
            bitmap.buffer = pLines;
            bitmap.num_grays = 256;
            bitmap.pixel_mode = ft_pixel_mode_grays;

            PangoRectangle logical_rect;
            PangoRectangle ink_rect;
            pango_layout_get_pixel_extents(m_pLayout, &ink_rect, &logical_rect);
            pango_ft2_render_layout(&bitmap, m_pLayout, -ink_rect.x, -ink_rect.y);
            switch (m_FontStyle.getAlignmentVal()) {
                case PANGO_ALIGN_LEFT:
                    m_AlignOffset = 0;
                    break;
                case PANGO_ALIGN_CENTER:
                    m_AlignOffset = -logical_rect.width/2;
                    break;
                case PANGO_ALIGN_RIGHT:
                    m_AlignOffset = -logical_rect.width;
                    break;
                default:
                    AVG_ASSERT(false);
            }
            setRenderColor(m_FontStyle.getColorVal());

            GLContextManager* pCM = GLContextManager::get();
            MCTexturePtr pTex = pCM->createTextureFromBmp(pBmp);
            getSurface()->create(A8, pTex);
            newSurface();
        }
        m_bRenderNeeded = false;
    }
}

void WordsNode::preRender(const VertexArrayPtr& pVA, bool bIsParentActive, 
        float parentEffectiveOpacity)
{
    Node::preRender(pVA, bIsParentActive, parentEffectiveOpacity);
    if (isVisible()) {
        renderText();
    }
    if (m_sText.length() != 0 && isVisible()) {
        scheduleFXRender();
    }
    calcVertexArray(pVA);
}

static ProfilingZoneID RenderProfilingZone("WordsNode::render");

void WordsNode::render()
{
    ScopeTimer timer(RenderProfilingZone);
    if (m_sText.length() != 0 && isVisible()) {
        IntPoint offset = m_InkOffset + IntPoint(m_AlignOffset, 0);
        glm::mat4 transform;
        if (offset == IntPoint(0,0)) {
            transform = getTransform();
        } else {
            transform = glm::translate(getTransform(), glm::vec3(offset.x, offset.y, 0));
        }
        blta8(transform, glm::vec2(getSurface()->getSize()));
    }
}

IntPoint WordsNode::getMediaSize()
{
    return m_LogicalSize;
}

const vector<string>& WordsNode::getFontFamilies()
{
    return TextEngine::get(true).getFontFamilies();
}

const vector<string>& WordsNode::getFontVariants(const string& sFontName)
{
    return TextEngine::get(true).getFontVariants(sFontName);
}

string WordsNode::removeExcessSpaces(const string & sText)
{
    string s = sText;
    string::size_type lastPos = s.npos;
    string::size_type pos = s.find_first_of(" \n\r");
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
        
PangoRectangle WordsNode::getGlyphRect(int i)
{
    
    if (i >= int(g_utf8_strlen(m_sText.c_str(), -1)) || i < 0) {
        throw(Exception(AVG_ERR_INVALID_ARGS, 
                string("getGlyphRect: Index ") + toString(i) + " out of range."));
    }
    const char* pText = pango_layout_get_text(m_pLayout);
    char * pChar = g_utf8_offset_to_pointer(pText, i);
    int byteOffset = pChar-pText;
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

void WordsNode::setParsedText(const UTF8String& sText)
{
    m_sText = removeExcessSpaces(sText);

    // This just does a syntax check and throws an exception if appropriate.
    // The results are discarded.
    PangoAttrList * pAttrList = 0;
    char * pText = 0;
    parseString(&pAttrList, &pText);
    pango_attr_list_unref(pAttrList);
    g_free(pText);
    m_bParsedText = true;
    updateLayout();
}

UTF8String WordsNode::applyBR(const UTF8String& sText)
{
    UTF8String sResult(sText);
    UTF8String sLowerText = toLowerCase(sResult); 
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

