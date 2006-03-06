//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
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

#ifndef _OGLTile_H_
#define _OGLTile_H_

#include "OGLHelper.h"
#include "SDLDisplayEngine.h"
#include "../graphics/Rect.h"
#include "../graphics/Bitmap.h"
#include "../base/CountedPointer.h"

#include "GL/gl.h"
#ifndef __APPLE__
#define GLX_GLXEXT_PROTOTYPES
#include "GL/glx.h"
#endif

namespace avg {

class OGLTile {
    public:
        OGLTile(IntRect Extent, IntPoint TexSize, int Stride, PixelFormat pf, 
                SDLDisplayEngine * pEngine);
        virtual ~OGLTile();

        const IntRect& getExtent() const;
        const IntPoint& getTexSize() const;
        int getTexID(int i) const;
        void downloadTexture(int i, BitmapPtr pBmp, int width, 
                OGLMemoryMode MemoryMode) const;
//        void downloadTextures(BitmapPtr pBmp, int width, 
//                OGLMemoryMode MemoryMode) const;
        void blt(const DPoint& TLPoint, const DPoint& TRPoint,
                const DPoint& BLPoint, const DPoint& BRPoint) const;

    private:
        void createTexture(int i, IntPoint Size, int Stride, PixelFormat pf);

        IntRect m_Extent;
        IntPoint m_TexSize;
        PixelFormat m_pf;
        SDLDisplayEngine * m_pEngine;
        int m_TextureMode;
        unsigned int m_TexID[3];
};

typedef CountedPointer<OGLTile> OGLTilePtr;

}

#endif

