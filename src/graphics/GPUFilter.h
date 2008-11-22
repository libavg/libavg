//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2008 Ulrich von Zadow
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

#ifndef _GPUFilter_H_
#define _GPUFilter_H_

#include "../api.h"
#include "Filter.h"
#include "Bitmap.h"
#include "PBOImage.h"
#include "FBOImage.h"

namespace avg {

class AVG_API GPUFilter: public Filter
{
public:
    GPUFilter(const IntPoint& size, PixelFormat pfSrc);
    GPUFilter(PBOImagePtr pSrcPBO, FBOImagePtr pDestFBO);
    virtual ~GPUFilter();

    virtual BitmapPtr apply(BitmapPtr pBmpSource);
    virtual void applyOnGPU() = 0;
    static bool isSupported();

protected:
    const IntPoint& getSize() const;

    PBOImagePtr getSrcPBO();
    FBOImagePtr getDestFBO();

private:
    PBOImagePtr m_pSrcPBO;
    FBOImagePtr m_pDestFBO;
    
};

} // namespace
#endif

