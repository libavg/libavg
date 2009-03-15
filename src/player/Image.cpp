//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2008 Ulrich von Zadow
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

#include "Image.h"

#include "../base/Logger.h"
#include "../base/Exception.h"

#include "../graphics/Filterfliprgb.h"

#include "OGLTiledSurface.h"
#include "SDLDisplayEngine.h"

#include <Magick++.h>

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

Image::Image(const string& sFilename, bool bTiled)
    : m_sFilename(sFilename),
      m_pBmp(new Bitmap(IntPoint(1,1), R8G8B8X8)),
      m_pSurface(0),
      m_pEngine(0),
      m_State(NOT_AVAILABLE),
      m_bTiled(bTiled)
{
    load();
}

Image::Image(const Bitmap* pBmp, bool bTiled)
    : m_sFilename(""),
      m_pSurface(0),
      m_pEngine(0),
      m_State(CPU),
      m_bTiled(bTiled)
{
    setBitmap(pBmp);
}

Image::~Image()
{
    if (m_State == GPU) {
        delete m_pSurface;
        m_pSurface = 0;
    }
}
        
void Image::setBitmap(const Bitmap * pBmp)
{
    if(!pBmp) {
        throw Exception(AVG_ERR_UNSUPPORTED, "setBitmap(): bitmap must not be None!");
    }
    PixelFormat pf = calcSurfacePF(*pBmp);
    if (m_pEngine) {
        if (!m_pSurface) {
            if (m_bTiled) {
                m_pSurface = m_pEngine->createTiledSurface();
            } else {
                m_pSurface = new OGLSurface(m_pEngine);
            }
        }
        if (m_pSurface->getSize() != pBmp->getSize() || 
                m_pSurface->getPixelFormat() != pf)
        {
            m_pSurface->create(pBmp->getSize(), pf, true);
        }            
        BitmapPtr pSurfaceBmp = m_pSurface->lockBmp();
        pSurfaceBmp->copyPixels(*pBmp);
        m_pSurface->unlockBmps();
        m_pBmp=BitmapPtr();
        m_State = GPU;
    } else {
#if defined(__i386__) || defined(_WIN32)
        switch (pf) {
            case R8G8B8X8:
                pf = B8G8R8X8;
                break;
            case R8G8B8A8:
                pf = B8G8R8A8;
                break;
            default:
                break;
        }
#endif
        m_pBmp = BitmapPtr(new Bitmap(pBmp->getSize(), pf, ""));
        m_pBmp->copyPixels(*pBmp);
        m_State = CPU;
    }
    m_sFilename = "";

}

void Image::moveToGPU(SDLDisplayEngine* pEngine)
{
    m_pEngine = pEngine;
    if (m_State == CPU) {
        m_State = GPU;
        setupSurface();
    }
}

void Image::moveToCPU()
{
    if (m_State != GPU) {
        return;
    }
    BitmapPtr pSurfaceBmp = m_pSurface->lockBmp();
    m_pBmp = BitmapPtr(new Bitmap(pSurfaceBmp->getSize(), 
                pSurfaceBmp->getPixelFormat()));
    m_pBmp->copyPixels(*pSurfaceBmp);
    m_pSurface->unlockBmps();
#ifdef __i386__
    // XXX Yuck
    if (!m_pEngine->hasRGBOrdering() && m_pBmp->getBytesPerPixel() >= 3) {
        FilterFlipRGB().applyInPlace(m_pBmp);
    }
#endif
    m_State = CPU;
    m_pEngine = 0;
    delete m_pSurface;
    m_pSurface = 0;
}

void Image::setFilename(const std::string& sFilename)
{
    if (m_State == GPU) {
        delete m_pSurface;
        m_pSurface = 0;
    }
    m_State = NOT_AVAILABLE;
    m_pBmp = BitmapPtr(new Bitmap(IntPoint(1,1), R8G8B8X8));
    m_sFilename = sFilename;
    load();
    if (m_pEngine) {
        moveToGPU(m_pEngine);
    }
}

const string& Image::getFilename() const
{
    return m_sFilename;
}

Bitmap* Image::getBitmap()
{
    if (m_State == GPU) {
        Bitmap* pBmp = new Bitmap(*(m_pSurface->lockBmp()));
        m_pSurface->unlockBmps();
        return pBmp;
    } else {
        return new Bitmap(*m_pBmp);
    }
}

IntPoint Image::getSize()
{
    if (m_State == GPU) {
        return m_pSurface->getSize();
    } else {
        return m_pBmp->getSize();
    }
}

PixelFormat Image::getPixelFormat()
{
    if (m_State == GPU) {
        return m_pSurface->getPixelFormat();
    } else {
        return m_pBmp->getPixelFormat();
    }
}

OGLSurface* Image::getSurface()
{
    assert(m_State != CPU);
    return m_pSurface;
}

OGLTiledSurface* Image::getTiledSurface()
{
    return dynamic_cast<OGLTiledSurface*>(getSurface());
}

Image::State Image::getState()
{
    return m_State;
}

SDLDisplayEngine* Image::getEngine()
{
    return m_pEngine;
}

void Image::load()
{
    if (m_sFilename == "") {
        return;
    }
    AVG_TRACE(Logger::MEMORY, "Loading " << m_sFilename);
    m_pBmp = BitmapPtr(new Bitmap(m_sFilename));
    m_State = CPU;
}

void Image::setupSurface()
{
    PixelFormat pf = calcSurfacePF(*m_pBmp);
    if (m_bTiled) {
        m_pSurface = m_pEngine->createTiledSurface();
    } else {
        m_pSurface = new OGLSurface(m_pEngine);
    }
    m_pSurface->create(m_pBmp->getSize(), pf, true);
    BitmapPtr pSurfaceBmp = m_pSurface->lockBmp();
    pSurfaceBmp->copyPixels(*m_pBmp);
#ifdef __i386__
    if (!(m_pEngine->hasRGBOrdering())) {
        FilterFlipRGB().applyInPlace(pSurfaceBmp);
    }
#endif
    m_pSurface->unlockBmps();
    m_pBmp=BitmapPtr();
}

PixelFormat Image::calcSurfacePF(const Bitmap& bmp)
{
    PixelFormat pf;
    pf = R8G8B8X8;
    if (bmp.hasAlpha()) {
        pf = R8G8B8A8;
    }
    if (bmp.getPixelFormat() == I8) {
        pf = I8;
    }
    return pf;
}

}
