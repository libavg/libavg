//
// $Id$
// 


#include "AVGSDLFont.h"
#include "AVGException.h"

#include <paintlib/plbitmap.h>
#include <paintlib/Filter/plfilterfill.h>
#include <paintlib/plpixel8.h>
#include <paintlib/pldirectfbbmp.h>

#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <cstring>
#include <unistd.h>

using namespace std;

AVGSDLFont::AVGSDLFont(const std::string & Filename, int Size)
{
    // TTF_OpenFont crashes if the file doesn't exist, so we check beforehand.
    struct stat FileInfo;
    if (stat(Filename.c_str(), &FileInfo) == -1) {
        throw AVGException(AVG_ERR_FONT_INIT_FAILED, 
                string("Font file \"") + Filename + "\" not found: " 
                + strerror(errno));
    }
    m_pFont = TTF_OpenFont(Filename.c_str(), Size);
    if (!m_pFont) {
        throw AVGException(AVG_ERR_FONT_INIT_FAILED, 
                string("SDL Font init failed: ") + TTF_GetError());
    }
    
}

AVGSDLFont::~AVGSDLFont()
{
    TTF_CloseFont(m_pFont);
    m_pFont = 0;
}
 
void AVGSDLFont::render(PLBmp& Bmp, const std::string & Text)
{
    SDL_Color FGColor= { 0xFF, 0xFF, 0xFF, 0xFF };
    SDL_Color BGColor= { 0x00, 0x00, 0x00, 0xFF };

    SDL_Surface * pSurface = 
            TTF_RenderText_Shaded(m_pFont, Text.c_str(), FGColor, BGColor);
    Bmp.Create(pSurface->w, pSurface->h, 8, false, true, 
            (PLBYTE*)(pSurface->pixels), pSurface->pitch);
    SDL_FreeSurface(pSurface);
}


