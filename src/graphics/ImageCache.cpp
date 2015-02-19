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

#include "ImageCache.h"

#include "../base/Exception.h"

using namespace std;

namespace avg {

ImageCache * ImageCache::s_pImageCache = 0;
    
ImageCache* ImageCache::get() 
{
    
    if (s_pImageCache == 0) {
        s_pImageCache = new ImageCache();
    }
    return s_pImageCache;
}

ImageCache::ImageCache()
    : m_CPUCacheSize(0),
      m_GPUCacheSize(0),
      m_CPUCacheUsed(0),
      m_GPUCacheUsed(0)

{
}

ImageCache::~ImageCache()
{
}

void ImageCache::setSize(long long cpuSize, long long gpuSize)
{
    m_CPUCacheSize = cpuSize;
    m_GPUCacheSize = gpuSize;
}

ImagePtr ImageCache::getImage(const std::string& sFilename,
        Image::TextureCompression compression)
{
    ImageMap::iterator it = m_pImageMap.find(sFilename);
    ImagePtr pImg;
    if (it == m_pImageMap.end()) {
        pImg = ImagePtr(new Image(sFilename, compression));
        m_pLRUList.push_front(pImg);
        m_pImageMap.insert(make_pair(sFilename, m_pLRUList.begin()));
        m_CPUCacheUsed += pImg->getBmpMemUsed();
        checkCPUUnload();
    } else {
        pImg = *(it->second);
        pImg->incBmpRef(compression);
        // Move item to front of list
        if (it != m_pImageMap.end()) {
            m_pLRUList.splice(m_pLRUList.begin(), m_pLRUList, it->second);
        }
    }
    assertValid();
    return pImg;
}

void ImageCache::onTexLoad(const std::string& sFilename)
{
    ImagePtr pImg = *(m_pImageMap[sFilename]);
    m_GPUCacheUsed += pImg->getTexMemUsed();
    ImageMap::iterator it = m_pImageMap.find(sFilename);
    // Move item to front of list
    if (it != m_pImageMap.end()) {
        m_pLRUList.splice(m_pLRUList.begin(), m_pLRUList, it->second);
    }
    checkGPUUnload();
}

void ImageCache::onAccess(const std::string& sFilename)
{
    checkCPUUnload();
    assertValid();
    ImageMap::iterator it = m_pImageMap.find(sFilename);
    // Move item to front of list
    if (it != m_pImageMap.end()) {
        m_pLRUList.splice(m_pLRUList.begin(), m_pLRUList, it->second);
    }
}

void ImageCache::onSizeChange(const std::string& sFilename, int sizeDiff)
{
    ImageMap::iterator it = m_pImageMap.find(sFilename);
    ImagePtr pImg = *(it->second);
    m_CPUCacheUsed += sizeDiff;
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

void ImageCache::dump() const
{
    for (LRUListType::const_iterator it=m_pLRUList.begin(); it!=m_pLRUList.end(); ++it) {
        (*it)->dump();
    }
}

void ImageCache::checkCPUUnload()
{
    while (m_CPUCacheUsed > m_CPUCacheSize) {
        ImagePtr pImg = *(m_pLRUList.rbegin());
        if (pImg->getBmpRefCount() == 0) {
            m_pImageMap.erase(pImg->getFilename());
            m_pLRUList.pop_back();
            m_CPUCacheUsed -= pImg->getBmpMemUsed();
            if (pImg->hasTex()) {
                m_GPUCacheUsed -= pImg->getTexMemUsed();
            }
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
    if (m_GPUCacheUsed > m_GPUCacheSize) {
        LRUListType::reverse_iterator it = m_pLRUList.rbegin();
        // Find first item that actually has a texture loaded.
        while (it != m_pLRUList.rend() && !((*it)->hasTex())) {
            it++;
        }
        if (it != m_pLRUList.rend()) {
            while (m_GPUCacheUsed > m_GPUCacheSize) {
                assertValid();
                ImagePtr pImg = *it;
                if (pImg->getTexRefCount() == 0) {
                    m_GPUCacheUsed -= pImg->getTexMemUsed();
                    pImg->unloadTex();
                    ++it;
                } else {
                    break;
                }
            }
        }
    }
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
