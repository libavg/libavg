//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2008 Ulrich von Zadow
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

#ifndef _OGLTexture_H_
#define _OGLTexture_H_

#include "../graphics/Bitmap.h"
#include "../graphics/OGLHelper.h"
#include "../base/Rect.h"

#include <boost/shared_ptr.hpp>

namespace avg {

class SDLDisplayEngine;
class VertexArray;

typedef std::vector<std::vector<DPoint> > VertexGrid;

class OGLTexture {
    public:
        OGLTexture(IntRect TexExtent, IntPoint TexSize, IntPoint TileSize, 
                IntRect TileIndexExtent, int Stride, PixelFormat pf,
                SDLDisplayEngine * pEngine);
        virtual ~OGLTexture();
        void resize(IntRect TexExtent, IntPoint TexSize, IntPoint TileSize, 
                int Stride);

        const IntPoint& getTexSize() const;
        int getTexID(int i) const;
        void downloadTexture(int i, BitmapPtr pBmp, int width, 
                OGLMemoryMode MemoryMode) const;
        void blt(const VertexGrid* pVertexes) const;
        const IntRect& getTileIndexExtent() const;
        const int getTexMemDim();
        const PixelFormat getPixelFormat() const;

    private:
        void calcTexCoords();
        void createTextures(int Stride);
        void createTexture(int i, IntPoint Size, int Stride, PixelFormat pf);
        void deleteTextures();

        IntRect m_TexExtent;  // Extent of Texture in the PBO.
        IntPoint m_TexSize;   // Size of Texture in pixels. POW2 if necessary.
        IntPoint m_TileSize;  // Size of Tiles in pixels. Always POW2.
        IntRect m_TileIndexExtent; // Tile indexes in the surface used in this texture.
        PixelFormat m_pf;
        SDLDisplayEngine * m_pEngine;
        int m_TextureMode;
        unsigned int m_TexID[3];
        VertexArray * m_pVertexes;
        std::vector<std::vector<DPoint> > m_TexCoords;
};

typedef boost::shared_ptr<OGLTexture> OGLTexturePtr;

}

#endif

