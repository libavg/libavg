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

#include "GPUBlurFilter.h"
#include "Bitmap.h"
#include "ShaderRegistry.h"
#include "ImagingProjection.h"
#include "OGLShader.h"

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

    setDimensions(size, stdDev, bClipBorders);
    createShader(SHADERID_VERT);
    setStdDev(stdDev);

    OGLShaderPtr pShader = getShader();
    m_pHorizWidthParam = pShader->getParam<float>("u_Width");
    m_pHorizRadiusParam = pShader->getParam<int>("u_Radius");
    m_pHorizTextureParam = pShader->getParam<int>("u_Texture");
    m_pHorizKernelTexParam = pShader->getParam<int>("u_KernelTex");

    pShader = avg::getShader(SHADERID_VERT);
    m_pVertWidthParam = pShader->getParam<float>("u_Width");
    m_pVertRadiusParam = pShader->getParam<int>("u_Radius");
    m_pVertTextureParam = pShader->getParam<int>("u_Texture");
    m_pVertKernelTexParam = pShader->getParam<int>("u_KernelTex");
}

GPUBlurFilter::~GPUBlurFilter()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void GPUBlurFilter::setStdDev(float stdDev)
{
    m_StdDev = stdDev;
    m_pGaussCurveTex = calcBlurKernelTex(m_StdDev, 1, m_bUseFloatKernel);
    setDimensions(getSrcSize(), stdDev, m_bClipBorders);
    IntRect destRect2(IntPoint(0,0), getDestRect().size());
    m_pProjection2 = ImagingProjectionPtr(new ImagingProjection(
            getDestRect().size(), destRect2));
}

void GPUBlurFilter::applyOnGPU(GLTexturePtr pSrcTex)
{
    int kernelWidth = m_pGaussCurveTex->getSize().x;
    getFBO(1)->activate();
    getShader()->activate();
    m_pHorizWidthParam->set(float(kernelWidth));
    m_pHorizRadiusParam->set((kernelWidth-1)/2);
    m_pHorizTextureParam->set(0);
    m_pHorizKernelTexParam->set(1);
    m_pGaussCurveTex->activate(GL_TEXTURE1);
    draw(pSrcTex);

    getFBO(0)->activate();
    OGLShaderPtr pVShader = avg::getShader(SHADERID_VERT);
    pVShader->activate();
    m_pVertWidthParam->set(float(kernelWidth));
    m_pVertRadiusParam->set((kernelWidth-1)/2);
    m_pVertTextureParam->set(0);
    m_pVertKernelTexParam->set(1);
    getDestTex(1)->activate(GL_TEXTURE0);
    m_pProjection2->draw(pVShader);
}

void GPUBlurFilter::setDimensions(IntPoint size, float stdDev, bool bClipBorders)
{
    
#ifndef AVG_ENABLE_EGL
    if (bClipBorders) {
        GPUFilter::setDimensions(size);
    } else {
        int radius = getBlurKernelRadius(stdDev);
        IntPoint offset(radius, radius);
        //TODO: TO_BORDER DOES NOT EXIST IN GLESV2
        GPUFilter::setDimensions(size, IntRect(-offset, size+offset), GL_CLAMP_TO_BORDER);
    }
#endif
}

}
