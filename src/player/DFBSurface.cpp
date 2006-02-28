//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  Current versions can be found at www.libavg.de
//

#include "DFBSurface.h"
#include "Player.h"
#include "../graphics/Bitmap.h"

#include "../base/Logger.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

IDirectFB * DFBSurface::s_pDirectFB = 0;

void DFBSurface::SetDirectFB
  ( IDirectFB * pDirectFB
  )
{
  s_pDirectFB = pDirectFB;
}

DFBSurface::DFBSurface()
    : m_pSurface(0)
{
}

DFBSurface::~DFBSurface()
{
    if (m_pSurface) {
        DFBResult err = m_pSurface->Release(m_pSurface);
        if (err) {
            AVG_TRACE(Logger::WARNING, 
                    "Can't release DFB surface in ~DFBSurface.");
        }
        m_pSurface = 0;
    }
}

// Note that this function unlocks the surface after getting the pixel offsets,
// which is probably ok for system memory surfaces but will definitely break 
// for video memory surfaces.
void DFBSurface::create(const IntPoint& Size, PixelFormat pf, 
                bool bFastDownload)
{
    if (!s_pDirectFB) {
        AVG_TRACE(Logger::ERROR, 
                "DFBSurface::create() called before SetDirectFB(). Aborting.");
        exit(-1);
    }
    DFBSurfaceDescription Desc;
    Desc.flags = DFBSurfaceDescriptionFlags (DSDESC_CAPS | DSDESC_WIDTH | 
            DSDESC_HEIGHT | DSDESC_PIXELFORMAT);
    Desc.caps = DSCAPS_SYSTEMONLY;
    Desc.width = Size.x;
    Desc.height = Size.y;

    // We allow BGR and RGB pixel formats here and expect the surface user to
    // swap red and blue if an RGB format is passed in here.
    if (pf == B8G8R8A8 || pf == R8G8B8A8) {
        Desc.pixelformat = DSPF_ARGB;
    } else if (pf == B8G8R8X8 || pf == R8G8B8X8) {
        Desc.pixelformat = DSPF_RGB32;
    } else if (pf == B8G8R8 || pf == R8G8B8) {
        Desc.pixelformat = DSPF_RGB24;
    } else if (pf == B5G6R5 || pf == R5G6B5) {
        Desc.pixelformat = DSPF_RGB16;
    } else if (pf == I8) {
        Desc.pixelformat = DSPF_A8;
    } else {
        AVG_TRACE(Logger::ERROR, 
                "Illegal pixelformat " << Bitmap::getPixelFormatString(pf) <<
                " in DFBSurface::create(). Aborting.");
        exit(-1);
    }
    DFBResult err = s_pDirectFB->CreateSurface(s_pDirectFB, &Desc, &m_pSurface);
    if (err) {
        AVG_TRACE(Logger::ERROR, 
                "Error creating DirectFB surface in DFBSurface::create(). Aborting.");
        exit(-1);
    }
    void * pPixels;
    int Pitch;
    m_pSurface->Lock(m_pSurface, DFBSurfaceLockFlags (DSLF_READ | DSLF_WRITE), 
            &pPixels, &Pitch);
    m_pBmp = BitmapPtr(new Bitmap(Size, pf, 
                (unsigned char*)pPixels, Pitch, false));
    m_pSurface->Unlock(m_pSurface);
}

BitmapPtr DFBSurface::lockBmp(int i)
{
    // DFB surfaces are always locked.
    return m_pBmp;
}

void DFBSurface::createFromDFBSurface(IDirectFBSurface * pSurface,
                const IntRect * pSrcRect)
{
    DFBRectangle DFBRect;
    IntRect SrcRect;
    if (pSrcRect) {
        SrcRect = *pSrcRect;
    } else {
        int w, h;
        m_pSurface->GetSize(m_pSurface, &w, &h);
        SrcRect = IntRect(0,0,w,h);
    }
    DFBRect.x = SrcRect.tl.x;
    DFBRect.y = SrcRect.tl.x;
    DFBRect.w = SrcRect.Width();
    DFBRect.h = SrcRect.Height();

    DFBSurfacePixelFormat DFBPF;
    m_pSurface->GetPixelFormat(m_pSurface, &DFBPF); 

    PixelFormat pf;
    switch (DFBPF) 
    {
        case DSPF_ARGB:
            pf = B8G8R8A8;
            break;
        case DSPF_RGB32:
            pf = B8G8R8X8;
            break;
        case DSPF_RGB24:
            pf = B8G8R8;
            break;
        case DSPF_RGB16:
            pf = B5G6R5;
            break;
        case DSPF_A8:
            pf = I8;
            break;
        default:
            AVG_TRACE(Logger::ERROR, 
                    "Unsupported pixel format in DFBSurface::createFromDFBSurface(). Aborting.");
            exit(-1);
    }

    pSurface->GetSubSurface(pSurface, &DFBRect, &m_pSurface);
    void * pPixels;
    int Pitch;
    m_pSurface->Lock(m_pSurface, DFBSurfaceLockFlags (DSLF_READ | DSLF_WRITE), 
            &pPixels, &Pitch);
    
    m_pBmp = BitmapPtr(new Bitmap(SrcRect.tl, pf, (unsigned char*)pPixels, Pitch,
                false));
    m_pSurface->Unlock(m_pSurface);
}

IDirectFBSurface* DFBSurface::getSurface()
{
    return m_pSurface;
}

}
