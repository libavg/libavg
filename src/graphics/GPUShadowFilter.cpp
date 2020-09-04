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

#include "GPUShadowFilter.h"
#include "Bitmap.h"
#include "ShaderRegistry.h"
#include "OGLShader.h"
#include "ImagingProjection.h"
#include "FBO.h"
#include "GLContextManager.h"
#include "GLTexture.h"

#include "../base/ObjectCounter.h"
#include "../base/MathHelper.h"
#include "../base/Exception.h"

#include <string.h>
#include <iostream>

#define SHADERID_HORIZ "horizshadow"
#define SHADERID_VERT "vertshadow"

using namespace std;

namespace avg {

GPUShadowFilter::GPUShadowFilter(const IntPoint& size, const glm::vec2& offset, 
        float stdDev, float opacity, const Pixel32& color)
    : GPUFilter(SHADERID_HORIZ, true, false, 2)
{
    ObjectCounter::get()->incRef(&typeid(*this));

    GLContext::getCurrent()->ensureFullShaders("GPUShadowFilter");
#ifndef AVG_ENABLE_EGL
    m_WrapMode = WrapMode(GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);
#endif

    setDimensions(size, stdDev, offset);
    GLContextManager* pCM = GLContextManager::get();
    pCM->createShader(SHADERID_VERT);
    setParams(offset, stdDev, opacity, color);
    
    m_pHorizWidthParam = pCM->createShaderParam<float>(SHADERID_HORIZ, "u_Width");
    m_pHorizRadiusParam = pCM->createShaderParam<int>(SHADERID_HORIZ, "u_Radius");
    m_pHorizTextureParam = pCM->createShaderParam<int>(SHADERID_HORIZ, "u_Texture");
    m_pHorizKernelTexParam = pCM->createShaderParam<int>(SHADERID_HORIZ, "u_KernelTex");
    m_pHorizOffsetParam = pCM->createShaderParam<glm::vec2>(SHADERID_HORIZ, "u_Offset");

    m_pVertWidthParam = pCM->createShaderParam<float>(SHADERID_VERT, "u_Width");
    m_pVertRadiusParam = pCM->createShaderParam<int>(SHADERID_VERT, "u_Radius");
    m_pVertTextureParam = pCM->createShaderParam<int>(SHADERID_VERT, "u_HBlurTex");
    m_pVertKernelTexParam = pCM->createShaderParam<int>(SHADERID_VERT, "u_KernelTex");
    m_pVertColorParam = pCM->createShaderParam<Pixel32>(SHADERID_VERT, "u_Color");
    m_pVertOrigTexParam = pCM->createShaderParam<int>(SHADERID_VERT, "u_OrigTex");
    m_pVertDestPosParam = pCM->createShaderParam<glm::vec2>(SHADERID_VERT, "u_DestPos");
    m_pVertDestSizeParam = pCM->createShaderParam<glm::vec2>(SHADERID_VERT, "u_DestSize");
}

GPUShadowFilter::~GPUShadowFilter()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void GPUShadowFilter::setParams(const glm::vec2& offset, float stdDev, float opacity, 
        const Pixel32& color)
{
    m_Offset = offset;
    m_StdDev = stdDev;
    m_Opacity = opacity;
    m_Color = color;
    m_pGaussCurveTex = calcBlurKernelTex(m_StdDev, m_Opacity, false);
    setDimensions(getSrcSize(), stdDev, offset);
    IntRect destRect2(IntPoint(0,0), getDestRect().size());
    m_pProjection2 = ImagingProjectionPtr(new ImagingProjection(
            getDestRect().size(), destRect2));
}

void GPUShadowFilter::applyOnGPU(GLContext* pContext, GLTexturePtr pSrcTex)
{
    int kernelWidth = m_pGaussCurveTex->getSize().x;
    getFBO(pContext, 1)->activate();
    getShader()->activate();
    m_pHorizWidthParam->set(pContext, float(kernelWidth));
    m_pHorizRadiusParam->set(pContext, (kernelWidth-1)/2);
    m_pHorizTextureParam->set(pContext, 0);
    m_pHorizKernelTexParam->set(pContext, 1);
    IntPoint size = getSrcSize();
    glm::vec2 texOffset(m_Offset.x/size.x, m_Offset.y/size.y);
    m_pHorizOffsetParam->set(pContext, texOffset);
    m_pGaussCurveTex->getTex(pContext)->activate(WrapMode(), GL_TEXTURE1);
    draw(pContext, pSrcTex, m_WrapMode);

    getFBO(pContext, 0)->activate();
    OGLShaderPtr pVShader = avg::getShader(SHADERID_VERT);
    pVShader->activate();
    m_pVertWidthParam->set(pContext, float(kernelWidth));
    m_pVertRadiusParam->set(pContext, (kernelWidth-1)/2);
    m_pVertTextureParam->set(pContext, 0);
    m_pVertKernelTexParam->set(pContext, 1);
    m_pVertColorParam->set(pContext, m_Color);

    pSrcTex->activate(m_WrapMode, GL_TEXTURE2);
    m_pVertOrigTexParam->set(pContext, 2);
    FRect destRect = getRelDestRect();
    m_pVertDestPosParam->set(pContext, destRect.tl);
    m_pVertDestSizeParam->set(pContext, destRect.size());
#ifndef AVG_ENABLE_EGL
    getDestTex(pContext, 1)->activate(m_WrapMode, GL_TEXTURE0);
#endif
    m_pProjection2->draw(pContext, avg::getShader(SHADERID_VERT));
}

void GPUShadowFilter::setDimensions(IntPoint size, float stdDev, const glm::vec2& offset)
{
    int radius = getBlurKernelRadius(stdDev);
    IntPoint radiusOffset(radius, radius);
    IntPoint intOffset(offset);
    IntRect destRect(intOffset-radiusOffset, intOffset+size+radiusOffset+IntPoint(1,1));
    destRect.expand(IntRect(IntPoint(0,0), size));
    GPUFilter::setDimensions(size, destRect);
}
 
}
