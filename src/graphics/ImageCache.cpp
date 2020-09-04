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

#include "ImageCache.h"

#include "../base/Exception.h"
#include "../base/OSHelper.h"
#include "../base/ConfigMgr.h"
#include "../base/Logger.h"

using namespace std;

namespace avg {

ImageCache * ImageCache::s_pImageCache = 0;

bool ImageCache::exists()
{
    return (s_pImageCache != 0);
}

ImageCache* ImageCache::get() 
{
    if (s_pImageCache == 0) {
        s_pImageCache = new ImageCache();
    }
    return s_pImageCache;
}

ImageCache::ImageCache()
    : m_CPUCacheUsed(0),
      m_GPUCacheUsed(0)
{
    glm::vec2 sizeOpt = ConfigMgr::get()->getSizeOption("scr", "imgcachesize");
    if (sizeOpt[0] == -1) {
        m_CPUCacheCapacity = (long long)(getPhysMemorySize())/4;
    } else {
        m_CPUCacheCapacity = (long long)(sizeOpt[0])*1024*1024;
    }
    if (sizeOpt[1] == -1) {
        m_GPUCacheCapacity = 16*1024*1024;
    } else {
        m_GPUCacheCapacity = (long long)(sizeOpt[1])*1024*1024;
    }
    AVG_TRACE(Logger::category::CONFIG, Logger::severity::INFO,
            "Image cache size: CPU=" << m_CPUCacheCapacity/(1024*1024) <<
            "MB, GPU=" << m_GPUCacheCapacity/(1024*1024) << "MB" << endl);
}

ImageCache::~ImageCache()
{
}

void ImageCache::setCapacity(long long cpuCapacity, long long gpuCapacity)
{
    m_CPUCacheCapacity = cpuCapacity;
    m_GPUCacheCapacity = gpuCapacity;
    checkCPUUnload();
}

long long ImageCache::getCapacity(CachedImage::StorageType st)
{
    if (st == CachedImage::STORAGE_CPU) {
        return m_CPUCacheCapacity;
    } else {
        return m_GPUCacheCapacity;
    }
}

long long ImageCache::getMemUsed(CachedImage::StorageType st)
{
    if (st == CachedImage::STORAGE_CPU) {
        return m_CPUCacheUsed;
    } else {
        return m_GPUCacheUsed;
    }
}

CachedImagePtr ImageCache::getImage(const std::string& sFilename,
        TexCompression compression)
{
    ImageMap::iterator it = m_pImageMap.find(sFilename);
    CachedImagePtr pImg;
    if (it == m_pImageMap.end()) {
        pImg = CachedImagePtr(new CachedImage(sFilename, compression));
        m_pLRUList.push_front(pImg);
        m_pImageMap.insert(make_pair(sFilename, m_pLRUList.begin()));
        m_CPUCacheUsed += pImg->getMemUsed(CachedImage::STORAGE_CPU);
        checkCPUUnload();
    } else {
        pImg = *(it->second);
        pImg->incBmpRef(compression);
        // Move item to front of list
        m_pLRUList.splice(m_pLRUList.begin(), m_pLRUList, it->second);
    }
    assertValid();
    return pImg;
}

void ImageCache::onTexLoad(const std::string& sFilename)
{
    CachedImagePtr pImg = *(m_pImageMap[sFilename]);
    m_GPUCacheUsed += pImg->getMemUsed(CachedImage::STORAGE_GPU);
    ImageMap::iterator it = m_pImageMap.find(sFilename);
    // Move item to front of list
    if (it != m_pImageMap.end()) {
        m_pLRUList.splice(m_pLRUList.begin(), m_pLRUList, it->second);
    }
    checkGPUUnload();
}

void ImageCache::onImageUnused(const std::string& sFilename, CachedImage::StorageType st)
{
    // Move image to first pos with use count == 0
    // This is currently O(n). If that becomes an issue, we need to remember the first
    // unused image for both CPU and GPU.
    LRUListType::iterator itOldPos = m_pImageMap.find(sFilename)->second;
    LRUListType::iterator itNewPos = itOldPos;
    itNewPos++;
    while (itNewPos != m_pLRUList.end() &&
            (*itNewPos)->getRefCount(st) != 0)
    {
        itNewPos++;
    }
    m_pLRUList.splice(itNewPos, m_pLRUList, itOldPos);
    checkCPUUnload();
}

void ImageCache::onSizeChange(int sizeDiff, CachedImage::StorageType st)
{
    if (st == CachedImage::STORAGE_CPU) {
        m_CPUCacheUsed += sizeDiff;
    } else {
        m_GPUCacheUsed += sizeDiff;
    }
    assertValid();
}

int ImageCache::getNumCPUImages() const
{
    return m_pImageMap.size();
}

int ImageCache::getNumGPUImages() const
{
    int numGPUImages = 0;
    for (LRUListType::const_iterator it=m_pLRUList.begin(); it!=m_pLRUList.end(); ++it) {
        if ((*it)->hasTex()) {
            numGPUImages++;
        }
    }
    return numGPUImages;
}

void ImageCache::unloadAllTextures()
{
    for (LRUListType::const_iterator it=m_pLRUList.begin(); it!=m_pLRUList.end(); ++it) {
        CachedImagePtr pImg = *it;
        AVG_ASSERT(pImg->getRefCount(CachedImage::STORAGE_GPU) == 0);
        if (pImg->hasTex()) {
            m_GPUCacheUsed -= pImg->getMemUsed(CachedImage::STORAGE_GPU);
            pImg->unloadTex();
        }
    }
}

void ImageCache::dump() const
{
    cerr << "----------------" << endl;
    cerr << "ImageCache: " << m_pLRUList.size() << ", CPU used: " << m_CPUCacheUsed <<
            ", GPU used: " << m_GPUCacheUsed << endl;
    for (LRUListType::const_iterator it=m_pLRUList.begin(); it!=m_pLRUList.end(); ++it) {
        (*it)->dump();
    }
}

void ImageCache::checkCPUUnload()
{
    while (m_CPUCacheUsed > m_CPUCacheCapacity) {
        CachedImagePtr pImg = *(m_pLRUList.rbegin());
        if (pImg->getRefCount(CachedImage::STORAGE_CPU) == 0) {
            m_pImageMap.erase(pImg->getFilename());
            m_pLRUList.pop_back();
            m_CPUCacheUsed -= pImg->getMemUsed(CachedImage::STORAGE_CPU);
            m_GPUCacheUsed -= pImg->getMemUsed(CachedImage::STORAGE_GPU);
        } else {
            // Cache full, but everything's in use.
            break;
        }
    }
    assertValid();
    checkGPUUnload();
}

void ImageCache::checkGPUUnload()
{
    if (m_GPUCacheUsed > m_GPUCacheCapacity) {
        LRUListType::reverse_iterator it = m_pLRUList.rbegin();
        // Find first item that actually has a texture loaded.
        while (it != m_pLRUList.rend() && !((*it)->hasTex())) {
            it++;
        }
        if (it != m_pLRUList.rend()) {
            while (m_GPUCacheUsed > m_GPUCacheCapacity) {
                assertValid();
                CachedImagePtr pImg = *it;
                if (pImg->getRefCount(CachedImage::STORAGE_GPU) == 0) {
                    if (pImg->hasTex()) {
                        m_GPUCacheUsed -= pImg->getMemUsed(CachedImage::STORAGE_GPU);
                        pImg->unloadTex();
                    }
                    ++it;
                } else {
                    // Cache full, but everything's in use.
                    break;
                }
            }
        }
    }
    assertValid();
}

void ImageCache::assertValid()
{
    if (m_CPUCacheUsed == 0) {
        AVG_ASSERT(m_pLRUList.size() == 0);
        AVG_ASSERT(m_pImageMap.size() == 0);
    }
    if (m_pLRUList.size() == 0) {
        AVG_ASSERT(m_CPUCacheUsed == 0);
        AVG_ASSERT(m_GPUCacheUsed == 0);
    }
    if (getNumGPUImages() == 0) {
        AVG_ASSERT(m_GPUCacheUsed == 0);
    }
    AVG_ASSERT(m_pLRUList.size() == m_pImageMap.size());
}

}
