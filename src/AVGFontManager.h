//
// $Id$
// 

#ifndef _AVGFONTMANAGER_H_
#define _AVGFONTMANAGER_H_

#include "IAVGFont.h"

#include <map>
#include <string>

class AVGFontManager
{
	public:
        AVGFontManager ();
        virtual ~AVGFontManager ();

        IAVGFont * getFont(const std::string& Name, int size);

    private:
        virtual IAVGFont * loadFont(const std::string& Filename, int Size) = 0;
        const std::string & getFontPath();

        typedef std::map<std::string, IAVGFont *> FontMap;
        FontMap m_FontMap;
};

#endif 

