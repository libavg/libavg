//
// $Id$
// 

#ifndef _AVGDFBFONT_H_
#define _AVGDFBFONT_H_

#include "IAVGFont.h"

#include <directfb.h>

class AVGDFBFont: public IAVGFont
{
	public:
        AVGDFBFont(IDirectFB * pDFB, const std::string & Filename, int Size);
        virtual ~AVGDFBFont();
        virtual void render(IAVGSurface& Surface, const std::string & Text);

    private:
        IDirectFBFont * m_pFont;
        IDirectFB * m_pDFB;
};

#endif 

