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
        
        Words(const ArgList& Args, Player * pPlayer);
        virtual ~Words();
        
        virtual void initText(const std::string& sText);
        virtual void setRenderingEngines(DisplayEngine * pDisplayEngine, AudioEngine * pAudioEngine);
        virtual void disconnect();
        virtual void preRender();
        virtual void render(const DRect& Rect);
        virtual std::string getTypeStr();

        virtual double getWidth();
        virtual double getHeight();

        const std::string& getFont() const;
        void setFont(const std::string& sName);
            
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
        
        bool getItalic() const;
        void setItalic(bool bItalic);
        
        std::string getWeight() const;
        void setWeight(const std::string& sWeight);
        
        bool getSmallCaps() const;
        void setSmallCaps(bool bSmallCaps);
        
        std::string getStretch() const;
        void setStretch(const std::string& sStretch);

        static const std::vector<std::string>& getFonts();
        
        double getLastCharX() const;
        double getLastCharY() const;

    protected:        
        virtual DPoint getPreferredMediaSize();
    
    private:
        void drawString();
        void parseString(PangoAttrList** ppAttrList, char** ppText);
        Pixel32 colorStringToColor(const std::string & colorString);
        std::string removeExcessSpaces(const std::string & sText);
        static void checkFontError(int Ok, const std::string& sMsg);
        static void FT2SubstituteFunc(FcPattern *pattern, gpointer data);
        static void initFonts();

        // Exposed Attributes
        std::string m_FontName;
        std::string m_Text;
        std::string m_ColorName;
        Pixel32 m_Color;
        double m_Size;
        int m_ParaWidth;
        int m_Indent;
        double m_LineSpacing;
        PangoAlignment m_Alignment;
        PangoWeight m_Weight;
        bool m_bItalic;
        PangoStretch m_Stretch;
        bool m_bSmallCaps;

        DPoint m_StringExtents;
        PangoContext * m_pContext;
        PangoFontDescription * m_pFontDescription;

        bool m_bFontChanged;
        bool m_bDrawNeeded;

        static std::set<std::string> s_sFontsNotFound;
        static bool s_bInitialized;
        
        DPoint m_LastCharPos;
};

}

#endif //_Words_H_

