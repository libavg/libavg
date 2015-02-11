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
    
}

ImagePtr ImageCache::getImage(const std::string& sFilename,
        Image::TextureCompression compression)
{
    cerr << "getImage: " << sFilename << endl;
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
    }
    return pImg;
}

void ImageCache::onTexLoad(const std::string& sFilename)
{
    cerr << "onTexLoad: " << sFilename << endl;
    ImagePtr pImg = *(m_pImageMap[sFilename]);
    m_GPUCacheSize += pImg->getTexMemUsed();
    checkGPUUnload();
}

void ImageCache::onAccess(const std::string& sFilename)
{
    cerr << "onAccess: " << sFilename << endl;
    auto it = m_pImageMap.find(sFilename);
    // Move item to front of list
    m_pLRUList.splice(m_pLRUList.begin(), m_pLRUList, it->second);
    ImagePtr pImg = *(it->second);
    checkCPUUnload();
}

void ImageCache::onSizeChange(const std::string& sFilename, int sizeDiff)
{
    auto it = m_pImageMap.find(sFilename);
    ImagePtr pImg = *(it->second);
    m_CPUCacheUsed += sizeDiff;
}

int ImageCache::getNumCPUImages() const
{
    return m_pImageMap.size();
}

int ImageCache::getNumGPUImages() const
{
    // TODO: Count images with Tex use count > 0
}

void ImageCache::checkCPUUnload()
{
    cerr << "checkCPUUnload" << endl;
    while (m_CPUCacheUsed > m_CPUCacheSize) {
        ImagePtr pImg = *(m_pLRUList.rbegin());
        cerr << "  unload: " << pImg->getFilename() << endl;
        if (pImg->getBmpRefCount() == 0) {
            m_pImageMap.erase(pImg->getFilename());
            m_pLRUList.pop_back();
            m_CPUCacheUsed -= pImg->getBmpMemUsed();
            m_GPUCacheUsed -= pImg->getTexMemUsed();
            cerr << "  -> RefCount == 0, used: " << m_CPUCacheUsed << endl;
        } else {
            // Cache full, but everything's in use.
            cerr << "  -> RefCount != 0" << endl;
            break;
        }
    }
}

void ImageCache::checkGPUUnload()
{
}

}
