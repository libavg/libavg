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

#include <string>
#include <iostream>

namespace avg {

class ISurface;

class Words : public RasterNode
{
	public:
        Words();
        Words(const xmlNodePtr xmlNode, DivNode * pParent);
        virtual ~Words();
        
        virtual void initText(const std::string& sText);
        virtual void init (DisplayEngine * pEngine, DivNode * pParent,
                Player * pPlayer);
        virtual void initVisible();
        virtual void prepareRender(int time, const DRect& parent);
        virtual void render(const DRect& Rect);
        virtual std::string getTypeStr();

        const std::string& getFont() const
        {
            return m_FontName;
        }
        
        void setFont(const std::string& sName)
        {
            invalidate();
            m_FontName = sName;
            m_bFontChanged = true;
            m_bDrawNeeded = true;
            invalidate();
        }
            
        const std::string& getText() const 
        {
            return m_Text;
        }
        
        void setText(const std::string& sText)
        {
            if (m_Text != sText) {
                invalidate();
                m_Text = sText;
                m_bDrawNeeded = true;
                invalidate();
            }
        }

        const std::string& getColor() const
        {
            return m_ColorName;
        }

        void setColor(const std::string& sColor)
        {
            invalidate();
            m_ColorName = sColor;
            m_Color = colorStringToColor(m_ColorName);
            m_bDrawNeeded = true;
            invalidate();
        }

        double getSize() const
        {
            return m_Size;
        }
        
        void setSize(double Size)
        {
            invalidate();
            m_Size = Size;
            m_bFontChanged = true;
            m_bDrawNeeded = true;
            invalidate();
        }

        int getParaWidth() const
        {
            return m_ParaWidth;
        }
        
        void setParaWidth(int ParaWidth)
        {
            invalidate();
            m_ParaWidth = ParaWidth;
            m_bDrawNeeded = true;
            invalidate();
        }
        
        int getIndent() const
        {
            return m_Indent;
        }
        
        void setIndent(int Indent)
        {
            invalidate();
            m_Indent = Indent;
            m_bDrawNeeded = true;
            invalidate();
        }

        double getLineSpacing() const
        {
            return m_LineSpacing;
        }
        
        void setLineSpacing(double LineSpacing)
        {
            invalidate();
            m_LineSpacing = LineSpacing;
            m_bDrawNeeded = true;
            invalidate();
        }

        std::string getAlignment() const;
        void setAlignment(const std::string& sAlignment);
        bool getItalic() const
        {
            return m_bItalic;
        }
        
        void setItalic(bool bItalic)
        {
            invalidate();
            m_bItalic = bItalic;
            m_bFontChanged = true;
            m_bDrawNeeded = true;
            invalidate();
        }
        
        std::string getWeight() const;
        void setWeight(const std::string& sWeight);
        
        bool getSmallCaps() const
        {
            return m_bSmallCaps;
        }
        
        void setSmallCaps(bool bSmallCaps)
        {
            invalidate();
            m_bSmallCaps = bSmallCaps;
            m_bFontChanged = true;
            m_bDrawNeeded = true;
            invalidate();
        }
        
        std::string getStretch() const;
        void setStretch(const std::string& sStretch);


    protected:        
        virtual DPoint getPreferredMediaSize();
    
    private:
        void drawString();
        Pixel32 colorStringToColor(const std::string & colorString);
        std::string removeExcessSpaces(const std::string & sText);
        static void FT2SubstituteFunc (FcPattern *pattern, gpointer data);

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

        ISurface * m_pSurface;
        DPoint m_StringExtents;
        PangoContext * m_pContext;
        PangoFontDescription * m_pFontDescription;

        bool m_bFontChanged;
        bool m_bDrawNeeded;
        
};

}

#endif //_Words_H_

