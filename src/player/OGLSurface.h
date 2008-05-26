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

#ifndef _OGLSurface_H_
#define _OGLSurface_H_

#include "ISurface.h"
#include "DisplayEngine.h"
#include "OGLTexture.h"

#include "../base/Point.h"
#include "../base/Rect.h"

#include "../graphics/OGLHelper.h"

#include <vector>
#include <string>

namespace avg {

class SDLDisplayEngine;

class OGLSurface: public ISurface {
    public:
        OGLSurface(SDLDisplayEngine * pEngine);
        virtual ~OGLSurface();

        // Implementation of ISurface.
        virtual void create(const IntPoint& Size, PixelFormat PF,
                bool bFastDownload);
        virtual BitmapPtr lockBmp(int index=0);
        virtual void unlockBmps();

        // Methods specific to OGLSurface
        void createFromBits(Point<int> Size, PixelFormat pf,
                unsigned char * pBits, int Stride);

        void bind();
        void unbind();
        void rebind();

        PixelFormat getPixelFormat();
        IntPoint getSize();

        void blt(const DPoint& DestSize, DisplayEngine::BlendMode Mode);

        void setMaxTileSize(const IntPoint& MaxTileSize);
        VertexGrid getOrigVertexCoords();
        VertexGrid getWarpedVertexCoords();
        void setWarpedVertexCoords(const VertexGrid& Grid);
 
        bool isOneTexture(IntPoint Size);
        
        int getTotalTexMemory();

    private:
        void calcTileSizes();
        void initTileVertices(VertexGrid& Grid);
        void initTileVertex (int x, int y, DPoint& Vertex);

        void createBitmap(const IntPoint& Size, PixelFormat pf, int index);
        void deleteBuffers();
        void unlockBmp(int i);
        void bindOneTexture(OGLTexture& Texture);
        void bltTexture(const DPoint& DestSize, DisplayEngine::BlendMode Mode);
        DPoint calcFinalVertex(const DPoint& Size,
                const DPoint & NormalizedVertex);
        void checkBlendModeError(std::string sMode);

        SDLDisplayEngine * m_pEngine;
       
        bool m_bCreated;
        bool m_bBound;

        BitmapPtr m_pBmps[3];
        IntPoint m_Size;
        PixelFormat m_pf;

        IntPoint m_TextureSize;
        IntPoint m_NumTextures;
        IntPoint m_MaxTileSize;
        IntPoint m_TileSize;
        IntPoint m_NumTiles;
        std::vector<std::vector<OGLTexturePtr> > m_pTextures;
        VertexGrid m_TileVertices;

        OGLMemoryMode m_MemoryMode;

        // PBO memory mode
        GLuint m_hPixelBuffers[3];
};

}

#endif

