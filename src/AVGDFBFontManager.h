//
// $Id$
// 

#ifndef _AVGDFBFONTMANAGER_H_
#define _AVGDFBFONTMANAGER_H_

#include "AVGFontManager.h"

#include <directfb/directfb.h>

#include <map>
#include <string>

class AVGDFBFontManager: public AVGFontManager
{
	public:
        AVGDFBFontManager (IDirectFB * pDFB, const std::string& sFontPath);
        virtual ~AVGDFBFontManager ();

    private:
        virtual IAVGFont * loadFont(const std::string& Filename, int Size);
        IDirectFB * m_pDFB; 
};

#endif 

