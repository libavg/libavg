//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2011 Ulrich von Zadow
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

#ifndef _GPUBlurFilter_H_
#define _GPUBlurFilter_H_

#include "../api.h"
#include "GPUFilter.h"
#include "GLShaderParam.h"
#include "GLTexture.h"

namespace avg {

class AVG_API GPUBlurFilter: public GPUFilter
{
public:
    GPUBlurFilter(const IntPoint& size, PixelFormat pfSrc, PixelFormat pfDest, 
            float stdDev, bool bClipBorders, bool bStandalone=true, 
            bool bUseFloatKernel=false);
    virtual ~GPUBlurFilter();
    
    void setStdDev(float stdDev);
    virtual void applyOnGPU(GLTexturePtr pSrcTex);

private:
    void setDimensions(IntPoint size, float stdDev, bool bClipBorders);

    float m_StdDev;
    bool m_bClipBorders;
    bool m_bUseFloatKernel;

    GLTexturePtr m_pGaussCurveTex;
    ImagingProjectionPtr m_pProjection2;

    FloatGLShaderParamPtr m_pHorizWidthParam;
    IntGLShaderParamPtr m_pHorizRadiusParam;
    IntGLShaderParamPtr m_pHorizTextureParam;
    IntGLShaderParamPtr m_pHorizKernelTexParam;

    FloatGLShaderParamPtr m_pVertWidthParam;
    IntGLShaderParamPtr m_pVertRadiusParam;
    IntGLShaderParamPtr m_pVertTextureParam;
    IntGLShaderParamPtr m_pVertKernelTexParam;
};

typedef boost::shared_ptr<GPUBlurFilter> GPUBlurFilterPtr;

}
#endif

