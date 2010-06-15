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

#include "GPUShadowFilter.h"
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

GPUShadowFilter::GPUShadowFilter(const IntPoint& size, const DPoint& offset, 
        double stdDev, double opacity, const Pixel32& color)
    : GPUFilter(size, B8G8R8A8, B8G8R8A8, false, 2),
      m_Offset(offset),
      m_StdDev(stdDev),
      m_Opacity(opacity),
      m_Color(color)
{
    ObjectCounter::get()->incRef(&typeid(*this));

    initShaders();
    m_pGaussCurveTex = calcBlurKernelTex(m_StdDev, m_Opacity);
}

GPUShadowFilter::~GPUShadowFilter()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void GPUShadowFilter::setParams(const DPoint& offset, double stdDev, double opacity, 
        const Pixel32& color)
{
    m_Offset = offset;
    m_StdDev = stdDev;
    m_Opacity = opacity;
    m_Color = color;
    m_pGaussCurveTex = calcBlurKernelTex(m_StdDev, m_Opacity);
}

void GPUShadowFilter::applyOnGPU(GLTexturePtr pSrcTex)
{
    int kernelWidth = m_pGaussCurveTex->getSize().x;
    glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);
    OGLShaderPtr pHShader = getShader(SHADERID_HORIZ);
    pHShader->activate();
    pHShader->setUniformFloatParam("width", kernelWidth);
    pHShader->setUniformIntParam("radius", (kernelWidth-1)/2);
    pHShader->setUniformIntParam("texture", 0);
    pHShader->setUniformIntParam("kernelTex", 1);
    m_pGaussCurveTex->activate(GL_TEXTURE1);
    draw(pSrcTex);

    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    OGLShaderPtr pVShader = getShader(SHADERID_VERT);
    pVShader->activate();
    pVShader->setUniformFloatParam("width", kernelWidth);
    pVShader->setUniformIntParam("radius", (kernelWidth-1)/2);
    pVShader->setUniformIntParam("hBlurTex", 0);
    pVShader->setUniformIntParam("kernelTex", 1);
    DPoint texOffset = DPoint(m_Offset.x/getSize().x, m_Offset.y/getSize().y);
    pVShader->setUniformDPointParam("offset", texOffset);
    pVShader->setUniformColorParam("color", m_Color);
    pSrcTex->activate(GL_TEXTURE2);
    pVShader->setUniformIntParam("origTex", 2);
    draw(getDestTex(1));
    glproc::UseProgramObject(0);
}

void GPUShadowFilter::initShaders()
{
    string sProgramHead =
        "uniform float width;\n"
        "uniform int radius;\n"
        "uniform sampler2D kernelTex;\n"
        ;

    string sHorizProgram = sProgramHead + 
        "uniform sampler2D texture;\n"
        "void main(void)\n"
        "{\n"
        "    float sum = 0.;\n"
        "    float dx = dFdx(gl_TexCoord[0].x);\n"
        "    for (int i=-radius; i<=radius; ++i) {\n"
        "        float a = texture2D(texture, gl_TexCoord[0].st+vec2(float(i)*dx,0)).a;\n"
        "        float coeff = \n"
        "                texture2D(kernelTex, vec2((float(i+radius)+0.5)/width,0)).r;\n"
        "        sum += a*coeff;\n"
        "    }\n"
        "    gl_FragColor = vec4(sum, sum, sum, sum);\n"
        "}\n"
        ;
    getOrCreateShader(SHADERID_HORIZ, sHorizProgram);

    string sVertProgram = sProgramHead +
        "uniform sampler2D hBlurTex;\n"
        "uniform sampler2D origTex;\n"
        "uniform vec2 offset;\n"
        "uniform float gamma;\n"
        "uniform vec4 color;\n"
        "void main(void)\n"
        "{\n"
        "    float sum = 0.;\n"
        "    float dy = dFdy(gl_TexCoord[0].y);\n"
        "    for (int i=-radius; i<=radius; ++i) {\n"
        "        float a = texture2D(hBlurTex,\n"
        "                gl_TexCoord[0].st+vec2(0,float(i)*dy)-offset).a;\n"
        "        float coeff = \n"
        "                texture2D(kernelTex, vec2((float(i+radius)+0.5)/width,0)).r;\n"
        "        sum += a*coeff;\n"
        "    }\n"
        "    vec4 origCol = texture2D(origTex, gl_TexCoord[0].st);\n"
        "    gl_FragColor = origCol+(1.-origCol.a)*color*sum;\n"
        "}\n"
        ;
    getOrCreateShader(SHADERID_VERT, sVertProgram);
}


}
