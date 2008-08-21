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

#include "RasterNode.h"
#include "../graphics/Pixel32.h"

#include <pango/pango.h>
#include <fontconfig/fontconfig.h>

#include <set>
#include <string>
#include <iostream>
#include <vector>

namespace avg {

class ISurface;
 
class Words : public RasterNode
{
    public:
        static NodeDefinition getNodeDefinition();
        
        Words(const ArgList& Args, Player * pPlayer, bool bFromXML);
        virtual ~Words();
        
        virtual void initText(const std::string& sText);
        virtual void setRenderingEngines(DisplayEngine * pDisplayEngine, 
                AudioEngine * pAudioEngine);
        virtual void disconnect();
        virtual void preRender();
        virtual void render(const DRect& Rect);
        virtual std::string getTypeStr();

        virtual double getWidth();
        virtual double getHeight();

        const std::string& getFont() const;
        void setFont(const std::string& sName);

        const std::string& getFontVariant() const;
        void setFontVariant(const std::string& sVariant);

        const std::string& getText() const; 
        void setText(const std::string& sText);
        
        const std::string& getColor() const;
        void setColor(const std::string& sColor);
        
        double getSize() const;
        void setSize(double Size);
        
        int getParaWidth() const;
        void setParaWidth(int ParaWidth);
        
        int getIndent() const;
        void setIndent(int Indent);
        
        double getLineSpacing() const;
        void setLineSpacing(double LineSpacing);
        
        std::string getAlignment() const;
        void setAlignment(const std::string& sAlignment);
        
        static const std::vector<std::string>& getFontFamilies();
        static const std::vector<std::string>& getFontVariants(
                const std::string& sFontName);
        
        double getLastCharX() const;
        double getLastCharY() const;
        virtual IntPoint getMediaSize();
    
    private:
        void drawString();
        void parseString(PangoAttrList** ppAttrList, char** ppText);
        Pixel32 colorStringToColor(const std::string & colorString);
        void setParsedText(const std::string& sText);
        std::string applyBR(const std::string& sText);
        std::string removeExcessSpaces(const std::string & sText);
        static PangoFontFamily * getFontFamily(const std::string& sFamily);
        static void checkFontError(int Ok, const std::string& sMsg);
        static void FT2SubstituteFunc(FcPattern *pattern, gpointer data);
        static void initFonts();

        // Exposed Attributes
        std::string m_sFontName;
        std::string m_sFontVariant;
        std::string m_sText;
        std::string m_sColorName;
        Pixel32 m_Color;
        double m_Size;
        int m_ParaWidth;
        int m_Indent;
        double m_LineSpacing;
        PangoAlignment m_Alignment;

        bool m_bParsedText;
        IntPoint m_StringExtents;
        PangoFontDescription * m_pFontDescription;

        bool m_bFontChanged;
        bool m_bDrawNeeded;

        static PangoContext * s_pPangoContext;
        static std::set<std::string> s_sFontsNotFound;
        static std::set<std::pair<std::string, std::string> > s_VariantsNotFound;
        static bool s_bInitialized;
        static int s_NumFontFamilies;
        static PangoFontFamily** s_ppFontFamilies;
        
        DPoint m_LastCharPos;
};

}

#endif //_Words_H_

