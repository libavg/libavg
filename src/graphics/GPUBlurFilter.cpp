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

OGLShaderPtr GPUBlurFilter::s_pShader;

GPUBlurFilter::GPUBlurFilter(const IntPoint& size, PixelFormat pf, 
        double radius)
    : m_Size(size),
      m_PF(pf),
      m_Radius(radius),
      m_pSrcPBO(new PBOImage(size, pf)),
      m_pDestFBO(new FBOImage(size, pf))
{
    ObjectCounter::get()->incRef(&typeid(*this));

    if (!s_pShader) {
        initShader();
    }
    calcKernel();
}

GPUBlurFilter::~GPUBlurFilter()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

BitmapPtr GPUBlurFilter::apply(BitmapPtr pBmpSource)
{
    m_pSrcPBO->setImage(pBmpSource);
    m_pDestFBO->activate();
    GLhandleARB hProgram = s_pShader->getProgram();
    glproc::UseProgramObject(hProgram);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "GPUBlurFilter::apply: glUseProgramObject()");
    s_pShader->setUniformIntParam("radius", (m_KernelWidth-1)/2);
    s_pShader->setUniformFloatArrayParam("kernel", m_KernelWidth, m_Kernel);
    s_pShader->setUniformIntParam("Texture", 0);
    m_pSrcPBO->draw();

    glproc::UseProgramObject(0);
    m_pDestFBO->deactivate();
    return m_pDestFBO->getImage();
}

void GPUBlurFilter::initShader()
{
    string sProgram =
        "#extension GL_ARB_texture_rectangle : enable\n" 
        
        "uniform sampler2DRect Texture;\n"
        "uniform int radius;\n"
        "uniform float kernel[255];\n"

        "void main(void)\n"
        "{\n"
        "    vec4 sum = vec4(0,0,0,0);\n"
        "    for (int i=-radius; i<=radius; ++i) {\n"
        "        vec4 tex = texture2DRect(Texture, gl_TexCoord[0].st+vec2(i,0));\n"
        "        sum += tex*kernel[i+radius];\n"
        "    }\n"
        "    gl_FragColor = sum;\n"
        "}\n"
        ;

    s_pShader = OGLShaderPtr(new OGLShader(sProgram));
}

void GPUBlurFilter::dumpKernel()
{
    cerr << "Gauss, radius " << m_Radius << endl;
    cerr << "  Kernel width: " << m_KernelWidth << endl;
    for (int i=0; i<m_KernelWidth; ++i) {
        cerr << "  " << m_Kernel[i] << endl;
    }
}

void GPUBlurFilter::calcKernel()
{
    int IntRadius = int(ceil(m_Radius));
    m_KernelWidth = IntRadius*2+1;
    for (int i=0; i<= IntRadius; ++i) {
        m_Kernel[IntRadius+i] = exp(-i*i/m_Radius-1)/sqrt(2*M_PI);
        m_Kernel[IntRadius-i] = m_Kernel[IntRadius+i];
    }
}

} // namespace
