//
// $Id$
//

#include "AVGDFBDisplayEngine.h"
#include "AVGException.h"

#include <paintlib/plbitmap.h>
#include <paintlib/pldirectfbbmp.h>
#include <paintlib/plrect.h>

#include <directfb.h>

#include <iostream>
#include <unistd.h>

using namespace std;

AVGDFBDisplayEngine::AVGDFBDisplayEngine()
    : m_pPrimary(0),
      m_pDirectFB(0)
{
}

AVGDFBDisplayEngine::~AVGDFBDisplayEngine()
{
    teardown();
}

void dumpSurface (IDirectFBSurface * pSurf, const string & name)
{
    int w, h;
    cerr << "Surface: " << name << endl;

    pSurf->GetSize(pSurf, &w, &h);
    cerr << "  Size: " << w << "x" << h << endl;

    DFBRectangle rect;
    pSurf->GetVisibleRectangle(pSurf, &rect);
    cerr << "  VisibleRect: x: " << rect.x << ", y: " << rect.y << 
            ", w: " << rect.w << ", h: " << rect.h << endl;

    cerr.setf(ios::hex);
    DFBSurfaceCapabilities caps;
    pSurf->GetCapabilities(pSurf, &caps);
    cerr << "  Caps: " << caps << endl;

    DFBSurfacePixelFormat fmt;
    pSurf->GetPixelFormat(pSurf, &fmt);
    cerr << "  PixelFormat: " << fmt << endl;
    cerr.setf(ios::dec);
}


void AVGDFBDisplayEngine::init(int width, int height, bool isFullscreen)
{
    char ** argv = new (char *)[3];
    int argc = 1;
    argv[0] = strdup ("bogus_appname");
    if (!isFullscreen) {
        argc = 3;
        argv[1] = strdup ("--dfb:force-windowed");
        argv[2] = strdup ("--dfb:system=SDL");
    }
    DFBResult err;
    err = DirectFBInit (&argc, &argv);
    errorCheck(AVG_ERR_VIDEO_INIT_FAILED, err);
    err = DirectFBCreate (&m_pDirectFB);
    errorCheck(AVG_ERR_VIDEO_INIT_FAILED, err);
    if (isFullscreen) {
        err = m_pDirectFB->SetCooperativeLevel(m_pDirectFB, DFSCL_FULLSCREEN);
        errorCheck(AVG_ERR_VIDEO_INIT_FAILED, err);
    }

    PLDirectFBBmp::SetDirectFB(m_pDirectFB);
    m_Width = width;
    m_Height = height;
    m_IsFullscreen = isFullscreen;
    
    DFBSurfaceDescription Desc;
    Desc.flags = DFBSurfaceDescriptionFlags(DSDESC_CAPS);// | DSDESC_WIDTH | DSDESC_HEIGHT);
    Desc.caps =  DFBSurfaceCapabilities(DSCAPS_PRIMARY | DSCAPS_FLIPPING);
/*    Desc.width = m_Width;
    Desc.height = m_Height;
  */  
    err = m_pDirectFB->CreateSurface(m_pDirectFB, &Desc, &m_pPrimary);
/*    m_pPrimary->GetSize(m_pPrimary, &m_Width, &m_Height);
    cerr << "Primary surface size: " << m_Width << "x " << m_Height << endl;*/
    dumpSurface (m_pPrimary, "m_pPrimary");
    errorCheck(AVG_ERR_VIDEO_INIT_FAILED, err);
}

void AVGDFBDisplayEngine::teardown()
{
    m_pPrimary->Release(m_pPrimary);
    m_pDirectFB->Release(m_pDirectFB);
}

void AVGDFBDisplayEngine::setClipRect(const PLRect& rc)
{
/*
    SDL_Rect SDLRect;
    SDLRect.x = rc.tl.x;
    SDLRect.y = rc.tl.y;
    SDLRect.w = rc.Width();
    SDLRect.h = rc.Height();

    SDL_SetClipRect(m_pScreen->GetSurface(), &SDLRect);
*/
}

void AVGDFBDisplayEngine::render(PLBmp * pBmp, const PLPoint& pos, double opacity)
{
    PLDirectFBBmp * pDFBBmp = dynamic_cast<PLDirectFBBmp *>(pBmp);
    PLASSERT(pDFBBmp); // createSurface() should have been used to create 
                       // the bitmap.
    IDirectFBSurface * pSurf = pDFBBmp->GetSurface();
    if (pBmp->HasAlpha()) {
        m_pPrimary->SetBlittingFlags(m_pPrimary, DSBLIT_BLEND_ALPHACHANNEL);
    } else {
        m_pPrimary->SetBlittingFlags(m_pPrimary, DSBLIT_NOFX);    
    }
//    dumpSurface (pSurf, "pDFBBmp");
//    dumpSurface (m_pPrimary, "m_pPrimary");
    DFBResult err = m_pPrimary->Blit(m_pPrimary, pDFBBmp->GetSurface(), 0, 
            pos.x, pos.y);
    errorCheck(AVG_ERR_VIDEO_GENERAL, err);

/*
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
*/
}


void AVGDFBDisplayEngine::swapBuffers()
{
    DFBResult err = m_pPrimary->Flip(m_pPrimary, 0, DSFLIP_WAITFORSYNC);
    err = m_pPrimary->FillRectangle(m_pPrimary, 0, 0, m_Width, m_Height);
    errorCheck(AVG_ERR_VIDEO_GENERAL, err);

/*
    SDL_Flip(m_pScreen->GetSurface());
    SDL_SetClipRect(m_pScreen->GetSurface(), 0);
    SDL_Rect SDLRect;
    SDLRect.x = 0;
    SDLRect.y = 0;
    SDLRect.w = m_pScreen->GetWidth();
    SDLRect.h = m_pScreen->GetHeight();
    SDL_FillRect(m_pScreen->GetSurface(), &SDLRect, 0);
*/
}

PLBmp * AVGDFBDisplayEngine::createSurface()
{
    return new PLDirectFBBmp;
}

void AVGDFBDisplayEngine::errorCheck(int avgcode, DFBResult dfbcode) {
    if (dfbcode) {
        throw AVGException(avgcode, 
                DirectFBErrorString(dfbcode));
    }
}
