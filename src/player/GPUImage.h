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

#ifndef _GPUImage_H_
#define _GPUImage_H_

#include "../api.h"

#include "../base/GLMHelper.h"

#include "../graphics/PixelFormat.h"
#include "../graphics/TexInfo.h"

#include <boost/shared_ptr.hpp>
#include <string>

namespace avg {

class OGLSurface;
class OffscreenCanvas;
typedef boost::shared_ptr<OffscreenCanvas> OffscreenCanvasPtr;
class Bitmap;
typedef boost::shared_ptr<Bitmap> BitmapPtr;
class CachedImage;
typedef boost::shared_ptr<CachedImage> CachedImagePtr;

class AVG_API GPUImage
{
    public:
        enum State {CPU, GPU};
        enum Source {NONE, FILE, BITMAP, SCENE};

        GPUImage(OGLSurface * pSurface, bool bUseMipmaps);
        virtual ~GPUImage();

        virtual void moveToGPU();
        virtual void moveToCPU();

        void setEmpty();
        void setFilename(const std::string& sFilename,
                TexCompression comp = TEXCOMPRESSION_NONE);
        void setBitmap(BitmapPtr pBmp, 
                TexCompression comp = TEXCOMPRESSION_NONE);
        void setCanvas(OffscreenCanvasPtr pCanvas);
        OffscreenCanvasPtr getCanvas() const;
        const std::string& getFilename() const;

        BitmapPtr getBitmap();
        IntPoint getSize();
        PixelFormat getPixelFormat();
        OGLSurface* getSurface();
        State getState();
        Source getSource();

    private:
        void setupImageSurface();
        void setupBitmapSurface();
        bool changeSource(Source newSource);
        void unload();
        void assertValid() const;

        std::string m_sFilename;
        CachedImagePtr m_pImage;
        OGLSurface * m_pSurface;

        OffscreenCanvasPtr m_pCanvas;
        BitmapPtr m_pBmp;

        State m_State;
        Source m_Source;
        bool m_bUseMipmaps;
};

typedef boost::shared_ptr<GPUImage> GPUImagePtr;

}

#endif

