//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2020 Ulrich von Zadow
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

#include "GPUBlurFilter.h"
#include "Bitmap.h"
#include "ShaderRegistry.h"
#include "ImagingProjection.h"
#include "OGLShader.h"
#include "GLContextManager.h"
#include "FBO.h"
#include "MCTexture.h"
#include "GLTexture.h"

#include "../base/ObjectCounter.h"
#include "../base/MathHelper.h"
#include "../base/Exception.h"

#include <string.h>
#include <iostream>

#define SHADERID_HORIZ "horizblur"
#define SHADERID_VERT "vertblur"

using namespace std;

namespace avg {

GPUBlurFilter::GPUBlurFilter(const IntPoint& size, PixelFormat pfSrc, PixelFormat pfDest,
        float stdDev, bool bClipBorders, bool bStandalone, bool bUseFloatKernel)
    : GPUFilter(pfSrc, pfDest, bStandalone, SHADERID_HORIZ, 2),
      m_bClipBorders(bClipBorders),
      m_bUseFloatKernel(bUseFloatKernel)
{
    ObjectCounter::get()->incRef(&typeid(*this));

    GLContext::getCurrent()->ensureFullShaders("GPUBlurFilter");
#ifndef AVG_ENABLE_EGL
    if (!m_bClipBorders) {
        //TODO: TO_BORDER DOES NOT EXIST IN GLESV2
        m_WrapMode = WrapMode(GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);
    }
#endif
    setDimensions(size, stdDev);
    GLContextManager* pCM = GLContextManager::get();
    pCM->createShader(SHADERID_VERT);
    setStdDev(stdDev);

    m_pHorizWidthParam = pCM->createShaderParam<float>(SHADERID_HORIZ, "u_Width");
    m_pHorizRadiusParam = pCM->createShaderParam<int>(SHADERID_HORIZ, "u_Radius");
    m_pHorizTextureParam = pCM->createShaderParam<int>(SHADERID_HORIZ, "u_Texture");
    m_pHorizKernelTexParam = pCM->createShaderParam<int>(SHADERID_HORIZ, "u_KernelTex");

    m_pVertWidthParam = pCM->createShaderParam<float>(SHADERID_VERT, "u_Width");
    m_pVertRadiusParam = pCM->createShaderParam<int>(SHADERID_VERT, "u_Radius");
    m_pVertTextureParam = pCM->createShaderParam<int>(SHADERID_VERT, "u_Texture");
    m_pVertKernelTexParam = pCM->createShaderParam<int>(SHADERID_VERT, "u_KernelTex");
}

GPUBlurFilter::~GPUBlurFilter()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void GPUBlurFilter::setStdDev(float stdDev)
{
    m_StdDev = stdDev;
    m_pGaussCurveTex = calcBlurKernelTex(m_StdDev, 1, m_bUseFloatKernel);
    setDimensions(getSrcSize(), stdDev);
    IntRect destRect2(IntPoint(0,0), getDestRect().size());
    m_pProjection2 = ImagingProjectionPtr(new ImagingProjection(
            getDestRect().size(), destRect2));
}

void GPUBlurFilter::applyOnGPU(GLContext* pContext, GLTexturePtr pSrcTex)
{
    int kernelWidth = m_pGaussCurveTex->getSize().x;
    getFBO(pContext, 1)->activate();
    getShader()->activate();
    m_pHorizWidthParam->set(pContext, float(kernelWidth));
    m_pHorizRadiusParam->set(pContext, (kernelWidth-1)/2);
    m_pHorizTextureParam->set(pContext, 0);
    m_pHorizKernelTexParam->set(pContext, 1);
    m_pGaussCurveTex->getTex(pContext)->activate(WrapMode(), GL_TEXTURE1);
    draw(pContext, pSrcTex, m_WrapMode);

    getFBO(pContext, 0)->activate();
    OGLShaderPtr pVShader = avg::getShader(SHADERID_VERT);
    pVShader->activate();
    m_pVertWidthParam->set(pContext, float(kernelWidth));
    m_pVertRadiusParam->set(pContext, (kernelWidth-1)/2);
    m_pVertTextureParam->set(pContext, 0);
    m_pVertKernelTexParam->set(pContext, 1);
    getDestTex(pContext, 1)->activate(m_WrapMode, GL_TEXTURE0);
    m_pProjection2->draw(pContext, pVShader);
}

void GPUBlurFilter::setDimensions(IntPoint size, float stdDev)
{
    if (m_bClipBorders) {
        GPUFilter::setDimensions(size);
    } else {
        int radius = getBlurKernelRadius(stdDev);
        IntPoint offset(radius, radius);
        GPUFilter::setDimensions(size, IntRect(-offset, size+offset));
    }
}

}
