//
// $Id$
// 

#ifndef _Words_H_
#define _Words_H_

#include "RasterNode.h"

#include <paintlib/plpixel32.h>
#include <paintlib/plbitmap.h>

#include <pango/pango.h>
#include <fontconfig/fontconfig.h>

#include <string>

namespace avg {

class ISurface;

class Words : public RasterNode
{
	public:
        Words ();
        Words (const xmlNodePtr xmlNode, Container * pParent);
        virtual ~Words ();
        
        virtual void initText(const std::string& sText);
        virtual void init (IDisplayEngine * pEngine, Container * pParent,
                Player * pPlayer);
        virtual void initVisible();
        virtual void render (const DRect& Rect);
        virtual std::string getTypeStr ();

        const std::string& getFont() const
        {
            return m_FontName;
        }
        
        void setFont(const std::string& sName)
        {
            m_FontName = sName;
        }
            
        const std::string& getText() const 
        {
            return m_Text;
        }
        
        void setText(const std::string& sText)
        {
            m_Text = sText;
        }

        const std::string& getColor() const
        {
            return m_ColorName;
        }

        void setColor(const std::string& sColor)
        {
            m_ColorName = sColor;
            m_Color = colorStringToColor(m_ColorName);
        }

        double getSize() const
        {
            return m_Size;
        }
        
        void setSize(double Size)
        {
            m_Size = Size;
        }

        int getParaWidth() const
        {
            return m_ParaWidth;
        }
        
        void setParaWidth(int ParaWidth)
        {
            m_ParaWidth = ParaWidth;
        }
        
        int getIndent() const
        {
            return m_Indent;
        }
        
        void setIndent(int Indent)
        {
            m_Indent = Indent;
        }

        double getLineSpacing() const
        {
            return m_LineSpacing;
        }
        
        void setLineSpacing(double LineSpacing)
        {
            m_LineSpacing = LineSpacing;
        }

        std::string getAlignment() const;
        void setAlignment(const std::string& sAlignment);
        bool getItalic() const
        {
            return m_bItalic;
        }
        
        void setItalic(bool bItalic)
        {
            m_bItalic = bItalic;
        }
        
        std::string getWeight() const;
        void setWeight(const std::string& sWeight);
        
        bool getSmallCaps() const
        {
            return m_bSmallCaps;
        }
        
        void setSmallCaps(bool bSmallCaps)
        {
            m_bSmallCaps = bSmallCaps;
        }
        
        std::string getStretch() const;
        void setStretch(const std::string& sStretch);


    protected:        
        virtual DPoint getPreferredMediaSize();
    
    private:
        void drawString();
        PLPixel32 colorStringToColor(const std::string & colorString);
        std::string removeExcessSpaces(const std::string & sText);
        static void FT2SubstituteFunc (FcPattern *pattern, gpointer data);

        // Exposed Attributes
        std::string m_FontName;
        std::string m_Text;
        std::string m_ColorName;
        PLPixel32 m_Color;
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
        
};

}

#endif //_Words_H_

