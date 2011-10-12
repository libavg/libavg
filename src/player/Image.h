//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2011 Ulrich von Zadow
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

#include "MaterialInfo.h"

#include "../base/Point.h"
#include "../graphics/Bitmap.h"

#include <boost/shared_ptr.hpp>
#include <string>

namespace avg {

class OGLSurface;
class OffscreenCanvas;
typedef boost::shared_ptr<OffscreenCanvas> OffscreenCanvasPtr;

class AVG_API Image
{
    public:
        enum State {CPU, GPU};
        enum Source {NONE, FILE, BITMAP, SCENE};
        enum TextureCompression {
            TEXTURECOMPRESSION_NONE,
            TEXTURECOMPRESSION_B5G6R5
        };

        Image(OGLSurface * pSurface, const MaterialInfo& material);
        virtual ~Image();

        virtual void moveToGPU();
        virtual void moveToCPU();

        void discard();
        void setEmpty();
        void setFilename(const std::string& sFilename,
                TextureCompression comp = TEXTURECOMPRESSION_NONE);
        void setBitmap(BitmapPtr pBmp, 
                TextureCompression comp = TEXTURECOMPRESSION_NONE);
        void setCanvas(OffscreenCanvasPtr pCanvas);
        OffscreenCanvasPtr getCanvas() const;
        const std::string& getFilename() const;

        BitmapPtr getBitmap();
        IntPoint getSize();
        PixelFormat getPixelFormat();
        OGLSurface* getSurface();
        State getState();
        Source getSource();

        static TextureCompression string2compression(const std::string& s);
        static std::string compression2String(TextureCompression compression);

    private:
        void setupSurface();
        PixelFormat calcSurfacePF(const Bitmap& Bmp);
        bool changeSource(Source newSource);
        void assertValid() const;

        std::string m_sFilename;
        BitmapPtr m_pBmp;
        OGLSurface * m_pSurface;
        OffscreenCanvasPtr m_pCanvas;

        State m_State;
        Source m_Source;
        MaterialInfo m_Material;
};

typedef boost::shared_ptr<Image> ImagePtr;

}

#endif

