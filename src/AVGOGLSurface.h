//
// $Id$
//

#ifndef _AVGOGLSurface_H_
#define _AVGOGLSurface_H_

#include "IAVGSurface.h"

#include <paintlib/plsubbmp.h>

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
        int getTexID();
        int getTexWidth();
        int getTexHeight();

        static int getTextureMode();

    private:
        int getSrcMode();
   
        PLBmpBase * m_pBmp;

        unsigned int m_TexID;
        bool m_bBound;

        int m_TexWidth;
        int m_TexHeight;

        static int s_TextureMode;
        static int s_MaxTexSize;
};

#endif

