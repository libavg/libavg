//
// $Id$
// 

#include "AVGFontManager.h"
#include "AVGException.h"
#include "AVGPlayer.h"
#include "AVGLogger.h"

#include <iostream>
#include <sstream>
#include <stdlib.h>

using namespace std;

AVGFontManager::AVGFontManager (const string& sFontPath)
    : m_sFontPath(sFontPath)
{
}

AVGFontManager::~AVGFontManager ()
{
    FontMap::iterator it;
    for (it = m_FontMap.begin(); it != m_FontMap.end(); it++) {
        IAVGFont * pFont = (*it).second;
        delete pFont;
    }
}

IAVGFont * AVGFontManager::getFont(const string& Name, int Size)
{
    FontMap::iterator it;
    stringstream s;
    s << Name << Size;
    string Desc(s.str());
    it = m_FontMap.find(Desc);
    if (it == m_FontMap.end()) {
        string FontPath = m_sFontPath+"/"+Name+".ttf";
        AVG_TRACE(AVGPlayer::DEBUG_MEMORY, "Loading " << FontPath << ", size " << Size);
        IAVGFont * pFont = loadFont(FontPath, Size);
        m_FontMap.insert(FontMap::value_type(Desc, pFont));
        return pFont;
    } else {
        return (*it).second;
    }
}

