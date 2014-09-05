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

#ifndef _WordsNode_H_
#define _WordsNode_H_

#include "../api.h"
#include "RasterNode.h"
#include "FontStyle.h"
#include "../graphics/Pixel32.h"
#include "../base/UTF8String.h"

#include <pango/pango.h>

#include <string>
#include <vector>

namespace avg {

class AVG_API WordsNode : public RasterNode
{
    public:
        static void registerType();
        
        WordsNode(const ArgList& args);
        virtual ~WordsNode();
        
        virtual void connectDisplay();
        virtual void connect(CanvasPtr pCanvas);
        virtual void disconnect(bool bKill);
        virtual void preRender(const VertexArrayPtr& pVA, bool bIsParentActive, 
                float parentEffectiveOpacity);
        virtual void render();

        virtual float getWidth() const;
        virtual void setWidth(float width);

        virtual float getHeight() const;
        virtual void setHeight(float width);

        virtual glm::vec2 getSize() const;
        virtual void setSize(const glm::vec2& pt);

        glm::vec2 toLocal(const glm::vec2& globalPos) const;
        glm::vec2 toGlobal(const glm::vec2& localPos) const;

        void setTextFromNodeValue(const std::string& sText);

        const FontStyle& getFontStyle() const;
        void setFontStyle(const FontStyle& fontStyle);

        const std::string& getFont() const;
        void setFont(const std::string& sName);

        const std::string& getFontVariant() const;
        void setFontVariant(const std::string& sVariant);

        const UTF8String& getText() const; 
        void setText(const UTF8String& sText);
        
        const std::string& getColor() const;
        void setColor(const std::string& sColor);
        
        virtual float getAAGamma() const;
        virtual void setAAGamma(float gamma);

        float getFontSize() const;
        void setFontSize(float size);
        
        int getIndent() const;
        void setIndent(int indent);
        
        float getLineSpacing() const;
        void setLineSpacing(float lineSpacing);
        
        bool getRawTextMode() const;
        void setRawTextMode(bool rawTextMode);
        
        std::string getAlignment() const;
        void setAlignment(const std::string& sAlignment);
 
        std::string getWrapMode() const;
        void setWrapMode(const std::string& sWrapMode);

        bool getJustify() const;
        void setJustify(bool bJustify);

        float getLetterSpacing() const;
        void setLetterSpacing(float letterSpacing);

        bool getHint() const;
        void setHint(bool bHint);

        glm::vec2 getGlyphPos(int i);
        glm::vec2 getGlyphSize(int i);
        virtual IntPoint getMediaSize();
        
        int getNumLines();
        PyObject* getCharIndexFromPos(glm::vec2 p);
        std::string getTextAsDisplayed();
        glm::vec2 getLineExtents(int line);
    
        static const std::vector<std::string>& getFontFamilies();
        static const std::vector<std::string>& getFontVariants(
                const std::string& sFontName);
        static void addFontDir(const std::string& sDir);

    private:
        virtual void calcMaskCoords();
        void updateFont();
        void updateLayout();
        void renderText();
        void parseString(PangoAttrList** ppAttrList, char** ppText);
        void setParsedText(const UTF8String& sText);
        UTF8String applyBR(const UTF8String& sText);
        std::string removeExcessSpaces(const std::string & sText);
        PangoRectangle getGlyphRect(int i);

        // Exposed Attributes
        FontStyle m_FontStyle;
        UTF8String m_sText;
        UTF8String m_sRawText;

        bool m_bParsedText;
        bool m_bRawTextMode;
        IntPoint m_LogicalSize;
        IntPoint m_InkOffset;
        IntPoint m_InkSize;
        int m_AlignOffset;
        PangoFontDescription * m_pFontDescription;
        PangoLayout * m_pLayout;

        bool m_bRenderNeeded;
};

}

#endif

