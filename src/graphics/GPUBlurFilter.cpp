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

#include "../base/ObjectCounter.h"
#include "../base/MathHelper.h"

#include <iostream>

using namespace std;

namespace avg {

OGLShaderPtr GPUBlurFilter::s_pHorizShader;
OGLShaderPtr GPUBlurFilter::s_pVertShader;

GPUBlurFilter::GPUBlurFilter(const IntPoint& size, PixelFormat pfSrc, double stdDev)
    : GPUFilter(size, pfSrc),
      m_StdDev(stdDev)
{
    ObjectCounter::get()->incRef(&typeid(*this));

    init();
}

GPUBlurFilter::GPUBlurFilter(PBOImagePtr pSrcPBO, FBOImagePtr pDestFBO, double stdDev)
    : GPUFilter(pSrcPBO, pDestFBO),
      m_StdDev(stdDev)
{
    ObjectCounter::get()->incRef(&typeid(*this));

    init();
}

void GPUBlurFilter::init()
{
    IntPoint size = getSrcPBO()->getSize();
    m_pGaussCurvePBO = PBOImagePtr(new PBOImage(IntPoint(255, 1), I32F, I32F, false, false));
    m_pInterFBO = FBOImagePtr(new FBOImage(size, R32G32B32A32F, B8G8R8A8, false, false));
    if (!s_pHorizShader) {
        initShaders();
    }
    calcKernel();
    m_pGaussCurvePBO->setImage(m_Kernel);
//    dumpKernel();
}

GPUBlurFilter::~GPUBlurFilter()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void GPUBlurFilter::applyOnGPU()
{
    m_pInterFBO->activate();
    s_pHorizShader->activate();
    s_pHorizShader->setUniformIntParam("radius", (m_KernelWidth-1)/2);
    s_pHorizShader->setUniformIntParam("Texture", 0);
    s_pHorizShader->setUniformIntParam("kernelTex", 1);
    m_pGaussCurvePBO->activateTex(GL_TEXTURE1);
    getSrcPBO()->draw();

    getDestFBO()->activate();
    s_pVertShader->activate();
    s_pVertShader->setUniformIntParam("radius", (m_KernelWidth-1)/2);
    s_pVertShader->setUniformIntParam("Texture", 0);
    s_pVertShader->setUniformIntParam("kernelTex", 1);
    m_pInterFBO->draw();
    getDestFBO()->deactivate();
}

void GPUBlurFilter::initShaders()
{
    string sProgramHead =
        "#extension GL_ARB_texture_rectangle : enable\n" 
        
        "uniform sampler2DRect Texture;\n"
        "uniform int radius;\n"
        "uniform sampler2DRect kernelTex;\n"
        ;

    string sHorizProgram = sProgramHead + 
        "void main(void)\n"
        "{\n"
        "    vec4 sum = vec4(0,0,0,0);\n"
        "    for (int i=-radius; i<=radius; ++i) {\n"
        "        vec4 tex = texture2DRect(Texture, gl_TexCoord[0].st+vec2(i,0));\n"
        "        float coeff = texture2DRect(kernelTex, vec2(float(i+radius)+0.5,0)).r;\n"
        "        sum += tex*coeff;\n"
        "    }\n"
        "    gl_FragColor = sum;\n"
        "}\n"
        ;

    s_pHorizShader = OGLShaderPtr(new OGLShader(sHorizProgram));

    string sVertProgram = sProgramHead + 
        "void main(void)\n"
        "{\n"
        "    vec4 sum = vec4(0,0,0,0);\n"
        "    for (int i=-radius; i<=radius; ++i) {\n"
        "        vec4 tex = texture2DRect(Texture, gl_TexCoord[0].st+vec2(0,i));\n"
        "        float coeff = texture2DRect(kernelTex, vec2(float(i+radius)+0.5,0)).r;\n"
        "        sum += tex*coeff;\n"
        "    }\n"
        "    gl_FragColor = sum;\n"
        "}\n"
        ;

    s_pVertShader = OGLShaderPtr(new OGLShader(sVertProgram));
}

void GPUBlurFilter::dumpKernel()
{
    cerr << "Gauss, std dev " << m_StdDev << endl;
    cerr << "  Kernel width: " << m_KernelWidth << endl;
    float sum = 0;
    for (int i=0; i<m_KernelWidth; ++i) {
        sum += m_Kernel[i];
        cerr << "  " << m_Kernel[i] << endl;
    }
    cerr << "Sum of coefficients: " << sum << endl;
}

void GPUBlurFilter::calcKernel()
{
    int KernelCenter = int(ceil(m_StdDev*3));
    m_KernelWidth = KernelCenter*2+1;
    assert (m_KernelWidth < 256);
    float sum = 0;
    for (int i=0; i<= KernelCenter; ++i) {
        m_Kernel[KernelCenter+i] = float(exp(-i*i/(2*m_StdDev*m_StdDev))
                /sqrt(2*PI*m_StdDev*m_StdDev));
        sum += m_Kernel[KernelCenter+i];
        if (i != 0) {
            m_Kernel[KernelCenter-i] = m_Kernel[KernelCenter+i];
            sum += m_Kernel[KernelCenter-i];
        }
    }

    // Make sure the sum of coefficients is 1 despite the inaccuracies
    // introduced by using a kernel of finite size.
    for (int i=0; i<=m_KernelWidth; ++i) {
        m_Kernel[i] /= sum;
    }

}

} // namespace
