//
// $Id$
//

#ifndef _OGLSurface_H_
#define _OGLSurface_H_

#include "ISurface.h"
#include "DisplayEngine.h"
#include "../graphics/Rect.h"

#define GL_GLEXT_PROTOTYPES
#include "GL/gl.h"

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
            MESA, // glXAllocateMemoryMESA
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
        GLuint m_hPixelBuffer;
        void * m_pMESABuffer;
        
        static int s_TextureMode;
        static int s_MaxTexSize;
};

}

#endif

