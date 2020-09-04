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

#include "CachedImage.h"

#include "../base/Exception.h"
#include "../base/ObjectCounter.h"
#include "../base/Logger.h"
#include "../base/TimeSource.h"

#include "BitmapLoader.h"
#include "Bitmap.h"
#include "GLContextManager.h"
#include "MCTexture.h"
#include "ImageCache.h"
#include "Filterfliprgb.h"

using namespace std;

namespace avg {

CachedImage::CachedImage(const std::string& sFilename, TexCompression compression)
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

CachedImage::~CachedImage()
{
    ObjectCounter::get()->decRef(&typeid(*this));
    AVG_ASSERT(m_BmpRefCount == 0);
    AVG_ASSERT(m_TexRefCount == 0);
}

std::string CachedImage::getFilename() const
{
    return m_sFilename;
}

void CachedImage::incBmpRef(TexCompression compression)
{
    m_BmpRefCount++;
    if (compression == TEXCOMPRESSION_NONE && m_Compression == TEXCOMPRESSION_B5G6R5)
    {
        // Reload from disk, making sure the cache knows about the size change
        int oldSize = m_pBmp->getMemNeeded();
        m_Compression = compression;
        BitmapPtr pBmp = loadBitmap(m_sFilename);
        m_pBmp = applyCompression(pBmp);
        ImageCache::get()->onSizeChange(pBmp->getMemNeeded()-oldSize, STORAGE_CPU);
    }
}

void CachedImage::decBmpRef()
{
    AVG_ASSERT(m_BmpRefCount >= 1);
    m_BmpRefCount--;
    AVG_ASSERT(m_TexRefCount <= m_BmpRefCount);
    if (m_BmpRefCount == 0 && m_TexRefCount == 0) {
        ImageCache::get()->onImageUnused(m_sFilename, STORAGE_CPU);
    }
}

void CachedImage::incTexRef(bool bUseMipmaps)
{
    m_TexRefCount++;
    AVG_ASSERT(m_TexRefCount <= m_BmpRefCount);
    if (m_TexRefCount == 1) {
        m_bUseMipmaps = bUseMipmaps;
        if (!m_pTex) {
            createTexture();
            ImageCache::get()->onTexLoad(m_sFilename);
        }
    } else if (bUseMipmaps && !m_bUseMipmaps) {
        m_bUseMipmaps = true;
        int oldSize = m_pTex->getMemNeeded();
        createTexture();
        ImageCache::get()->onSizeChange(m_pTex->getMemNeeded()-oldSize, STORAGE_GPU);
    }
}

void CachedImage::decTexRef()
{
    AVG_ASSERT(m_TexRefCount >= 1);
    m_TexRefCount--;
    if (m_TexRefCount == 0) {
        ImageCache::get()->onImageUnused(m_sFilename, STORAGE_GPU);
    }
}

void CachedImage::unloadTex()
{
    AVG_ASSERT(m_TexRefCount == 0);
    AVG_ASSERT(m_pTex);
    m_pTex = MCTexturePtr();
}

BitmapPtr CachedImage::getBmp()
{
    AVG_ASSERT(m_BmpRefCount >= 1);
    return m_pBmp;
}

MCTexturePtr CachedImage::getTex()
{
    AVG_ASSERT(m_TexRefCount >= 1);
    return m_pTex;
}

bool CachedImage::hasTex() const
{
    return m_pTex != MCTexturePtr();
}

int CachedImage::getMemUsed(StorageType st) const
{
    switch(st) {
        case CachedImage::STORAGE_CPU:
            return m_pBmp->getMemNeeded();
        case CachedImage::STORAGE_GPU:
            if (m_pTex) {
                return m_pTex->getMemNeeded();
            } else {
                return 0;
            }
        default:
            AVG_ASSERT(false);
            return 0;
    }

}

int CachedImage::getRefCount(StorageType st) const
{
    switch(st) {
        case CachedImage::STORAGE_CPU:
            return m_BmpRefCount;
        case CachedImage::STORAGE_GPU:
            return m_TexRefCount;
        default:
            AVG_ASSERT(false);
            return 0;
    }
}

void CachedImage::dump() const
{
    cerr << "  " << m_sFilename << ": " << m_BmpRefCount << ", " << m_TexRefCount
            << ", " << hasTex() << endl;
}

BitmapPtr CachedImage::applyCompression(BitmapPtr pBmp)
{
    // Duplicated code with GPUImage::setBitmap()
    if (m_Compression == TEXCOMPRESSION_B5G6R5) {
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

void CachedImage::createTexture()
{
    m_pTex = GLContextManager::get()->createTextureFromBmp(m_pBmp, m_bUseMipmaps);
}

}
