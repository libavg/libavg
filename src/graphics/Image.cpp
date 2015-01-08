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

#include "../base/Exception.h"
#include "../base/ObjectCounter.h"
#include "../base/Logger.h"

#include "BitmapLoader.h"
#include "Bitmap.h"
#include "GLContextManager.h"
#include "MCTexture.h"
#include "ImageRegistry.h"
#include "Filterfliprgb.h"

using namespace std;

namespace avg {

Image::Image(const std::string& sFilename, TextureCompression compression)
    : m_bUseMipmaps(false),
      m_Compression(compression),
      m_BmpRefCount(0),
      m_TexRefCount(0)
{
    ObjectCounter::get()->incRef(&typeid(*this));
    m_sFilename = sFilename;
    AVG_TRACE(Logger::category::MEMORY, Logger::severity::INFO, "Loading " << sFilename);
    BitmapPtr pBmp = loadBitmap(m_sFilename);
    m_pBmp = applyCompression(pBmp);
    incBmpRef(m_Compression);
}

Image::Image(const BitmapPtr& pBmp, TextureCompression compression)
    : m_bUseMipmaps(false),
      m_Compression(compression),
      m_BmpRefCount(0),
      m_TexRefCount(0)
{
    ObjectCounter::get()->incRef(&typeid(*this));
    m_sFilename = "";
    m_pBmp = BitmapPtr(new Bitmap(pBmp->getSize(), pBmp->getPixelFormat(), ""));
    m_pBmp->copyPixels(*pBmp);
    m_pBmp = applyCompression(m_pBmp);
    incBmpRef(m_Compression);
}

Image::~Image()
{
    ObjectCounter::get()->decRef(&typeid(*this));
    AVG_ASSERT(m_BmpRefCount == 0);
    AVG_ASSERT(m_TexRefCount == 0);
}

std::string Image::getFilename() const
{
    return m_sFilename;
}

void Image::incBmpRef(TextureCompression compression)
{
    m_BmpRefCount++;
    if (compression == TEXTURECOMPRESSION_NONE &&
            m_Compression == TEXTURECOMPRESSION_B5G6R5)
    {
        m_Compression = compression;
        BitmapPtr pBmp = loadBitmap(m_sFilename);
        m_pBmp = applyCompression(pBmp);
    }
}

void Image::decBmpRef()
{
    AVG_ASSERT(m_BmpRefCount >= 1);
    m_BmpRefCount--;

    testDelete();
}

void Image::incTexRef(bool bUseMipmaps)
{
    m_TexRefCount++; 
    if (m_TexRefCount == 1) {
        m_bUseMipmaps = bUseMipmaps;
        createTexture();
    } else if (bUseMipmaps && !m_bUseMipmaps) {
        m_bUseMipmaps = true;
        createTexture();
    }
}

void Image::decTexRef()
{
    AVG_ASSERT(m_TexRefCount >= 1);
    m_TexRefCount--; 
    if (m_TexRefCount == 0) {
        m_pTex = MCTexturePtr();
    }

    testDelete();
}

BitmapPtr Image::getBmp()
{
    AVG_ASSERT(m_BmpRefCount >= 1);
    return m_pBmp;
}

MCTexturePtr Image::getTex()
{
    AVG_ASSERT(m_TexRefCount >= 1);
    return m_pTex;
}

Image::TextureCompression Image::string2compression(const string& s)
{
    if (s == "none") {
        return Image::TEXTURECOMPRESSION_NONE;
    } else if (s == "B5G6R5") {
        return Image::TEXTURECOMPRESSION_B5G6R5;
    } else {
        throw(Exception(AVG_ERR_UNSUPPORTED, 
                "GPUImage compression "+s+" not supported."));
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

BitmapPtr Image::applyCompression(BitmapPtr pBmp)
{
    if (m_Compression == TEXTURECOMPRESSION_B5G6R5) {
        BitmapPtr pDestBmp = BitmapPtr(new Bitmap(pBmp->getSize(), B5G6R5, m_sFilename));
        if (!BitmapLoader::get()->isBlueFirst()) {
            FilterFlipRGB().applyInPlace(pBmp);
        }
        pDestBmp->copyPixels(*pBmp);
        return pDestBmp;
    } else {
        return pBmp;
    }
}

void Image::createTexture()
{
    m_pTex = GLContextManager::get()->createTexture(m_pBmp->getSize(),
            m_pBmp->getPixelFormat(), m_bUseMipmaps,
            GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
    GLContextManager::get()->scheduleTexUpload(m_pTex, m_pBmp);
}

void Image::testDelete()
{
    if (m_BmpRefCount == 0 && m_TexRefCount == 0 && m_sFilename != "") {
        ImageRegistry::get()->deleteImage(m_sFilename);
    }
}

}
