//
// $Id$
//

#ifndef _AVGOGLSurface_H_
#define _AVGOGLSurface_H_

#include "IAVGSurface.h"
#include "AVGRect.h"

#include <paintlib/plsubbmp.h>
#include <paintlib/plrect.h>

#include <vector>

class AVGOGLSurface: public IAVGSurface {
    public:
        AVGOGLSurface();

        virtual ~AVGOGLSurface();

        // Implementation of IAVGSurface.

        virtual void create(int Width, int Height, int bpp, 
                bool bHasAlpha);

        virtual PLBmpBase* getBmp();

        // Methods specific to AVGOGLSurface
        void createFromBits(int Width, int Height, int bpp, 
                bool bHasAlpha, PLBYTE* pBits, int Stride);

        // Discards the bitmap data but leaves the texture intact.
        void discardBmp();

        void bind();
        void unbind();
        void rebind();

        void blt(const AVGDRect* pDestRect, double opacity, 
                double angle, const AVGDPoint& pivot);
        unsigned int getTexID();

        void setMaxTileSize(const PLPoint& MaxTileSize);
        int getNumVerticesX();
        int getNumVerticesY();
        AVGDPoint getOrigVertexCoord(int x, int y);
        AVGDPoint getWarpedVertexCoord(int x, int y);
        void setWarpedVertexCoord(int x, int y, const AVGDPoint& Vertex);
        
        static int getTextureMode();

    private:
        struct TextureTile {
            PLRect m_Extent;
            unsigned int m_TexID;
            int m_TexWidth;
            int m_TexHeight;
        };
        
        void setupTiles();
        void initTileVertices();
        void initTileVertex (int x, int y, AVGDPoint& Vertex);

        void bltTexture(const AVGDRect* pDestRect, 
                double angle, const AVGDPoint& pivot);
        AVGDPoint calcFinalVertex(const AVGDRect* pDestRect,
                const AVGDPoint & NormalizedVertex);
        int bltTile(const TextureTile& Tile, 
                const AVGDPoint& TLPoint, const AVGDPoint& TRPoint,
                const AVGDPoint& BLPoint, const AVGDPoint& BRPoint);
        int getDestMode();
        int getSrcMode();
   
        PLBmpBase * m_pBmp;
        PLSubBmp * m_pSubBmp;   // Cached pointer to avoid slow dynamic_cast.

        PLPoint m_MaxTileSize;
        PLPoint m_TileSize;
        int m_NumHorizTextures;
        int m_NumVertTextures;
        std::vector<std::vector<TextureTile> > m_Tiles;
        std::vector<std::vector<AVGDPoint> > m_TileVertices;

        bool m_bBound;

        static int s_TextureMode;
        static int s_MaxTexSize;
};

#endif

