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

#include "GPUFilter.h"
#include "Bitmap.h"

#include "VertexArray.h"
#include "../base/ObjectCounter.h"
#include "../base/Exception.h"
#include "../base/MathHelper.h"

#include <iostream>
#include <string.h>

using namespace std;
using namespace boost;

namespace avg {

thread_specific_ptr<PBOPtr> GPUFilter::s_pFilterKernelPBO;

GPUFilter::GPUFilter(const IntPoint& size, PixelFormat pfSrc, PixelFormat pfDest,
        bool bStandalone, unsigned numTextures)
    : m_pFBO(new FBO(size, pfDest, numTextures))
{
    if (bStandalone) {
        m_pSrcTex = GLTexturePtr(new GLTexture(size, pfSrc));
        m_pSrcPBO = PBOPtr(new PBO(size, pfSrc, GL_STREAM_DRAW));
    }
    ObjectCounter::get()->incRef(&typeid(*this));
}
  
GPUFilter::~GPUFilter()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

BitmapPtr GPUFilter::apply(BitmapPtr pBmpSource)
{
    AVG_ASSERT(m_pSrcTex);
    m_pSrcPBO->moveBmpToTexture(pBmpSource, m_pSrcTex);
    apply(m_pSrcTex);
    BitmapPtr pFilteredBmp = m_pFBO->getImage();
    BitmapPtr pDestBmp;
    if (pFilteredBmp->getPixelFormat() != pBmpSource->getPixelFormat()) {
        pDestBmp = BitmapPtr(new Bitmap(getSize(), pBmpSource->getPixelFormat()));
        pDestBmp->copyPixels(*pFilteredBmp);
    } else {
        pDestBmp = pFilteredBmp;
    }
    return pDestBmp;
}

void GPUFilter::apply(GLTexturePtr pSrcTex)
{
    m_pFBO->activate();
    m_pFBO->setupImagingProjection();
    applyOnGPU(pSrcTex);
    m_pFBO->deactivate();
    m_pFBO->copyToDestTexture();
}

GLTexturePtr GPUFilter::getDestTex(int i) const
{
    return m_pFBO->getTex(i);
}

BitmapPtr GPUFilter::getImage() const
{
    return m_pFBO->getImage();
}

FBOPtr GPUFilter::getFBO()
{
    return m_pFBO;
}

void GPUFilter::glContextGone()
{
    s_pFilterKernelPBO.reset();
}

void GPUFilter::draw(GLTexturePtr pTex)
{
    pTex->activate(GL_TEXTURE0);
    m_pFBO->drawImagingVertexes();
}

const IntPoint& GPUFilter::getSize() const
{
    return m_pFBO->getSize();
}

const string& GPUFilter::getStdShaderCode() const
{
    static string sCode = 
        "void unPreMultiplyAlpha(inout vec4 color)\n"
        "{\n"
        "    color.rgb /= color.a;\n"
        "}\n"
        "\n"
        "void preMultiplyAlpha(inout vec4 color)\n"
        "{\n"
        "    color.rgb *= color.a;\n"
        "}\n"
        "\n";

    return sCode;
}

void dumpKernel(int width, float* pKernel)
{
    cerr << "  Kernel width: " << width << endl;
    float sum = 0;
    for (int i=0; i<width; ++i) {
        sum += pKernel[i];
        cerr << "  " << pKernel[i] << endl;
    }
    cerr << "Sum of coefficients: " << sum << endl;
}

GLTexturePtr GPUFilter::calcBlurKernelTex(double stdDev, double opacity) const
// If opacity is -1, this is a brightness-preserving blur.
// Otherwise, opacity is the coefficient of the center pixel.
{
    int kernelCenter = int(ceil(stdDev*3));
    int kernelWidth = kernelCenter*2+1;
    float* pKernel;
    pKernel = new float[kernelWidth];
    float sum = 0;
    for (int i=0; i <= kernelCenter; ++i) {
        pKernel[kernelCenter+i] = float(exp(-i*i/(2*stdDev*stdDev))
                /sqrt(2*PI*stdDev*stdDev));
        sum += pKernel[kernelCenter+i];
        if (i != 0) {
            pKernel[kernelCenter-i] = pKernel[kernelCenter+i];
            sum += pKernel[kernelCenter-i];
        }
    }

    if (opacity == -1) {
        // This is a brightness-preserving blur.
        // Make sure the sum of coefficients is 1 despite the inaccuracies
        // introduced by using a kernel of finite size.
        for (int i=0; i<kernelWidth; ++i) {
            pKernel[i] /= sum;
        }
    } else {
        double factor = opacity/pKernel[kernelCenter];
        for (int i=0; i<kernelWidth; ++i) {
            pKernel[i] *= factor;
        }
    }
//    dumpKernel(kernelWidth, pKernel);
    
    IntPoint size(kernelWidth, 1);
    GLTexturePtr pTex(new GLTexture(size, R32G32B32A32F));
    if (s_pFilterKernelPBO.get() == 0) {
        s_pFilterKernelPBO.reset(new PBOPtr(new PBO(IntPoint(1024, 1), R32G32B32A32F,
                GL_STREAM_DRAW)));
    }
    (*s_pFilterKernelPBO)->activate();
    void * pPBOPixels = glproc::MapBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, GL_WRITE_ONLY);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "GPUFilter::calcBlurKernelTex MapBuffer()");
    float * pCurFloat = (float*)pPBOPixels;
    for (int i=0; i<kernelWidth; ++i) {
        for (int j=0; j<4; ++j) {
            *pCurFloat = pKernel[i];
            ++pCurFloat;
        }
    }
    glproc::UnmapBuffer(GL_PIXEL_UNPACK_BUFFER_EXT);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "GPUFilter::calcBlurKernelTex UnmapBuffer()");
   
    (*s_pFilterKernelPBO)->movePBOToTexture(pTex);

    delete[] pKernel;
    return pTex;
}

}
