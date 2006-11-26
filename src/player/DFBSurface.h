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

        virtual void create(const IntPoint& Size, PixelFormat pf, 
                bool bFastDownload);

        virtual BitmapPtr lockBmp(int i = 0);

        PixelFormat getPixelFormat();
        IntPoint getSize();

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

