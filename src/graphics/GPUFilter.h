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
#include "VertexArray.h"
#include "Bitmap.h"
#include "PBO.h"
#include "FBO.h"

namespace avg {

class AVG_API GPUFilter: public Filter
{
public:
    GPUFilter(const IntPoint& size, PixelFormat pfSrc, PixelFormat pfDest, 
            unsigned numTextures=1);
    virtual ~GPUFilter();

    virtual BitmapPtr apply(BitmapPtr pBmpSource);
    virtual void apply(GLTexturePtr pSrcTex);
    virtual void applyOnGPU(GLTexturePtr pSrcTex) = 0;
    FBOPtr getFBO();
    GLTexturePtr getDestTex(int i=0) const;

    static bool isSupported();

protected:
    void draw(GLTexturePtr pTex);
    const IntPoint& getSize() const;

private:
    void initVertexArray();

    GLTexturePtr m_pSrcTex;
    PBOPtr m_pSrcPBO;
    FBOPtr m_pFBO;
    VertexArray * m_pVertexes;
};

} // namespace
#endif

