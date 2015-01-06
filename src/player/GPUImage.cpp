//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2014 Ulrich von Zadow
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

#include "../graphics/Filterfliprgb.h"
#include "../graphics/BitmapLoader.h"
#include "../graphics/Bitmap.h"
#include "../graphics/ImageRegistry.h"
#include "../graphics/Image.h"
#include "../graphics/GLContextManager.h"

#include "OGLSurface.h"
#include "OffscreenCanvas.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

GPUImage::GPUImage(OGLSurface * pSurface, const MaterialInfo& material)
    : m_sFilename(""),
      m_pSurface(pSurface),
      m_State(CPU),
      m_Source(NONE),
      m_Material(material)
{
    ObjectCounter::get()->incRef(&typeid(*this));
    assertValid();
}

GPUImage::~GPUImage()
{
    if (m_State == GPU && m_Source != NONE) {
        m_pSurface->destroy();
        if (m_pImage) {
            m_pImage->decTexRef();
        }
    }
    if (m_pImage) {
        m_pImage->decBmpRef();
        m_pImage = ImagePtr();
    }
    ObjectCounter::get()->decRef(&typeid(*this));
}
        
void GPUImage::moveToGPU()
{
    assertValid();
    if (m_State == CPU) {
        switch (m_Source) {
            case FILE:
            case BITMAP:
                setupSurface();
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

void GPUImage::discard()
{
    assertValid();
    setEmpty();
    m_State = CPU;
    assertValid();
}

void GPUImage::setEmpty()
{
    assertValid();
    if (m_State == GPU) {
        m_pSurface->destroy();
        if (m_pImage) {
            m_pImage->decTexRef();
        }
    }
    if (m_pImage) {
        m_pImage->decBmpRef();
        m_pImage = ImagePtr();
    }
    changeSource(NONE);
    assertValid();
}

void GPUImage::setFilename(const std::string& sFilename, TextureCompression comp)
{
    assertValid();
    ImagePtr pImage = ImageRegistry::get()->getImage(sFilename);
    BitmapPtr pBmp = pImage->getBmp();
    if (comp == TEXTURECOMPRESSION_B5G6R5 && pBmp->hasAlpha()) {
        pImage->decBmpRef();
        throw Exception(AVG_ERR_UNSUPPORTED, 
                "B5G6R5-compressed textures with an alpha channel are not supported.");
    }
    if (m_pImage) {
        if (m_State == GPU) {
            m_pImage->decTexRef();
        }
        m_pImage->decBmpRef();
    }
    m_pImage = pImage;
    changeSource(FILE);

    m_sFilename = sFilename;

    switch (comp) {
        case TEXTURECOMPRESSION_B5G6R5:
            /*
             * TODO
            m_pBmp = BitmapPtr(new Bitmap(pBmp->getSize(), B5G6R5, sFilename));
            if (!BitmapLoader::get()->isBlueFirst()) {
                FilterFlipRGB().applyInPlace(pBmp);
            }
            m_pBmp->copyPixels(*pBmp);
            */
            break;
        case TEXTURECOMPRESSION_NONE:
            break;
        default:
            assert(false);
    }

    if (m_State == GPU) {
        m_pSurface->destroy();
        setupSurface();
    }
    assertValid();
}

void GPUImage::setBitmap(BitmapPtr pBmp, TextureCompression comp)
{
    assertValid();
    if (!pBmp) {
        throw Exception(AVG_ERR_UNSUPPORTED, "setBitmap(): bitmap must not be None!");
    }
    if (comp == TEXTURECOMPRESSION_B5G6R5 && pBmp->hasAlpha()) {
        throw Exception(AVG_ERR_UNSUPPORTED, 
                "B5G6R5-compressed textures with an alpha channel are not supported.");
    }
    if (m_pImage) {
        if (m_State == GPU) {
            m_pImage->decTexRef();
            m_pSurface->destroy();
        }
        m_pImage->decBmpRef();
    }
    bool bSourceChanged = changeSource(BITMAP);
    PixelFormat pf;
    switch (comp) {
        case TEXTURECOMPRESSION_NONE:
            pf = pBmp->getPixelFormat();
            break;
        case TEXTURECOMPRESSION_B5G6R5:
            pf = B5G6R5;
            if (!BitmapLoader::get()->isBlueFirst()) {
                FilterFlipRGB().applyInPlace(pBmp);
            }
            break;
        default:
            assert(false);
    }
    m_pImage = ImagePtr(new Image(pBmp));
    if (m_State == GPU) {
        m_pImage->incTexRef();
        m_pSurface->create(pf, m_pImage->getTex());
/*
        MCTexturePtr pTex = m_pSurface->getTex();
        if (bSourceChanged || m_pSurface->getSize() != m_pBmp->getSize() ||
                m_pSurface->getPixelFormat() != pf)
        {
            pTex = GLContextManager::get()->createTexture(m_pBmp->getSize(), pf, 
                    m_Material.getUseMipmaps(), m_Material.getWrapSMode(), 
                    m_Material.getWrapTMode());
            m_pSurface->create(pf, pTex);
        }
        GLContextManager::get()->scheduleTexUpload(pTex, m_pBmp);
*/
    }
    assertValid();
}

void GPUImage::setCanvas(OffscreenCanvasPtr pCanvas)
{
    assertValid();
    if (m_Source == SCENE && pCanvas == m_pCanvas) {
        return;
    }
    if (m_pImage) {
        if (m_State == GPU) {
            m_pImage->decTexRef();
            m_pSurface->destroy();
        }
        m_pImage->decBmpRef();
    }
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
        return BitmapPtr(new Bitmap(*m_pImage->getBmp()));
    }
}

IntPoint GPUImage::getSize()
{
    if (m_Source == NONE) {
        return IntPoint(0,0);
    } else {
        switch (m_State) {
            case CPU:
                if (m_Source == SCENE) {
                    return m_pCanvas->getSize();
                } else {
                    return m_pImage->getBmp()->getSize();
                }
            case GPU:
                return m_pSurface->getSize();
            default:
                AVG_ASSERT(false);
                return IntPoint(0,0);
        }
    }
}

PixelFormat GPUImage::getPixelFormat()
{
    PixelFormat pf;
    if (BitmapLoader::get()->isBlueFirst()) {
        pf = B8G8R8X8;
    } else {
        pf = R8G8B8X8;
    }
    if (m_Source != NONE) {
        switch (m_State) {
            case CPU:
                if (m_Source != SCENE) {
                    pf = m_pImage->getBmp()->getPixelFormat();
                }
            case GPU:
                pf = m_pSurface->getPixelFormat();
            default:
                AVG_ASSERT(false);
        }
    }
    return pf;
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

GPUImage::TextureCompression GPUImage::string2compression(const string& s)
{
    if (s == "none") {
        return GPUImage::TEXTURECOMPRESSION_NONE;
    } else if (s == "B5G6R5") {
        return GPUImage::TEXTURECOMPRESSION_B5G6R5;
    } else {
        throw(Exception(AVG_ERR_UNSUPPORTED, 
                "GPUImage compression "+s+" not supported."));
    }
}

string GPUImage::compression2String(TextureCompression compression)
{
    switch(compression) {
        case GPUImage::TEXTURECOMPRESSION_NONE:
            return "none";
        case GPUImage::TEXTURECOMPRESSION_B5G6R5:
            return "B5G6R5";
        default:
            AVG_ASSERT(false);
            return 0;
    }
}

void GPUImage::setupSurface()
{
    PixelFormat pf = m_pImage->getBmp()->getPixelFormat();
    m_pImage->incTexRef();
    MCTexturePtr pTex = m_pImage->getTex();
    m_pSurface->create(pf, pTex);
}

bool GPUImage::changeSource(Source newSource)
{
    if (newSource != m_Source) {
        switch (m_Source) {
            case NONE:
                break;
            case FILE:
            case BITMAP:
//                m_pBmp = BitmapPtr();
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

void GPUImage::assertValid() const
{
    AVG_ASSERT(m_pSurface);
    AVG_ASSERT((m_Source == FILE) == (m_sFilename != ""));
    AVG_ASSERT((m_Source == SCENE) == bool(m_pCanvas));
//    AVG_ASSERT((m_Source == FILE || m_Source == BITMAP) == bool(m_pBmp));
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
