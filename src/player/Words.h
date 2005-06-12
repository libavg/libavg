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
        virtual ~Words ();
        
        virtual void initText(const std::string& sText);
        virtual void init (IDisplayEngine * pEngine, Container * pParent,
                Player * pPlayer);
        virtual void initVisible();
        virtual void render (const DRect& Rect);
        virtual std::string getTypeStr ();

        void setFont(const std::string& sName);
        void setText(const std::string& sText);
        void setColor(const std::string& sColor);
        void setSize(double Size);
        void setParaWidth(int ParaWidth);
        void setIndent(int Indent);
        void setLineSpacing(double LineSpacing);
        bool setAlignment(const std::string& sAlignment);
        void setItalic(bool bItalic);
        bool setWeight(const std::string& sWeight);
        void setSmallCaps(bool bSmallCaps);
        bool setStretch(const std::string& sStretch);

        std::string getAlignment();
        std::string getWeight();
        std::string getStretch();

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

