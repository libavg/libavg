//
// $Id$
//

#ifndef _DFBSurface_H_
#define _DFBSurface_H_

#include "ISurface.h"

#include <paintlib/plsubbmp.h>

#include <directfb.h>

namespace avg {

class DFBSurface: public ISurface {
    public:
        static void SetDirectFB (IDirectFB * pDirectFB);

        DFBSurface();

        virtual ~DFBSurface();

        // Implementation of ISurface.

        virtual void create(int Width, int Height, const PLPixelFormat& pf);

        virtual PLBmpBase* getBmp();

        // Methods specific to DFBSurface

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

}

#endif

