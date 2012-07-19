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

#ifndef _GPUShadowFilter_H_
#define _GPUShadowFilter_H_

#include "../api.h"
#include "GPUFilter.h"
#include "GLShaderParam.h"
#include "GLTexture.h"

#include "../base/GLMHelper.h"

namespace avg {

class AVG_API GPUShadowFilter: public GPUFilter
{
public:
    GPUShadowFilter(const IntPoint& size, const glm::vec2& offset, float stdDev,
            float opacity, const Pixel32& color);
    virtual ~GPUShadowFilter();
    
    void setParams(const glm::vec2& offset, float stdDev, float opacity, 
            const Pixel32& color);
    virtual void applyOnGPU(GLTexturePtr pSrcTex);

private:
    void setDimensions(IntPoint size, float stdDev, const glm::vec2& offset);

    glm::vec2 m_Offset;
    float m_StdDev;
    float m_Opacity;
    Pixel32 m_Color;

    GLTexturePtr m_pGaussCurveTex;
    ImagingProjectionPtr m_pProjection2;

    FloatGLShaderParamPtr m_pHorizWidthParam;
    IntGLShaderParamPtr m_pHorizRadiusParam;
    IntGLShaderParamPtr m_pHorizTextureParam;
    IntGLShaderParamPtr m_pHorizKernelTexParam;
    Vec2fGLShaderParamPtr m_pHorizOffsetParam;

    FloatGLShaderParamPtr m_pVertWidthParam;
    IntGLShaderParamPtr m_pVertRadiusParam;
    IntGLShaderParamPtr m_pVertTextureParam;
    IntGLShaderParamPtr m_pVertKernelTexParam;
    ColorGLShaderParamPtr m_pVertColorParam;
    IntGLShaderParamPtr m_pVertOrigTexParam;
    Vec2fGLShaderParamPtr m_pVertDestPosParam;
    Vec2fGLShaderParamPtr m_pVertDestSizeParam;
};

typedef boost::shared_ptr<GPUShadowFilter> GPUShadowFilterPtr;

}
#endif

