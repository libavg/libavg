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

#include "WordsNode.h"
#include "OGLSurface.h"
#include "NodeDefinition.h"
#include "TextEngine.h"

#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/ScopeTimer.h"
#include "../base/XMLHelper.h"
#include "../base/StringHelper.h"
#include "../base/MathHelper.h"
#include "../base/ObjectCounter.h"

#include "../graphics/Filterfill.h"
#include "../graphics/GLContext.h"
#include "../graphics/GLTexture.h"
#include "../graphics/TextureMover.h"

#include <pango/pangoft2.h>

#include <iostream>
#include <algorithm>

using namespace std;

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
    NodeDefinition def = NodeDefinition("words", "rasternode", Node::buildNode<WordsNode>)
        .addChildren(sChildren)
        .addDTDElements(sDTDElements)
        .addArg(Arg<string>("font", "arial", false, offsetof(WordsNode, m_sFontName)))
        .addArg(Arg<string>("variant", "", false, offsetof(WordsNode, m_sFontVariant)))
        .addArg(Arg<UTF8String>("text", ""))
        .addArg(Arg<string>("color", "FFFFFF", false, offsetof(WordsNode, m_sColorName)))
        .addArg(Arg<float>("aagamma", 1.0f, false, offsetof(WordsNode, m_Gamma)))
        .addArg(Arg<float>("fontsize", 15, false, offsetof(WordsNode, m_FontSize)))
        .addArg(Arg<int>("indent", 0, false, offsetof(WordsNode, m_Indent)))
        .addArg(Arg<float>("linespacing", 0, false, offsetof(WordsNode, m_LineSpacing)))
        .addArg(Arg<string>("alignment", "left"))
        .addArg(Arg<string>("wrapmode", "word"))
        .addArg(Arg<bool>("justify", false, false, offsetof(WordsNode, m_bJustify)))
        .addArg(Arg<bool>("rawtextmode", false, false, 
                offsetof(WordsNode, m_bRawTextMode)))
        .addArg(Arg<float>("letterspacing", 0, false, 
                offsetof(WordsNode, m_LetterSpacing)))
        .addArg(Arg<bool>("hint", true, false, offsetof(WordsNode, m_bHint)))
        ;
    NodeRegistry::get()->registerNodeType(def);
}

