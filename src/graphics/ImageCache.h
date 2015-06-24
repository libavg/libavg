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

#ifndef _ImageCache_H_
#define _ImageCache_H_

#include "../api.h"

#include "../base/GLMHelper.h"

#include "CachedImage.h"
#include "TexInfo.h"

#include <boost/shared_ptr.hpp>
#include <string>
#include <list>

#ifdef _WIN32
#include <unordered_map>
#elif defined __APPLE__
#include <boost/unordered_map.hpp>
#else
#include <tr1/unordered_map>
#endif

namespace avg {

class AVG_API ImageCache
{
    public:
        static bool exists();
        static ImageCache* get();
        virtual ~ImageCache();

        void setCapacity(long long cpuCapacity, long long gpuCapacity);
        long long getCapacity(CachedImage::StorageType st);
        long long getMemUsed(CachedImage::StorageType st);
        CachedImagePtr getImage(const std::string& sFilename,
                TexCompression compression);
        void onTexLoad(const std::string& sFilename);
        void onImageUnused(const std::string& sFilename, CachedImage::StorageType st);
        void onSizeChange(int sizeDiff, CachedImage::StorageType st);
        int getNumCPUImages() const;
        int getNumGPUImages() const;

        void unloadAllTextures();
        void dump() const;

    private:
        ImageCache();
        void checkCPUUnload();
        void checkGPUUnload();

        void assertValid();

        typedef std::list<CachedImagePtr> LRUListType;
        // This is a list of all loaded images in three partitions:
        // 1) Images that have both CPU and GPU refcounts > 0
        // 2) Images that have only CPU refcount > 0
        // 3) Images that are unused but cached.
        // The third partition is sorted by LRU.
        LRUListType m_pLRUList;
#ifdef __APPLE__
        typedef boost::unordered_map<std::string, LRUListType::iterator> ImageMap;
#else
        typedef std::tr1::unordered_map<std::string, LRUListType::iterator> ImageMap;
#endif
        ImageMap m_pImageMap;

        long long m_CPUCacheCapacity;
        long long m_GPUCacheCapacity;
        long long m_CPUCacheUsed;
        long long m_GPUCacheUsed;

        static ImageCache * s_pImageCache;
};

}

#endif

