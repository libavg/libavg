//
// $Id$
//

#include "AVGSDLDisplayEngine.h"
#include "AVGException.h"

#include <paintlib/plbitmap.h>
#include <paintlib/plsdlbmp.h>
#include <paintlib/plrect.h>

AVGSDLDisplayEngine::AVGSDLDisplayEngine()
    : m_pScreen(0)
{
}

AVGSDLDisplayEngine::~AVGSDLDisplayEngine()
{
    teardown();
}

void AVGSDLDisplayEngine::init(int width, int height, bool isFullscreen)
{
    m_Width = width;
    m_Height = height;
    m_IsFullscreen = isFullscreen;

    int err = SDL_Init(SDL_INIT_VIDEO);
    if (err) {
        throw AVGException(AVG_ERR_VIDEO_INIT_FAILED, SDL_GetError());
    }
    int modeFlags = SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_HWACCEL;
    if (m_IsFullscreen) {
        modeFlags |= SDL_FULLSCREEN;
    }
    SDL_Surface * pSurface = SDL_SetVideoMode(m_Width, m_Height, 32, modeFlags);
    if (!pSurface) {
        throw AVGException(AVG_ERR_VIDEO_INIT_FAILED, SDL_GetError());
    }
    m_pScreen = new PLSDLBmp();
    m_pScreen->Attach(pSurface);
}

void AVGSDLDisplayEngine::teardown()
{
    if (m_pScreen) {
        delete m_pScreen;
        m_pScreen = 0;
        SDL_Quit();
    }
}

void AVGSDLDisplayEngine::setClipRect(const PLRect& rc)
{
    SDL_Rect SDLRect;
    SDLRect.x = rc.tl.x;
    SDLRect.y = rc.tl.y;
    SDLRect.w = rc.Width();
    SDLRect.h = rc.Height();

    SDL_SetClipRect(m_pScreen->GetSurface(), &SDLRect);
}

void AVGSDLDisplayEngine::render(PLBmp * pBmp, const PLPoint& pos, double opacity)
{
    PLSDLBmp * pSDLBmp = dynamic_cast<PLSDLBmp *>(pBmp);
    PLASSERT(pSDLBmp); // createSurface() should have been used to create 
                       // the bitmap.
    SDL_Rect src, dest;
    src.x = 0;
    src.y = 0;
    src.w = pSDLBmp->GetWidth();
    src.h = pSDLBmp->GetHeight();
    dest.x = pos.x;
    dest.y = pos.y;
    dest.w = pSDLBmp->GetWidth();
    dest.h = pSDLBmp->GetHeight();
    
//    if (opacity < 0.9999) {
        SDL_SetAlpha(pSDLBmp->GetSurface(), SDL_SRCALPHA, int(255*opacity));
//    } else {
//    }
    SDL_BlitSurface(pSDLBmp->GetSurface(), &src, m_pScreen->GetSurface(), &dest);
}


void AVGSDLDisplayEngine::swapBuffers()
{
    SDL_Flip(m_pScreen->GetSurface());
    SDL_SetClipRect(m_pScreen->GetSurface(), 0);
    SDL_Rect SDLRect;
    SDLRect.x = 0;
    SDLRect.y = 0;
    SDLRect.w = m_pScreen->GetWidth();
    SDLRect.h = m_pScreen->GetHeight();
    SDL_FillRect(m_pScreen->GetSurface(), &SDLRect, 0);
}

PLBmp * AVGSDLDisplayEngine::createSurface()
{
    return new PLSDLBmp();
}

