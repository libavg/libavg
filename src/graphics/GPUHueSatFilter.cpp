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

#include "GPUHueSatFilter.h"
#include "ShaderRegistry.h"
#include "OGLShader.h"

#include "../base/ObjectCounter.h"
#include "../base/Logger.h"

#define SHADERID_HSL_COLOR "huesat"

using namespace std;

namespace avg {

GPUHueSatFilter::GPUHueSatFilter(const IntPoint& size, bool bUseAlpha, bool bStandalone)
    : GPUFilter(SHADERID_HSL_COLOR, bUseAlpha, bStandalone),
      m_LightnessOffset(0.0),
      m_Hue(0.0),
      m_Saturation(0.0)
{
    ObjectCounter::get()->incRef(&typeid(*this));
    setDimensions(size);
    OGLShaderPtr pShader = getShader();
    m_pHueParam = pShader->getParam<float>("u_Hue");
    m_pSatParam = pShader->getParam<float>("u_Sat");
    m_pLightnessParam = pShader->getParam<float>("u_LightnessOffset");
    m_pColorizeParam = pShader->getParam<int>("u_bColorize");
    m_pTextureParam = pShader->getParam<int>("u_Texture");
}

GPUHueSatFilter::~GPUHueSatFilter()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void GPUHueSatFilter::setParams(int hue, int saturation,
        int light_add, bool colorize)
{
    m_Hue = float(hue);
    m_Saturation = saturation / 100.0f;
    m_LightnessOffset = light_add / 100.0f;
    m_bColorize = colorize;
}

void GPUHueSatFilter::applyOnGPU(GLTexturePtr pSrcTex)
{
    getShader()->activate();
    m_pHueParam->set(m_Hue);
    m_pSatParam->set(m_Saturation);
    m_pLightnessParam->set(m_LightnessOffset);
    m_pColorizeParam->set((int)(m_bColorize));
    m_pTextureParam->set(0);
    draw(pSrcTex);
}

}

