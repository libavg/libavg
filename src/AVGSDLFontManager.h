//
// $Id$
// 

#ifndef _AVGSDLFONTMANAGER_H_
#define _AVGSDLFONTMANAGER_H_

#include "AVGFontManager.h"

#include <map>
#include <string>

class AVGSDLFontManager: public AVGFontManager
{
	public:
        AVGSDLFontManager ();
        virtual ~AVGSDLFontManager ();

    private:
        virtual IAVGFont * loadFont(const std::string& Filename, int Size);
};

#endif 

