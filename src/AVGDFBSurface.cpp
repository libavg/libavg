//
// $Id$
//

#include "AVGDFBSurface.h"
#include "AVGPlayer.h"
#include "AVGLogger.h"

#include <paintlib/plrect.h>

#include <iostream>
#include <sstream>

using namespace std;

IDirectFB * AVGDFBSurface::s_pDirectFB = 0;

void AVGDFBSurface::SetDirectFB
  ( IDirectFB * pDirectFB
  )
{
  s_pDirectFB = pDirectFB;
}

AVGDFBSurface::AVGDFBSurface()
    : m_pSurface(0)
{
}

AVGDFBSurface::~AVGDFBSurface()
{
    if (m_pSurface) {
        DFBResult err = m_pSurface->Release(m_pSurface);
        if (err) {
            AVG_TRACE(AVGPlayer::DEBUG_WARNING, 
                    "Can't release DFB surface in ~AVGDFBSurface.");
        }
        m_pSurface = 0;
    }
}

// Note that this function unlocks the surface after getting the pixel offsets,
// which is probably ok for system memory surfaces but will definitely break 
// for video memory surfaces.
void AVGDFBSurface::create(int Width, int Height, int bpp, 
                bool bHasAlpha)
{
    if (!s_pDirectFB) {
        AVG_TRACE(AVGPlayer::DEBUG_ERROR, 
                "AVGDFBSurface::create() called before SetDirectFB(). Aborting.");
        exit(-1);
    }
    DFBSurfaceDescription Desc;
    Desc.flags = DFBSurfaceDescriptionFlags (DSDESC_CAPS | DSDESC_WIDTH | 
            DSDESC_HEIGHT | DSDESC_PIXELFORMAT);
    Desc.caps = DSCAPS_SYSTEMONLY;
    Desc.width = Width;
    Desc.height = Height;
    switch(bpp) 
    {
        case 32:
            if (bHasAlpha) { 
                Desc.pixelformat = DSPF_ARGB;
            } else {
                Desc.pixelformat = DSPF_RGB32;
            }
            break;
        case 24:
            Desc.pixelformat = DSPF_RGB24;
            break;
        case 16:
            Desc.pixelformat = DSPF_RGB16;
            break;
        case 8:
            Desc.pixelformat = DSPF_A8;
            break;
        default:
            AVG_TRACE(AVGPlayer::DEBUG_ERROR, 
                    "Illegal bpp in AVGDFBSurface::create(). Aborting.");
            exit(-1);
    }
    DFBResult err = s_pDirectFB->CreateSurface(s_pDirectFB, &Desc, &m_pSurface);
    if (err) {
        AVG_TRACE(AVGPlayer::DEBUG_ERROR, 
                "Error creating DirectFB surface in AVGDFBSurface::create(). Aborting.");
        exit(-1);
    }
    void * pPixels;
    int Pitch;
    m_pSurface->Lock(m_pSurface, DFBSurfaceLockFlags (DSLF_READ | DSLF_WRITE), 
            &pPixels, &Pitch);
    m_Bmp.Create(Width, Height, bpp, bHasAlpha, (PLBYTE*)pPixels, Pitch);
    m_pSurface->Unlock(m_pSurface);
}

PLBmpBase* AVGDFBSurface::getBmp()
{
    return &m_Bmp;
}

void AVGDFBSurface::createFromDFBSurface(IDirectFBSurface * pSurface,
                const PLRect * pSrcRect)
{
    DFBRectangle DFBRect;
    PLRect SrcRect;
    if (pSrcRect) {
        SrcRect = *pSrcRect;
    } else {
        int w, h;
        m_pSurface->GetSize(m_pSurface, &w, &h);
        SrcRect = PLRect(0,0,w,h);
    }
    DFBRect.x = SrcRect.tl.x;
    DFBRect.y = SrcRect.tl.x;
    DFBRect.w = SrcRect.Width();
    DFBRect.h = SrcRect.Height();

    DFBSurfacePixelFormat PixelFormat;
    m_pSurface->GetPixelFormat(m_pSurface, &PixelFormat); 

    bool bAlphaChannel = false;
    int bpp;
    switch (PixelFormat) 
    {
        case DSPF_ARGB:
            bAlphaChannel = true;
            bpp = 32;
            break;
        case DSPF_RGB32:
            bpp = 32;
            break;
        case DSPF_RGB24:
            bpp = 24;
            break;
        case DSPF_RGB16:
            bpp = 16;
            break;
        case DSPF_A8:
            bpp = 8;
            break;
        default:
            AVG_TRACE(AVGPlayer::DEBUG_ERROR, 
                    "Unsupported pixel format in AVGDFBSurface::create(). Aborting.");
            exit(-1);
    }

    pSurface->GetSubSurface(pSurface, &DFBRect, &m_pSurface);
    void * pPixels;
    int Pitch;
    m_pSurface->Lock(m_pSurface, DFBSurfaceLockFlags (DSLF_READ | DSLF_WRITE), 
            &pPixels, &Pitch);
    
    m_Bmp.Create(SrcRect.Width(), SrcRect.Height(), bpp, bAlphaChannel, 
            (PLBYTE*)pPixels, Pitch);
    m_pSurface->Unlock(m_pSurface);
}

IDirectFBSurface* AVGDFBSurface::getSurface()
{
    return m_pSurface;
}


