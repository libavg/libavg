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

#include "Image.h"

#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/ObjectCounter.h"

#include "../graphics/Filterfliprgb.h"
#include "../graphics/BitmapLoader.h"
#include "../graphics/GLContextManager.h"

#include "OGLSurface.h"
#include "OffscreenCanvas.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

Image::Image(OGLSurface * pSurface, const MaterialInfo& material)
    : m_sFilename(""),
      m_pSurface(pSurface),
      m_State(CPU),
      m_Source(NONE),
      m_Material(material)
{
    ObjectCounter::get()->incRef(&typeid(*this));
    assertValid();
}

Image::~Image()
{
    if (m_State == GPU && m_Source != NONE) {
        m_pSurface->destroy();
    }
    ObjectCounter::get()->decRef(&typeid(*this));
}
        
void Image::moveToGPU()
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

void Image::moveToCPU()
{
    assertValid();
    if (m_State == GPU) {
        m_State = CPU;
        m_pSurface->destroy();
    }
    assertValid();
}

void Image::discard()
{
    assertValid();
    setEmpty();
    m_State = CPU;
    assertValid();
}

void Image::setEmpty()
{
    assertValid();
    if (m_State == GPU) {
        m_pSurface->destroy();
    }
    changeSource(NONE);
    assertValid();
}

void Image::setFilename(const std::string& sFilename, TextureCompression comp)
{
    assertValid();
    AVG_TRACE(Logger::category::MEMORY, Logger::severity::INFO, "Loading " << sFilename);
    BitmapPtr pBmp = loadBitmap(sFilename);
    if (comp == TEXTURECOMPRESSION_B5G6R5 && pBmp->hasAlpha()) {
        throw Exception(AVG_ERR_UNSUPPORTED, 
                "B5G6R5-compressed textures with an alpha channel are not supported.");
    }
    changeSource(FILE);
    m_pBmp = pBmp;

    m_sFilename = sFilename;

    switch (comp) {
        case TEXTURECOMPRESSION_B5G6R5:
            m_pBmp = BitmapPtr(new Bitmap(pBmp->getSize(), B5G6R5, sFilename));
            if (!BitmapLoader::get()->isBlueFirst()) {
                FilterFlipRGB().applyInPlace(pBmp);
            }
            m_pBmp->copyPixels(*pBmp);
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

void Image::setBitmap(BitmapPtr pBmp, TextureCompression comp)
{
    assertValid();
    if (!pBmp) {
        throw Exception(AVG_ERR_UNSUPPORTED, "setBitmap(): bitmap must not be None!");
    }
    if (comp == TEXTURECOMPRESSION_B5G6R5 && pBmp->hasAlpha()) {
        throw Exception(AVG_ERR_UNSUPPORTED, 
                "B5G6R5-compressed textures with an alpha channel are not supported.");
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
    m_pBmp = BitmapPtr(new Bitmap(pBmp->getSize(), pf, ""));
    m_pBmp->copyPixels(*pBmp);
    if (m_State == GPU) {
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
    }
    assertValid();
}

void Image::setCanvas(OffscreenCanvasPtr pCanvas)
{
    assertValid();
    if (m_Source == SCENE && pCanvas == m_pCanvas) {
        return;
    }
    changeSource(SCENE);
    m_pCanvas = pCanvas;
    if (m_State == GPU) {
        m_pSurface->create(B8G8R8X8, m_pCanvas->getTex(), MCTexturePtr(), MCTexturePtr(),
                MCTexturePtr(), true);
    }
    assertValid();
}

OffscreenCanvasPtr Image::getCanvas() const
{
    return m_pCanvas;
}

const string& Image::getFilename() const
{
    return m_sFilename;
}

BitmapPtr Image::getBitmap()
{
    if (m_Source == NONE || m_Source == SCENE) {
        return BitmapPtr();
    } else {
        return BitmapPtr(new Bitmap(*m_pBmp));
    }
}

IntPoint Image::getSize()
{
    if (m_Source == NONE) {
        return IntPoint(0,0);
    } else {
        switch (m_State) {
            case CPU:
                if (m_Source == SCENE) {
                    return m_pCanvas->getSize();
                } else {
                    return m_pBmp->getSize();
                }
            case GPU:
                return m_pSurface->getSize();
            default:
                AVG_ASSERT(false);
                return IntPoint(0,0);
        }
    }
}

PixelFormat Image::getPixelFormat()
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
                    pf = m_pBmp->getPixelFormat();
                }
            case GPU:
                pf = m_pSurface->getPixelFormat();
            default:
                AVG_ASSERT(false);
        }
    }
    return pf;
}

OGLSurface* Image::getSurface()
{
    AVG_ASSERT(m_State == GPU);
    return m_pSurface;
}

Image::State Image::getState()
{
    return m_State;
}

Image::Source Image::getSource()
{
    return m_Source;
}

Image::TextureCompression Image::string2compression(const string& s)
{
    if (s == "none") {
        return Image::TEXTURECOMPRESSION_NONE;
    } else if (s == "B5G6R5") {
        return Image::TEXTURECOMPRESSION_B5G6R5;
    } else {
        throw(Exception(AVG_ERR_UNSUPPORTED, 
                "Image compression "+s+" not supported."));
    }
}

string Image::compression2String(TextureCompression compression)
{
    switch(compression) {
        case Image::TEXTURECOMPRESSION_NONE:
            return "none";
        case Image::TEXTURECOMPRESSION_B5G6R5:
            return "B5G6R5";
        default:
            AVG_ASSERT(false);
            return 0;
    }
}

void Image::setupSurface()
{
    PixelFormat pf = m_pBmp->getPixelFormat();
//    cerr << "setupSurface: " << pf << endl;
    MCTexturePtr pTex = GLContextManager::get()->createTexture(m_pBmp->getSize(), pf, 
            m_Material.getUseMipmaps(), 
            m_Material.getWrapSMode(), m_Material.getWrapTMode());
    m_pSurface->create(pf, pTex);
    GLContextManager::get()->scheduleTexUpload(pTex, m_pBmp);
}

bool Image::changeSource(Source newSource)
{
    if (newSource != m_Source) {
        switch (m_Source) {
            case NONE:
                break;
            case FILE:
            case BITMAP:
                m_pBmp = BitmapPtr();
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

void Image::assertValid() const
{
    AVG_ASSERT(m_pSurface);
    AVG_ASSERT((m_Source == FILE) == (m_sFilename != ""));
    AVG_ASSERT((m_Source == SCENE) == bool(m_pCanvas));
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
