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

#include <boost/thread/tss.hpp>

namespace avg {

class ImagingProjection;
typedef boost::shared_ptr<ImagingProjection> ImagingProjectionPtr;

class AVG_API GPUFilter: public Filter
{
public:
    GPUFilter(PixelFormat pfSrc, PixelFormat pfDest, bool bStandalone,
            unsigned numTextures=1);
    virtual ~GPUFilter();
    void setDimensions(const IntPoint& srcSize);
    void setDimensions(const IntPoint& srcSize, const IntRect& destRect,
            unsigned texMode);

    virtual BitmapPtr apply(BitmapPtr pBmpSource);
    virtual void apply(GLTexturePtr pSrcTex);
    virtual void applyOnGPU(GLTexturePtr pSrcTex) = 0;
    GLTexturePtr getDestTex(int i=0) const;
    BitmapPtr getImage() const;
    FBOPtr getFBO();

    const IntRect& getDestRect() const;
    const IntPoint& getSrcSize() const;
    DRect getRelDestRect() const;
    
    static void glContextGone();

protected:
    void draw(GLTexturePtr pTex);
    const std::string& getStdShaderCode() const;
    int getBlurKernelRadius(double stdDev) const;
    GLTexturePtr calcBlurKernelTex(double stdDev, double opacity=1) const;

private:
    PixelFormat m_PFSrc;
    PixelFormat m_PFDest;
    bool m_bStandalone;
    unsigned m_NumTextures;

    GLTexturePtr m_pSrcTex;
    PBOPtr m_pSrcPBO;
    FBOPtr m_pFBO;
    IntPoint m_SrcSize;
    IntRect m_DestRect;
    ImagingProjectionPtr m_pProjection;

    static boost::thread_specific_ptr<PBOPtr> s_pFilterKernelPBO;
};

typedef boost::shared_ptr<GPUFilter> GPUFilterPtr;

} // namespace
#endif

