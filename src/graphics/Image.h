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

#ifndef _Image_H_
#define _Image_H_

#include "../api.h"

#include <boost/shared_ptr.hpp>
#include <string>

namespace avg {

class Bitmap;
typedef boost::shared_ptr<Bitmap> BitmapPtr;
class MCTexture;
typedef boost::shared_ptr<MCTexture> MCTexturePtr;

class AVG_API Image
{
    public:
        enum TextureCompression {
            TEXTURECOMPRESSION_NONE,
            TEXTURECOMPRESSION_B5G6R5
        };
        enum StorageType {
            STORAGE_CPU,
            STORAGE_GPU
        };

        Image(const std::string& sFilename, TextureCompression compression);
        Image(const BitmapPtr& pBmp, TextureCompression compression);
        virtual ~Image();

        std::string getFilename() const;

        void incBmpRef(TextureCompression compression);
        void decBmpRef();
        void incTexRef(bool bUseMipmaps);
        void decTexRef();
        void unloadTex();

        BitmapPtr getBmp();
        MCTexturePtr getTex();
        bool hasTex() const;
        int getMemUsed(StorageType st) const;
        int getRefCount(StorageType st) const;

        static TextureCompression string2compression(const std::string& s);
        static std::string compression2String(TextureCompression compression);

        void dump() const;

    private:
        BitmapPtr applyCompression(BitmapPtr pBmp);
        void createTexture();
        void testDelete();

        std::string m_sFilename;
        BitmapPtr m_pBmp;
        MCTexturePtr m_pTex;

        bool m_bUseMipmaps;
        TextureCompression m_Compression;
        
        int m_BmpRefCount;
        int m_TexRefCount;
};

typedef boost::shared_ptr<Image> ImagePtr;

}

#endif

