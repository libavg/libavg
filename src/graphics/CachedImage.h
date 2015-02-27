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

#ifndef _CachedImage_H_
#define _CachedImage_H_

#include "../api.h"

#include "TexInfo.h"

#include <boost/shared_ptr.hpp>
#include <string>

namespace avg {

class Bitmap;
typedef boost::shared_ptr<Bitmap> BitmapPtr;
class MCTexture;
typedef boost::shared_ptr<MCTexture> MCTexturePtr;

class AVG_API CachedImage
{
    public:
        enum StorageType {
            STORAGE_CPU,
            STORAGE_GPU
        };

        CachedImage(const std::string& sFilename, TexCompression compression);
        virtual ~CachedImage();

        std::string getFilename() const;

        void incBmpRef(TexCompression compression);
        void decBmpRef();
        void incTexRef(bool bUseMipmaps);
        void decTexRef();
        void unloadTex();

        BitmapPtr getBmp();
        MCTexturePtr getTex();
        bool hasTex() const;
        int getMemUsed(StorageType st) const;
        int getRefCount(StorageType st) const;

        void dump() const;

    private:
        BitmapPtr applyCompression(BitmapPtr pBmp);
        void createTexture();
        void testDelete();

        std::string m_sFilename;
        BitmapPtr m_pBmp;
        MCTexturePtr m_pTex;

        bool m_bUseMipmaps;
        TexCompression m_Compression;
        
        int m_BmpRefCount;
        int m_TexRefCount;
};

typedef boost::shared_ptr<CachedImage> CachedImagePtr;

}

#endif

