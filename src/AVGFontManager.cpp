//
// $Id$
// 

#include "AVGFontManager.h"
#include "AVGException.h"

#include <iostream>
#include <sstream>
#include <stdlib.h>

using namespace std;

AVGFontManager::AVGFontManager ()
{
}

AVGFontManager::~AVGFontManager ()
{
    // Release all fonts.
}

IDirectFBFont * AVGFontManager::getFont(IDirectFB * pDFB, const string& Name, int Size)
{
    FontMap::iterator it;
    stringstream s;
    s << Name << Size;
    string Desc(s.str());
    it = m_FontMap.find(Desc);
    if (it == m_FontMap.end()) {
        IDirectFBFont * pFont = loadFont(pDFB, Name, Size);
        m_FontMap.insert(FontMap::value_type(Desc, pFont));
        return pFont;
    } else {
        return (*it).second;
    }
}

IDirectFBFont * AVGFontManager::loadFont(IDirectFB * pDFB, const string& Name, int Size)
{
    DFBFontDescription fontDesc;

    fontDesc.flags = DFBFontDescriptionFlags(DFDESC_HEIGHT | DFDESC_ATTRIBUTES);
    fontDesc.height = Size;
    fontDesc.attributes = (DFBFontAttributes)0;

    string FontPath = getFontPath()+"/"+Name+".ttf";

    IDirectFBFont * pFont;
    DFBResult err = pDFB->CreateFont(pDFB, FontPath.c_str(), &fontDesc, &pFont);
    if (err) {
        throw AVGException(AVG_ERR_FONT_INIT_FAILED, DirectFBErrorString(err));
    }

    return pFont;
}

const string & AVGFontManager::getFontPath()
{
    static string FontPath;
    if (FontPath=="") {
        char * pFontPath = getenv("AVG_FONT_PATH");
        if (pFontPath) {
            FontPath = pFontPath;
        } else {
            cerr << "Warning: no font path specified." << endl;
        }
    }
    return FontPath;
}

