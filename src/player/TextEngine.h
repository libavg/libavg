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

#ifndef _TextEngine_H_
#define _TextEngine_H_

#include <pango/pango.h>
#include <fontconfig/fontconfig.h>

#include <vector>
#include <string>
#include <set>
#include <map>

namespace avg {

class TextEngine {
public:
    static TextEngine& get(bool bHint);
    virtual ~TextEngine();

    PangoContext * getPangoContext();

    const std::vector<std::string>& getFontFamilies();
    const std::vector<std::string>& getFontVariants(const std::string& sFontName);
    void addFontDir(const std::string& sDir);

    PangoFontDescription * getFontDescription(const std::string& sFamily, 
            const std::string& sVariant);

private:
    TextEngine(bool bHint);
    void init();
    void deinit();
    void initFonts();
    PangoFontFamily * getFontFamily(const std::string& sFamily);

    void checkFontError(int Ok, const std::string& sMsg);

    bool m_bHint;
    PangoContext * m_pPangoContext;
    PangoFontMap * m_pFontMap;
    std::set<std::string> m_sFontsNotFound;
    std::set<std::pair<std::string, std::string> > m_VariantsNotFound;
    int m_NumFontFamilies;
    std::vector<std::string> m_sFonts;
    typedef std::map<std::pair<std::string, std::string>, PangoFontDescription* > 
            FontDescriptionCache;
    FontDescriptionCache m_FontDescriptionCache;
    PangoFontFamily** m_ppFontFamilies;
    std::vector<std::string> m_sFontDirs;

};

}
#endif
