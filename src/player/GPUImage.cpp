//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2020 Ulrich von Zadow
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

#include "GPUImage.h"

#include "../base/Exception.h"
#include "../base/ObjectCounter.h"

#include "../graphics/BitmapLoader.h"
#include "../graphics/Bitmap.h"
#include "../graphics/ImageCache.h"
#include "../graphics/CachedImage.h"
#include "../graphics/GLContextManager.h"
#include "../graphics/Filterfliprgb.h"

#include "OGLSurface.h"
#include "OffscreenCanvas.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

GPUImage::GPUImage(OGLSurface * pSurface, bool bUseMipmaps)
    : m_sFilename(""),
      m_pSurface(pSurface),
      m_State(CPU),
      m_Source(NONE),
      m_bUseMipmaps(bUseMipmaps)
{
    ObjectCounter::get()->incRef(&typeid(*this));
    assertValid();
}

GPUImage::~GPUImage()
{
    unload();
    ObjectCounter::get()->decRef(&typeid(*this));
}
        
void GPUImage::moveToGPU()
{
    assertValid();
    if (m_State == CPU) {
        switch (m_Source) {
            case FILE:
                setupImageSurface();
                break;
            case BITMAP:
                setupBitmapSurface();
                break;
            case SCENE:
                m_pSurface->create(B8G8R8X8, m_pCanvas->getTex(), MCTexturePtr(),
                        MCTexturePtr(), MCTexturePtr(), true);
                break;
            case NONE:
                break;
            default:
                AVG_ASSERT(false);
        }
        m_State = GPU;
    }
    assertValid();
}

void GPUImage::moveToCPU()
{
    assertValid();
    if (m_State == GPU) {
        m_State = CPU;
        m_pSurface->destroy();
        if (m_pImage) {
            m_pImage->decTexRef();
        }
    }
    assertValid();
}

void GPUImage::setEmpty()
{
    assertValid();
    unload();
    changeSource(NONE);
    assertValid();
}

void GPUImage::setFilename(const std::string& sFilename, TexCompression comp)
{
    assertValid();
    CachedImagePtr pImage = ImageCache::get()->getImage(sFilename, comp);
    BitmapPtr pBmp = pImage->getBmp();
    if (comp == TEXCOMPRESSION_B5G6R5 && pBmp->hasAlpha()) {
        pImage->decBmpRef();
        throw Exception(AVG_ERR_UNSUPPORTED, 
                "B5G6R5-compressed textures with an alpha channel are not supported.");
    }
    unload();
    m_pImage = pImage;
    m_pBmp = m_pImage->getBmp();
    changeSource(FILE);

    m_sFilename = sFilename;

    if (m_State == GPU) {
        m_pSurface->destroy();
        setupImageSurface();
    }
    assertValid();
}

void GPUImage::setBitmap(BitmapPtr pBmp, TexCompression comp)
{
    assertValid();
    if (!pBmp) {
        throw Exception(AVG_ERR_UNSUPPORTED, "setBitmap(): bitmap must not be None!");
    }
    if (comp == TEXCOMPRESSION_B5G6R5 && pBmp->hasAlpha()) {
        throw Exception(AVG_ERR_UNSUPPORTED, 
                "B5G6R5-compressed textures with an alpha channel are not supported.");
    }
    unload();
    changeSource(BITMAP);
    m_pBmp = BitmapPtr(new Bitmap(pBmp->getSize(), pBmp->getPixelFormat(), ""));
    m_pBmp->copyPixels(*pBmp);
    if (comp == TEXCOMPRESSION_B5G6R5) {
        BitmapPtr pDestBmp = BitmapPtr(new Bitmap(pBmp->getSize(), B5G6R5, ""));
        if (!BitmapLoader::get()->isBlueFirst()) {
            FilterFlipRGB().applyInPlace(m_pBmp);
        }
        pDestBmp->copyPixels(*m_pBmp);
        m_pBmp = pDestBmp;
    }
    if (m_State == GPU) {
        setupBitmapSurface();
    }
    assertValid();
}