WordsNode::WordsNode(const ArgList& args)
    : m_LogicalSize(0,0),
      m_pFontDescription(0),
      m_pLayout(0),
      m_bRenderNeeded(true)
{
    m_bParsedText = false;

    args.setMembers(this);
    setAlignment(args.getArgVal<string>("alignment"));
    setWrapMode(args.getArgVal<string>("wrapmode"));
    updateFont();
    setText(args.getArgVal<UTF8String>("text"));
    m_Color = colorStringToColor(m_sColorName);
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
    getSurface()->setAlphaGamma(m_Gamma);
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

void WordsNode::setAlignment(const string& sAlign)
{
    if (sAlign == "left") {
        m_Alignment = PANGO_ALIGN_LEFT;
    } else if (sAlign == "center") {
        m_Alignment = PANGO_ALIGN_CENTER;
    } else if (sAlign == "right") {
        m_Alignment = PANGO_ALIGN_RIGHT;
    } else {
        throw(Exception(AVG_ERR_UNSUPPORTED, 
                "WordsNode alignment "+sAlign+" not supported."));
    }
    updateLayout();
}

bool WordsNode::getJustify() const
{
    return m_bJustify;
}

void WordsNode::setJustify(bool bJustify)
{
    m_bJustify = bJustify;
    updateLayout();
}

float WordsNode::getLetterSpacing() const
{
    return m_LetterSpacing;
}

void WordsNode::setLetterSpacing(float letterSpacing)
{
    m_LetterSpacing = letterSpacing;
    updateLayout();
}

bool WordsNode::getHint() const
{
    return m_bHint;
}

void WordsNode::setHint(bool bHint)
{
    m_bHint = bHint;
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

void WordsNode::getElementsByPos(const glm::vec2& pos, vector<NodePtr>& pElements)
{
    glm::vec2 relPos = pos-glm::vec2(m_AlignOffset, 0);
    AreaNode::getElementsByPos(relPos, pElements);
}

const std::string& WordsNode::getFont() const
{
    return m_sFontName;
}

void WordsNode::setFont(const std::string& sName)
{
    m_sFontName = sName;
    updateFont();
}

const std::string& WordsNode::getFontVariant() const
{
    return m_sFontVariant;
}

void WordsNode::addFontDir(const std::string& sDir)
{
    TextEngine::get(true).addFontDir(sDir);
    TextEngine::get(false).addFontDir(sDir);
}

void WordsNode::setFontVariant(const std::string& sVariant)
{
    m_sFontVariant = sVariant;
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
    return m_sColorName;
}

void WordsNode::setColor(const string& sColor)
{
    m_sColorName = sColor;
    m_Color = colorStringToColor(m_sColorName);
    m_bRenderNeeded = true;
}

float WordsNode::getAAGamma() const
{
    return m_Gamma;
}

void WordsNode::setAAGamma(float gamma)
{
    m_Gamma = gamma;
    if (getState() == Node::NS_CANRENDER) {
        getSurface()->setAlphaGamma(m_Gamma);
    }
}

float WordsNode::getFontSize() const
{
    return m_FontSize;
}

void WordsNode::setFontSize(float size)
{
    if (size <= 1) {
        throw Exception(AVG_ERR_INVALID_ARGS, "Words node: Font size < 1 is illegal.");
    } 
    m_FontSize = size;
    updateFont();
}

int WordsNode::getIndent() const
{
    return m_Indent;
}

void WordsNode::setIndent(int indent)
{
    m_Indent = indent;
    updateLayout();
}

float WordsNode::getLineSpacing() const
{
    return m_LineSpacing;
}

void WordsNode::setLineSpacing(float lineSpacing)
{
    m_LineSpacing = lineSpacing;
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
    return pango_layout_get_line_count(m_pLayout);
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
    updateLayout();
}

string WordsNode::getWrapMode() const
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
        m_pFontDescription = TextEngine::get(m_bHint).getFontDescription(m_sFontName, 
                m_sFontVariant);
        pango_font_description_set_absolute_size(m_pFontDescription,
                (int)(m_FontSize * PANGO_SCALE));
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
        PangoContext* pContext = TextEngine::get(m_bHint).getPangoContext();
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

        pango_layout_set_wrap(m_pLayout, m_WrapMode);
        pango_layout_set_alignment(m_pLayout, m_Alignment);
        pango_layout_set_justify(m_pLayout, m_bJustify);
        if (getUserSize().x != 0) {
            pango_layout_set_width(m_pLayout, int(getUserSize().x * PANGO_SCALE));
        }
        pango_layout_set_indent(m_pLayout, m_Indent * PANGO_SCALE);
        if (m_Indent < 0) {
            // For hanging indentation, we add a tabstop to support lists
            PangoTabArray* pTabs = pango_tab_array_new_with_positions(1, false,
                    PANGO_TAB_LEFT, -m_Indent * PANGO_SCALE);
            pango_layout_set_tabs(m_pLayout, pTabs);
            pango_tab_array_free(pTabs);
        }
        pango_layout_set_spacing(m_pLayout, (int)(m_LineSpacing*PANGO_SCALE));
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
    if (!(getState() == NS_CANRENDER)) {
        return;
    }
    if (m_bRenderNeeded) {
        if (m_sText.length() != 0) {
            ScopeTimer timer(RenderTextProfilingZone);
            int maxTexSize = GLContext::getMain()->getMaxTexSize();
            if (m_InkSize.x > maxTexSize || m_InkSize.y > maxTexSize) {
                throw Exception(AVG_ERR_UNSUPPORTED, 
                        "WordsNode size exceeded maximum (Size=" 
                        + toString(m_InkSize) + ", max=" + toString(maxTexSize) + ")");
            }
            GLTexturePtr pTex(new GLTexture(m_InkSize, A8));
            getSurface()->create(A8, pTex);
            TextureMoverPtr pMover = TextureMover::create(m_InkSize, A8, GL_DYNAMIC_DRAW);

            BitmapPtr pBmp = pMover->lock();
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
            switch (m_Alignment) {
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

            pMover->unlock();
            pMover->moveToTexture(*pTex);

            newSurface();
        }
        m_bRenderNeeded = false;
    }
}

void WordsNode::redraw()
{
    AVG_ASSERT(m_sText.length() < 32767);
    
    renderText();
}

void WordsNode::preRender(const VertexArrayPtr& pVA, bool bIsParentActive, 
        float parentEffectiveOpacity)
{
    Node::preRender(pVA, bIsParentActive, parentEffectiveOpacity);
    if (isVisible()) {
        redraw();
    }
    if (m_sText.length() != 0 && isVisible()) {
        renderFX(getSize(), m_Color, false);
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
        blta8(transform, glm::vec2(getSurface()->getSize()), getEffectiveOpacity(), 
                m_Color, getBlendMode());
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

