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

#ifndef _Words_H_
#define _Words_H_

#include "../api.h"
#include "RasterNode.h"
#include "../graphics/Pixel32.h"
#include "../base/UTF8String.h"

#include <pango/pango.h>

#include <string>
#include <vector>

namespace avg {

class AVG_API Words : public RasterNode
{
    public:
        static NodeDefinition createDefinition();
        
        Words(const ArgList& Args, bool bFromXMLNodeValue);
        virtual ~Words();
        
        virtual void setRenderingEngines(DisplayEngine * pDisplayEngine, 
                AudioEngine * pAudioEngine);
        virtual void disconnect(bool bKill);
        virtual void preRender();
        virtual void render(const DRect& Rect);

        virtual double getWidth();
        virtual double getHeight();

        void setTextFromNodeValue(const std::string& sText);

        const std::string& getFont() const;
        void setFont(const std::string& sName);

        const std::string& getFontVariant() const;
        void setFontVariant(const std::string& sVariant);

        const UTF8String& getText() const; 
        void setText(const UTF8String& sText);
        
        const std::string& getColor() const;
        void setColor(const std::string& sColor);
        
        double getFontSize() const;
        void setFontSize(double Size);
        
        int getParaWidth() const;
        void setParaWidth(int ParaWidth);
        
        int getIndent() const;
        void setIndent(int Indent);
        
        double getLineSpacing() const;
        void setLineSpacing(double LineSpacing);
        
        bool getRawTextMode() const;
        void setRawTextMode(bool RawTextMode);
        
        std::string getAlignment() const;
        void setAlignment(const std::string& sAlignment);
 
        std::string getWrapMode() const;
        void setWrapMode(const std::string& sWrapMode);

        bool getJustify() const;
        void setJustify(bool bJustify);

        double getLetterSpacing() const;
        void setLetterSpacing(double letterSpacing);

        IntPoint getGlyphPos(int i);
        IntPoint getGlyphSize(int i);
        virtual IntPoint getMediaSize();
    
        static const std::vector<std::string>& getFontFamilies();
        static const std::vector<std::string>& getFontVariants(
                const std::string& sFontName);
        static void addFontDir(const std::string& sDir);
        
    private:
        void drawString();
        void parseString(PangoAttrList** ppAttrList, char** ppText);
        void setParsedText(const UTF8String& sText);
        UTF8String applyBR(const UTF8String& sText);
        std::string removeExcessSpaces(const std::string & sText);
        PangoRectangle getGlyphRect(int i);

        // Exposed Attributes
        std::string m_sFontName;
        std::string m_sFontVariant;
        UTF8String m_sText;
        UTF8String m_sRawText;
        std::string m_sColorName;
        Pixel32 m_Color;
        double m_Size;
        int m_ParaWidth;
        int m_Indent;
        double m_LineSpacing;
        PangoAlignment m_Alignment;
        PangoWrapMode m_WrapMode;
        bool m_bJustify;
        double m_LetterSpacing;

        bool m_bParsedText;
        bool m_bRawTextMode;
        IntPoint m_StringExtents;
        IntPoint m_PosOffset;
        PangoFontDescription * m_pFontDescription;
        PangoLayout * m_pLayout;

        bool m_bFontChanged;
        bool m_bDrawNeeded;
};

}

#endif //_Words_H_

