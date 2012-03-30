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
        float stdDev, bool bClipBorders, bool bStandalone)
    : GPUFilter(pfSrc, pfDest, bStandalone, 2)
{
    ObjectCounter::get()->incRef(&typeid(*this));

    setDimensions(size, stdDev, bClipBorders);
    createShader(SHADERID_HORIZ);
    createShader(SHADERID_VERT);
    m_bClipBorders = bClipBorders;
    setStdDev(stdDev);
}

GPUBlurFilter::~GPUBlurFilter()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void GPUBlurFilter::setStdDev(float stdDev)
{
    m_StdDev = stdDev;
    m_pGaussCurveTex = calcBlurKernelTex(m_StdDev);
    setDimensions(getSrcSize(), stdDev, m_bClipBorders);
    IntRect destRect2(IntPoint(0,0), getDestRect().size());
    m_pProjection2 = ImagingProjectionPtr(new ImagingProjection(
            getDestRect().size(), destRect2));
}

void GPUBlurFilter::applyOnGPU(GLTexturePtr pSrcTex)
{
    int kernelWidth = m_pGaussCurveTex->getSize().x;
    getFBO(1)->activate();
    OGLShaderPtr pHShader = getShader(SHADERID_HORIZ);
    pHShader->activate();
    pHShader->setUniformFloatParam("width", float(kernelWidth));
    pHShader->setUniformIntParam("radius", (kernelWidth-1)/2);
    pHShader->setUniformIntParam("texture", 0);
    pHShader->setUniformIntParam("kernelTex", 1);
    m_pGaussCurveTex->activate(GL_TEXTURE1);
    draw(pSrcTex);

    m_pProjection2->activate();
    getFBO(0)->activate();
    OGLShaderPtr pVShader = getShader(SHADERID_VERT);
    pVShader->activate();
    pVShader->setUniformFloatParam("width", float(kernelWidth));
    pVShader->setUniformIntParam("radius", (kernelWidth-1)/2);
    pVShader->setUniformIntParam("texture", 0);
    pVShader->setUniformIntParam("kernelTex", 1);
    getDestTex(1)->activate(GL_TEXTURE0);
    m_pProjection2->draw();
    glproc::UseProgramObject(0);
}

void GPUBlurFilter::setDimensions(IntPoint size, float stdDev, bool bClipBorders)
{
    
    if (bClipBorders) {
        GPUFilter::setDimensions(size);
    } else {
        int radius = getBlurKernelRadius(stdDev);
        IntPoint offset(radius, radius);
        GPUFilter::setDimensions(size, IntRect(-offset, size+offset), GL_CLAMP_TO_BORDER);
    }
}

}
