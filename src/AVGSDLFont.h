//
// $Id$
// 

#ifndef _AVGDFBFONT_H_
#define _AVGDFBFONT_H_

#include "IAVGFont.h"

#include <SDL/SDL_ttf.h>

class AVGSDLFont: public IAVGFont
{
	public:
        AVGSDLFont(const std::string & Filename, int Size);
        virtual ~AVGSDLFont();
        virtual void render(IAVGSurface& Surface, const std::string & Text);

    private:
        TTF_Font * m_pFont;
};

#endif 

