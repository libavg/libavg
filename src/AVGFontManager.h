//
// $Id$
// 

#ifndef _AVGFONTMANAGER_H_
#define _AVGFONTMANAGER_H_

#include <directfb/directfb.h>

#include <map>
#include <string>

class AVGFontManager
{
	public:
        AVGFontManager ();
        virtual ~AVGFontManager ();

        IDirectFBFont * getFont(IDirectFB * pDFB, const std::string& Name, int size);


    private:
        IDirectFBFont * loadFont(IDirectFB * pDFB, const std::string& Name, int Size);
        const std::string & getFontPath();

        typedef std::map<std::string, IDirectFBFont *> FontMap;
        FontMap m_FontMap;
};

#endif 

