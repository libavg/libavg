//
// $Id$
//

#ifndef _OGLSurface_H_
#define _OGLSurface_H_

#include "ISurface.h"
#include "IDisplayEngine.h"
#include "Rect.h"

#include <paintlib/plsubbmp.h>
#include <paintlib/plrect.h>

#include <vector>

namespace avg {

class OGLSurface: public ISurface {
    public:
        OGLSurface();

        virtual ~OGLSurface();

        // Implementation of ISurface.

        virtual void create(int Width, int Height, const PLPixelFormat& pf);

        virtual PLBmpBase* getBmp();

        // Methods specific to OGLSurface
        void createFromBits(int Width, int Height, const PLPixelFormat& pf,
                PLBYTE* pBits, int Stride);

        // Discards the bitmap data but leaves the texture intact.
        void discardBmp();

        void bind();
        void unbind();
        void rebind();

        void blt(const DRect* pDestRect, double opacity, 
                double angle, const DPoint& pivot, 
                IDisplayEngine::BlendMode Mode);
        unsigned int getTexID();

        void setMaxTileSize(const PLPoint& MaxTileSize);
        int getNumVerticesX();
        int getNumVerticesY();
        DPoint getOrigVertexCoord(int x, int y);
        DPoint getWarpedVertexCoord(int x, int y);
        void setWarpedVertexCoord(int x, int y, const DPoint& Vertex);
 
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
        void initTileVertex (int x, int y, DPoint& Vertex);

        void bindOneTexture(TextureTile& Tile);
        void bltTexture(const DRect* pDestRect, 
                double angle, const DPoint& pivot, 
                IDisplayEngine::BlendMode Mode);
        DPoint calcFinalVertex(const DRect* pDestRect,
                const DPoint & NormalizedVertex);
        void bltTile(const TextureTile& Tile, 
                const DPoint& TLPoint, const DPoint& TRPoint,
                const DPoint& BLPoint, const DPoint& BRPoint);
        int getDestMode();
        int getSrcMode();
   
        bool m_bBound;

        PLBmpBase * m_pBmp;
        PLSubBmp * m_pSubBmp;   // Cached pointer to avoid slow dynamic_cast.

        PLPoint m_MaxTileSize;
        PLPoint m_TileSize;
        int m_NumHorizTextures;
        int m_NumVertTextures;
        std::vector<std::vector<TextureTile> > m_Tiles;
        std::vector<std::vector<DPoint> > m_TileVertices;

        static int s_TextureMode;
        static int s_MaxTexSize;
};

}

#endif

