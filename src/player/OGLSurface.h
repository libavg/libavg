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
#include "../graphics/Rect.h"

#define GL_GLEXT_PROTOTYPES
#include "GL/gl.h"
#ifndef __APPLE__
#define GLX_GLXEXT_PROTOTYPES
#include "GL/glx.h"
#endif

#include <vector>
#include <string>

namespace avg {

class OGLSurface: public ISurface {
    public:
        OGLSurface();
        virtual ~OGLSurface();

        // Implementation of ISurface.
        virtual void create(const IntPoint& Size, PixelFormat PF,
                bool bFastDownload);
        virtual BitmapPtr lockBmp();
        virtual void unlockBmp();

        // Methods specific to OGLSurface
        void createFromBits(Point<int> Size, PixelFormat pf,
                unsigned char * pBits, int Stride);

        // Discards the bitmap data but leaves the texture intact.
        void discardBmp();

        void bind();
        void unbind();
        void rebind();

        void blt(const DRect* pDestRect, double opacity, 
                double angle, const DPoint& pivot, 
                DisplayEngine::BlendMode Mode);
        unsigned int getTexID();

        void setMaxTileSize(const Point<int>& MaxTileSize);
        int getNumVerticesX();
        int getNumVerticesY();
        DPoint getOrigVertexCoord(int x, int y);
        DPoint getWarpedVertexCoord(int x, int y);
        void setWarpedVertexCoord(int x, int y, const DPoint& Vertex);
 
        static int getTextureMode();

    private:
        enum MemoryMode { 
            OGL,  // Standard OpenGL
            PBO   // pixel buffer objects
        };
        struct TextureTile {
            IntRect m_Extent;
            unsigned int m_TexID;
            int m_TexWidth;
            int m_TexHeight;
        };
        
        void setupTiles();
        void initTileVertices();
        void initTileVertex (int x, int y, DPoint& Vertex);

        void bindOneTexture(TextureTile& Tile);
        void bltTexture(const DRect* pDestRect, 
                double angle, const DPoint& pivot, 
                DisplayEngine::BlendMode Mode);
        DPoint calcFinalVertex(const DRect* pDestRect,
                const DPoint & NormalizedVertex);
        void bltTile(const TextureTile& Tile, 
                const DPoint& TLPoint, const DPoint& TRPoint,
                const DPoint& BLPoint, const DPoint& BRPoint);
        int getDestMode();
        int getSrcMode();
        int getPixelType();
        void checkBlendModeError(std::string sMode);

        static MemoryMode getMemoryModeSupported();
   
        bool m_bBound;

        BitmapPtr m_pBmp;
        IntPoint m_Size;
        PixelFormat m_pf;

        Point<int> m_MaxTileSize;
        Point<int> m_TileSize;
        int m_NumHorizTextures;
        int m_NumVertTextures;
        std::vector<std::vector<TextureTile> > m_Tiles;
        std::vector<std::vector<DPoint> > m_TileVertices;

        MemoryMode m_MemoryMode;

        // PBO memory mode
        GLuint m_hPixelBuffer;
        static PFNGLGENBUFFERSPROC s_GenBuffersProc;
        static PFNGLBUFFERDATAPROC s_BufferDataProc;
        static PFNGLDELETEBUFFERSPROC s_DeleteBuffersProc;
        static PFNGLBINDBUFFERPROC s_BindBufferProc;
        static PFNGLMAPBUFFERPROC s_MapBufferProc;
        static PFNGLUNMAPBUFFERPROC s_UnmapBufferProc;

        static int s_TextureMode;
        static int s_MaxTexSize;
};

}

#endif

