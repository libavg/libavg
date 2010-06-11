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

#include "GPUBlurFilter.h"
#include "Bitmap.h"
#include "ShaderRegistry.h"

#include "../base/ObjectCounter.h"
#include "../base/MathHelper.h"
#include "../base/Exception.h"

#include <string.h>
#include <iostream>

#define SHADERID_HORIZ "HORIZBLUR"
#define SHADERID_VERT "VERTBLUR"

using namespace std;

namespace avg {

GPUBlurFilter::GPUBlurFilter(const IntPoint& size, PixelFormat pfSrc, PixelFormat pfDest,
        double stdDev, bool bStandalone)
    : GPUFilter(size, pfSrc, pfDest, bStandalone, 2),
      m_StdDev(stdDev)
{
    ObjectCounter::get()->incRef(&typeid(*this));

    initShaders();
    m_pGaussCurveTex = calcBlurKernelTex(m_StdDev);
}

GPUBlurFilter::~GPUBlurFilter()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void GPUBlurFilter::setParam(double stdDev)
{
    m_StdDev = stdDev;
    m_pGaussCurveTex = calcBlurKernelTex(m_StdDev);
}

void GPUBlurFilter::applyOnGPU(GLTexturePtr pSrcTex)
{
    int kernelWidth = m_pGaussCurveTex->getSize().x;
    glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);
    OGLShaderPtr pHShader = getShader(SHADERID_HORIZ);
    pHShader->activate();
    pHShader->setUniformFloatParam("width", kernelWidth);
    pHShader->setUniformIntParam("radius", (kernelWidth-1)/2);
    pHShader->setUniformIntParam("Texture", 0);
    pHShader->setUniformIntParam("kernelTex", 1);
    m_pGaussCurveTex->activate(GL_TEXTURE1);
    draw(pSrcTex);

    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    OGLShaderPtr pVShader = getShader(SHADERID_VERT);
    pVShader->activate();
    pVShader->setUniformFloatParam("width", kernelWidth);
    pVShader->setUniformIntParam("radius", (kernelWidth-1)/2);
    pVShader->setUniformIntParam("Texture", 0);
    pVShader->setUniformIntParam("kernelTex", 1);
    draw(getDestTex(1));
    glproc::UseProgramObject(0);
}

void GPUBlurFilter::initShaders()
{
    string sProgramHead =
        "uniform sampler2D Texture;\n"
        "uniform float width;\n"
        "uniform int radius;\n"
        "uniform sampler2D kernelTex;\n"
        ;

    string sHorizProgram = sProgramHead + 
        "void main(void)\n"
        "{\n"
        "    vec4 sum = vec4(0,0,0,0);\n"
        "    for (int i=-radius; i<=radius; ++i) {\n"
        "        float dx = dFdx(gl_TexCoord[0].x);\n"
        "        vec4 tex = texture2D(Texture, gl_TexCoord[0].st+vec2(float(i)*dx,0));\n"
        "        float coeff = \n"
        "                texture2D(kernelTex, vec2((float(i+radius)+0.5)/width,0)).r;\n"
        "        sum += tex*coeff;\n"
        "    }\n"
        "    gl_FragColor = sum;\n"
        "}\n"
        ;
    getOrCreateShader(SHADERID_HORIZ, sHorizProgram);

    string sVertProgram = sProgramHead + 
        "void main(void)\n"
        "{\n"
        "    vec4 sum = vec4(0,0,0,0);\n"
        "    for (int i=-radius; i<=radius; ++i) {\n"
        "        float dy = dFdy(gl_TexCoord[0].y);\n"
        "        vec4 tex = texture2D(Texture, gl_TexCoord[0].st+vec2(0,float(i)*dy));\n"
        "        float coeff = \n"
        "                texture2D(kernelTex, vec2((float(i+radius)+0.5)/width,0)).r;\n"
        "        sum += tex*coeff;\n"
        "    }\n"
        "    gl_FragColor = sum;\n"
        "}\n"
        ;
    getOrCreateShader(SHADERID_VERT, sVertProgram);
}


} // namespace
