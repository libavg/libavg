//
// $Id$
//

#ifndef _AVGDFBSurface_H_
#define _AVGDFBSurface_H_

#include "IAVGSurface.h"

#include <paintlib/plsubbmp.h>

#include <directfb.h>

class AVGDFBSurface: public IAVGSurface {
    public:
        static void SetDirectFB (IDirectFB * pDirectFB);

        AVGDFBSurface();

        virtual ~AVGDFBSurface();

        // Implementation of IAVGSurface.

        virtual void create(int Width, int Height, int bpp, 
                bool bHasAlpha);

        virtual PLBmpBase* getBmp();

        // Methods specific to AVGDFBSurface

        // Creates a surface containing a PLSubBmp that encompasses
        // a recangle in a DirectFBSurface. Ownership of the 
        // original DirectFBSurface remains with the caller. An internal
        // DirectFBSurface is created via IDirectFBSurface::GetSubSurface().
        void createFromDFBSurface(IDirectFBSurface * pSurface,
                const PLRect * SrcRect);

        IDirectFBSurface* getSurface();

    private:
        static IDirectFB * s_pDirectFB; 
        IDirectFBSurface * m_pSurface;
        PLSubBmp m_Bmp;
};

#endif