void GPUImage::setCanvas(OffscreenCanvasPtr pCanvas)
{
    assertValid();
    if (m_Source == SCENE && pCanvas == m_pCanvas) {
        return;
    }
    unload();
    changeSource(SCENE);
    m_pCanvas = pCanvas;
    if (m_State == GPU) {
        m_pSurface->create(B8G8R8X8, m_pCanvas->getTex(), MCTexturePtr(), MCTexturePtr(),
                MCTexturePtr(), true);
    }
    assertValid();
}

OffscreenCanvasPtr GPUImage::getCanvas() const
{
    return m_pCanvas;
}

const string& GPUImage::getFilename() const
{
    return m_sFilename;
}

BitmapPtr GPUImage::getBitmap()
{
    if (m_Source == NONE || m_Source == SCENE) {
        return BitmapPtr();
    } else {
        return m_pBmp;
    }
}

IntPoint GPUImage::getSize()
{
    switch (m_Source) {
        case NONE:
            return IntPoint(0,0);
        case SCENE:
            return m_pCanvas->getSize();
        case FILE:
        case BITMAP:
            return m_pBmp->getSize();
        default:
            AVG_ASSERT(false);
            return IntPoint(0,0);
    }
}

PixelFormat GPUImage::getPixelFormat()
{
    if (m_Source == FILE || m_Source == BITMAP) {
        return m_pBmp->getPixelFormat();
    } else {
        if (BitmapLoader::get()->isBlueFirst()) {
            return B8G8R8X8;
        } else {
            return R8G8B8X8;
        }
    }
}

OGLSurface* GPUImage::getSurface()
{
    AVG_ASSERT(m_State == GPU);
    return m_pSurface;
}

GPUImage::State GPUImage::getState()
{
    return m_State;
}

GPUImage::Source GPUImage::getSource()
{
    return m_Source;
}

void GPUImage::setupImageSurface()
{
    PixelFormat pf = m_pImage->getBmp()->getPixelFormat();
    m_pImage->incTexRef(m_bUseMipmaps);
    MCTexturePtr pTex = m_pImage->getTex();
    m_pSurface->create(pf, pTex);
}

void GPUImage::setupBitmapSurface()
{
    GLContextManager* pCM = GLContextManager::get();
    MCTexturePtr pTex = pCM->createTextureFromBmp(m_pBmp, m_bUseMipmaps);
    m_pSurface->create(m_pBmp->getPixelFormat(), pTex);
}

bool GPUImage::changeSource(Source newSource)
{
    if (newSource != m_Source) {
        switch (m_Source) {
            case NONE:
                break;
            case FILE:
            case BITMAP:
                m_sFilename = "";
                break;
            case SCENE:
                m_pCanvas = OffscreenCanvasPtr();
                break;
            default:
                AVG_ASSERT(false);
        }
        m_Source = newSource;
        return true;
    } else {
        return false;
    }
}

void GPUImage::unload()
{
    if (m_pImage) {
        if (m_State == GPU) {
            m_pImage->decTexRef();
            m_pSurface->destroy();
        }
        m_pImage->decBmpRef();
        m_pImage = CachedImagePtr();
    }
    m_pBmp = BitmapPtr();
    if (m_State == GPU && m_Source != NONE) {
        m_pSurface->destroy();
    }
}

void GPUImage::assertValid() const
{
    AVG_ASSERT(m_pSurface);
    AVG_ASSERT((m_Source == FILE) == (m_sFilename != ""));
    AVG_ASSERT((m_Source == SCENE) == bool(m_pCanvas));
    AVG_ASSERT((m_Source == FILE) == bool(m_pImage));
    AVG_ASSERT((m_Source == FILE || m_Source == BITMAP) == bool(m_pBmp));
    switch (m_State) {
        case CPU:
            AVG_ASSERT(!(m_pSurface->isCreated()));
            break;
        case GPU:
            if (m_Source != NONE) {
                AVG_ASSERT(m_pSurface->isCreated());
            } else {
                AVG_ASSERT(!m_pSurface->isCreated());
            }
            break;
        default:
            AVG_ASSERT(false);
    }
}

}
