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


        static int getTextureMode();

    private:
        struct TextureTile {
            PLRect m_Extent;
            unsigned int m_TexID;
            int m_TexWidth;
            int m_TexHeight;
        };
        
        void bindOneTexture(TextureTile& Tile);
        void bltTexture(const AVGDRect* pDestRect, 
                double angle, const AVGDPoint& pivot);
        int AVGOGLSurface::bltTile(const TextureTile& Tile, 
                const AVGDRect& DestRect);
        int getDestMode();
        int getSrcMode();
   
        PLBmpBase * m_pBmp;
        PLSubBmp * m_pSubBmp;   // Cached pointer to avoid slow dynamic_cast.

        PLPoint m_TileSize;
        std::vector<std::vector<TextureTile> > m_Tiles;

        bool m_bBound;

        static int s_TextureMode;
        static int s_MaxTexSize;
};

#endif

