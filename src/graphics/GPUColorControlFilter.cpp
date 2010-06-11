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

#include "GPUColorControlFilter.h"
#include "Bitmap.h"
#include "ShaderRegistry.h"

#include "../base/ObjectCounter.h"
#include "../base/Exception.h"

#include <iostream>

#define SHADERID "BRIGHTNESS"

using namespace std;

namespace avg {

GPUColorControlFilter::GPUColorControlFilter(const IntPoint& size, bool bStandalone)
    : GPUFilter(size, B8G8R8A8, B8G8R8A8, bStandalone),
      m_Brightness(1),
      m_Contrast(1),
      m_RGamma(1),
      m_GGamma(1),
      m_BGamma(1)

{
    ObjectCounter::get()->incRef(&typeid(*this));

    initShader();
}

GPUColorControlFilter::~GPUColorControlFilter()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void GPUColorControlFilter::setParams(float brightness, float contrast, float rGamma, 
        float gGamma, float bGamma)
{
    m_Brightness = brightness;
    m_Contrast = contrast;
    m_RGamma = rGamma;
    m_GGamma = gGamma;
    m_BGamma = bGamma;
}

void GPUColorControlFilter::applyOnGPU(GLTexturePtr pSrcTex)
{
    OGLShaderPtr pShader = getShader(SHADERID);
    pShader->activate();
    pShader->setUniformIntParam("Texture", 0);
    pShader->setUniformIntParam("texture", 0);
    pShader->setUniformFloatParam("brightness", m_Brightness);
    pShader->setUniformFloatParam("contrast", m_Contrast);
    pShader->setUniformFloatParam("rGamma", 1.f/m_RGamma);
    pShader->setUniformFloatParam("gGamma", 1.f/m_GGamma);
    pShader->setUniformFloatParam("bGamma", 1.f/m_BGamma);
    
    draw(pSrcTex);

    glproc::UseProgramObject(0);
}

void GPUColorControlFilter::initShader()
{
    string sProgram =
        "uniform sampler2D texture;\n"
        "uniform float brightness;\n"
        "uniform float contrast;\n"
        "uniform float rGamma;\n"
        "uniform float gGamma;\n"
        "uniform float bGamma;\n"
        "\n"
        +getStdShaderCode()+
        "void main(void)\n"
        "{\n"
        "  vec4 tex = texture2D(texture, gl_TexCoord[0].st);\n"
        "  unPreMultiplyAlpha(tex);\n"
        "  vec3 avg = vec3(0.5, 0.5, 0.5);\n"
        "  tex.rgb = mix(avg, tex.rgb, contrast);\n"
        "  tex.rgb = tex.rgb*brightness;\n"
        "  tex.rgb = vec3(pow(tex.r, rGamma), pow(tex.g, gGamma),\n"
        "          pow(tex.b, bGamma));\n"
        "  preMultiplyAlpha(tex);\n"
        "  gl_FragColor = tex;\n"
        "}\n"
        ;

    getOrCreateShader(SHADERID, sProgram);
}

} // namespace
