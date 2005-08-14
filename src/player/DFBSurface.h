//
// $Id$
//

#ifndef _DFBSurface_H_
#define _DFBSurface_H_

#include "ISurface.h"
#include "../graphics/Bitmap.h"
#include "../graphics/Rect.h"

#include <directfb.h>

namespace avg {

class DFBSurface: public ISurface {
    public:
        static void SetDirectFB (IDirectFB * pDirectFB);

        DFBSurface();

        virtual ~DFBSurface();

        // Implementation of ISurface.

        virtual void create(const IntPoint& Size, PixelFormat pf);

        virtual BitmapPtr getBmp();

        // Methods specific to DFBSurface

        // Creates a surface containing a Bitmap that encompasses
        // a recangle in a DirectFBSurface. Ownership of the 
        // original DirectFBSurface remains with the caller. An internal
        // DirectFBSurface is created via IDirectFBSurface::GetSubSurface().
        void createFromDFBSurface(IDirectFBSurface * pSurface,
                const IntRect * SrcRect);

        IDirectFBSurface* getSurface();

    private:
        static IDirectFB * s_pDirectFB; 
        IDirectFBSurface * m_pSurface;
        BitmapPtr m_pBmp;
};

}

#endif

