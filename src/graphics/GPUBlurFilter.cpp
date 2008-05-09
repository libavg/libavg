//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
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
#include "../base/Exception.h"

#include <iostream>

using namespace std;

namespace avg {

OGLShaderPtr GPUBlurFilter::s_pHorizShader;
OGLShaderPtr GPUBlurFilter::s_pVertShader;

GPUBlurFilter::GPUBlurFilter(const IntPoint& size, PixelFormat pf, 
        double stdDev)
    : m_Size(size),
      m_PF(pf),
      m_StdDev(stdDev),
      m_pSrcPBO(new PBOImage(size, pf)),
      m_pInterFBO(new FBOImage(size, pf)),
      m_pDestFBO(new FBOImage(size, pf))
{
    ObjectCounter::get()->incRef(&typeid(*this));

    if (!s_pHorizShader) {
        initShaders();
    }
    calcKernel();
//    dumpKernel();
}

GPUBlurFilter::~GPUBlurFilter()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

BitmapPtr GPUBlurFilter::apply(BitmapPtr pBmpSource)
{
    m_pSrcPBO->setImage(pBmpSource);
    m_pInterFBO->activate();
    s_pHorizShader->activate();
    s_pHorizShader->setUniformIntParam("radius", (m_KernelWidth-1)/2);
    s_pHorizShader->setUniformFloatArrayParam("kernel", m_KernelWidth, m_Kernel);
    s_pHorizShader->setUniformIntParam("Texture", 0);
    m_pSrcPBO->draw();

    m_pDestFBO->activate();
    s_pVertShader->activate();
    s_pVertShader->setUniformIntParam("radius", (m_KernelWidth-1)/2);
    s_pVertShader->setUniformFloatArrayParam("kernel", m_KernelWidth, m_Kernel);
    s_pVertShader->setUniformIntParam("Texture", 0);
    m_pInterFBO->draw();
    m_pDestFBO->deactivate();

    return m_pDestFBO->getImage();
}

void GPUBlurFilter::initShaders()
{
    string sProgramHead =
        "#extension GL_ARB_texture_rectangle : enable\n" 
        
        "uniform sampler2DRect Texture;\n"
        "uniform int radius;\n"
        "uniform float kernel[255];\n"
        ;

    string sHorizProgram = sProgramHead + 
        "void main(void)\n"
        "{\n"
        "    vec4 sum = vec4(0,0,0,0);\n"
        "    for (int i=-radius; i<=radius; ++i) {\n"
        "        vec4 tex = texture2DRect(Texture, gl_TexCoord[0].st+vec2(i,0));\n"
        "        sum += tex*kernel[i+radius];\n"
        "    }\n"
        "    gl_FragColor = floor(sum*255.0+0.5)/255.0;\n"
        "}\n"
        ;

    s_pHorizShader = OGLShaderPtr(new OGLShader(sHorizProgram));

    string sVertProgram = sProgramHead + 
        "void main(void)\n"
        "{\n"
        "    vec4 sum = vec4(0,0,0,0);\n"
        "    for (int i=-radius; i<=radius; ++i) {\n"
        "        vec4 tex = texture2DRect(Texture, gl_TexCoord[0].st+vec2(0,i));\n"
        "        sum += tex*kernel[i+radius];\n"
        "    }\n"
        "    gl_FragColor = floor(sum*255.0+0.5)/255.0;\n"
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
    int KernelCenter = ceil(m_StdDev*3); 
    m_KernelWidth = KernelCenter*2+1;
    float sum = 0;
    for (int i=0; i<= KernelCenter; ++i) {
        m_Kernel[KernelCenter+i] = exp(-i*i/(2*m_StdDev*m_StdDev))
                /sqrt(2*M_PI*m_StdDev*m_StdDev);
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
