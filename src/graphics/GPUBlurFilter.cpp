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
#include "../base/Exception.h"

#include <string.h>
#include <iostream>

using namespace std;

namespace avg {

OGLShaderPtr GPUBlurFilter::s_pHorizShader;
OGLShaderPtr GPUBlurFilter::s_pVertShader;

GPUBlurFilter::GPUBlurFilter(const IntPoint& size, PixelFormat pfSrc, double stdDev, 
            bool bStandalone)
    : GPUFilter(size, pfSrc, R32G32B32A32F, bStandalone, 2),
      m_StdDev(stdDev)
{
    ObjectCounter::get()->incRef(&typeid(*this));

    init();
}

void GPUBlurFilter::init()
{
    IntPoint size = getSize();
    m_pGaussCurveTex = GLTexturePtr(new GLTexture(IntPoint(255, 1), I32F));
    if (!s_pHorizShader) {
        initShaders();
    }
    calcKernel();
}

GPUBlurFilter::~GPUBlurFilter()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void GPUBlurFilter::applyOnGPU(GLTexturePtr pSrcTex)
{
    glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);
    s_pHorizShader->activate();
    s_pHorizShader->setUniformIntParam("radius", (m_KernelWidth-1)/2);
    s_pHorizShader->setUniformIntParam("Texture", 0);
    s_pHorizShader->setUniformIntParam("kernelTex", 1);
    m_pGaussCurveTex->activate(GL_TEXTURE1);
    draw(pSrcTex);

    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    s_pVertShader->activate();
    s_pVertShader->setUniformIntParam("radius", (m_KernelWidth-1)/2);
    s_pVertShader->setUniformIntParam("Texture", 0);
    s_pVertShader->setUniformIntParam("kernelTex", 1);
    draw(getDestTex(1));
}

void GPUBlurFilter::initShaders()
{
    string sProgramHead =
        "uniform sampler2D Texture;\n"
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
        "        float coeff = texture2D(kernelTex, vec2((float(i+radius)+0.5)/255.,0)).r;\n"
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
        "        float dy = dFdy(gl_TexCoord[0].y);\n"
        "        vec4 tex = texture2D(Texture, gl_TexCoord[0].st+vec2(0,float(i)*dy));\n"
        "        float coeff = texture2D(kernelTex, vec2((float(i+radius)+0.5)/255.,0)).r;\n"
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
    AVG_ASSERT (m_KernelWidth < 256);
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

    IntPoint size = m_pGaussCurveTex->getSize();
    PBO pbo(size, I32F, GL_STREAM_DRAW);
    pbo.activate();
    void * pPBOPixels = glproc::MapBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, GL_WRITE_ONLY);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "GPUBlurFilter::setImage MapBuffer()");
    int memNeeded = size.x*size.y*Bitmap::getBytesPerPixel(I32F);
    memcpy(pPBOPixels, m_Kernel, memNeeded);
    glproc::UnmapBuffer(GL_PIXEL_UNPACK_BUFFER_EXT);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "GPUBlurFilter::setImage: UnmapBuffer()");
    
    m_pGaussCurveTex->activate(GL_TEXTURE0);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, size.x);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "GPUBlurFilter::setImage: glPixelStorei(GL_UNPACK_ROW_LENGTH)");
    glTexImage2D(GL_TEXTURE_2D, 0, m_pGaussCurveTex->getGLInternalFormat(), 
            size.x, size.y, 0, GLTexture::getGLFormat(I32F), 
            GLTexture::getGLType(I32F), 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "GPUBlurFilter::setImage: glTexImage2D()");
    pbo.deactivate();
}

} // namespace
