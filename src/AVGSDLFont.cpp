//
// $Id$
// 


#include "AVGSDLFont.h"
#include "AVGException.h"
#include "IAVGSurface.h"
#include "AVGPlayer.h"
#include "AVGLogger.h"

#include <paintlib/plbitmap.h>
#include <paintlib/Filter/plfilterfill.h>
#include <paintlib/plpixel8.h>

#include <iostream>
#include <sstream>

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
        AVG_TRACE(AVGPlayer::DEBUG_ERROR, "Font file '" << Filename << "' not found: " 
                << strerror(errno) << ". Aborting.");
        exit(-1);
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
 
void AVGSDLFont::render(IAVGSurface& Surface, const std::string & Text)
{
    SDL_Color FGColor= { 0xFF, 0xFF, 0xFF, 0xFF };
    SDL_Color BGColor= { 0x00, 0x00, 0x00, 0xFF };

    SDL_Surface * pSDLSurface = 
            TTF_RenderUTF8_Shaded(m_pFont, Text.c_str(), FGColor, BGColor);
    Surface.create(pSDLSurface->w, pSDLSurface->h, 8, false);

    PLBYTE * pSrcPixels = (PLBYTE*)(pSDLSurface->pixels);
    PLBmpBase * pDestBmp = Surface.getBmp();
    PLBYTE ** ppDestLines = pDestBmp->GetLineArray();
    for (int y=0; y<pSDLSurface->h; y++) {
        memcpy (ppDestLines[y], pSrcPixels + y*pSDLSurface->pitch,
                pSDLSurface->w);
    }
    
    SDL_FreeSurface(pSDLSurface);
}


