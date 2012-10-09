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

#ifndef _FontStyle_H_
#define _FontStyle_H_

#include "../api.h"

#include "ExportedObject.h"

#include "../graphics/Pixel32.h"

#include <pango/pango.h>
#include <boost/shared_ptr.hpp>

#include <string>

namespace avg {

class FontStyle;
typedef boost::shared_ptr<class FontStyle> FontStylePtr;

class AVG_API FontStyle: public ExportedObject
{
    public:
        static void registerType();
        
        FontStyle(const ArgList& args);
        FontStyle();
        virtual ~FontStyle();

        void setDefaultedArgs(const ArgList& args);

        const std::string& getFont() const;
        void setFont(const std::string& sName);

        const std::string& getFontVariant() const;
        void setFontVariant(const std::string& sVariant);
        
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

        PangoAlignment getAlignmentVal() const;
        PangoWrapMode getWrapModeVal() const;
        Pixel32 getColorVal() const;

    private:
        std::string m_sName;
        std::string m_sVariant;
        std::string m_sColorName;
        Pixel32 m_Color;
        float m_AAGamma;
        float m_Size;
        int m_Indent;
        float m_LineSpacing;
        PangoAlignment m_Alignment;
        PangoWrapMode m_WrapMode;
        bool m_bJustify;
        float m_LetterSpacing;
        bool m_bHint;

};

typedef boost::shared_ptr<class FontStyle> FontStylePtr;

}
#endif
