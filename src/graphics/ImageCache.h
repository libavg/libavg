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

#include "Image.h"

#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>
#include <map>

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
        static ImageCache* get();

        void setSize(long long cpuSize, long long gpuSize);
        ImagePtr getImage(const std::string& sFilename,
                Image::TextureCompression compression);
        void onTexLoad(const std::string& sFilename);
        void onAccess(const std::string& sFilename);
        void onSizeChange(const std::string& sFilename, int sizeDiff);
        int getNumCPUImages() const;
        int getNumGPUImages() const;

    private:
        ImageCache();
        virtual ~ImageCache();
        void checkCPUUnload();
        void checkGPUUnload();

        typedef std::list<ImagePtr> LRUListType;
        LRUListType m_pLRUList;
        typedef boost::unordered_map<std::string, LRUListType::iterator> ImageMap;
        ImageMap m_pImageMap;

        long long m_CPUCacheSize;
        long long m_GPUCacheSize;
        long long m_CPUCacheUsed;
        long long m_GPUCacheUsed;

        static ImageCache * s_pImageCache;
};

}

#endif

