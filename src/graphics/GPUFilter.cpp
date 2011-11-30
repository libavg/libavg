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

#include "GPUFilter.h"
#include "Bitmap.h"

#include "VertexArray.h"
#include "ImagingProjection.h"
#include "../base/ObjectCounter.h"
#include "../base/Exception.h"
#include "../base/MathHelper.h"

#include <iostream>
#include <string.h>

using namespace std;
using namespace boost;

namespace avg {

GPUFilter::GPUFilter(PixelFormat pfSrc, PixelFormat pfDest, bool bStandalone, 
        unsigned numTextures, bool bMipmap)
    : m_PFSrc(pfSrc),
      m_PFDest(pfDest),
      m_bStandalone(bStandalone),
      m_NumTextures(numTextures),
      m_bMipmap(bMipmap),
      m_SrcSize(0,0),
      m_DestRect(0,0,0,0)
{
    ObjectCounter::get()->incRef(&typeid(*this));
}

GPUFilter::~GPUFilter()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void GPUFilter::setDimensions(const IntPoint& srcSize)
{
    setDimensions(srcSize, IntRect(IntPoint(0,0), srcSize), GL_CLAMP_TO_EDGE);
}

void GPUFilter::setDimensions(const IntPoint& srcSize, const IntRect& destRect,
        unsigned texMode)
{
    bool bProjectionChanged = false;
    if (destRect != m_DestRect) {
        m_pFBO = FBOPtr(new FBO(destRect.size(), m_PFDest, m_NumTextures, 1, false,
                m_bMipmap));
        m_DestRect = destRect;
        bProjectionChanged = true;
    }
    if (m_bStandalone && srcSize != m_SrcSize) {
        m_pSrcTex = GLTexturePtr(new GLTexture(srcSize, m_PFSrc, false, texMode, 
                texMode));
        m_pSrcPBO = PBOPtr(new PBO(srcSize, m_PFSrc, GL_STREAM_DRAW));
        bProjectionChanged = true;
    }
    m_SrcSize = srcSize;
    if (bProjectionChanged) {
        m_pProjection = ImagingProjectionPtr(new ImagingProjection(srcSize, destRect));
    }
}
  
BitmapPtr GPUFilter::apply(BitmapPtr pBmpSource)
{
    AVG_ASSERT(m_pSrcTex);
    AVG_ASSERT(m_pFBO);
    m_pSrcPBO->moveBmpToTexture(pBmpSource, *m_pSrcTex);
    apply(m_pSrcTex);
    BitmapPtr pFilteredBmp = m_pFBO->getImage();
    BitmapPtr pDestBmp;
    if (pFilteredBmp->getPixelFormat() != pBmpSource->getPixelFormat()) {
        pDestBmp = BitmapPtr(new Bitmap(m_DestRect.size(),
                pBmpSource->getPixelFormat()));
        pDestBmp->copyPixels(*pFilteredBmp);
    } else {
        pDestBmp = pFilteredBmp;
    }
    return pDestBmp;
}

void GPUFilter::apply(GLTexturePtr pSrcTex)
{
    m_pFBO->activate();
    m_pProjection->activate();
    applyOnGPU(pSrcTex);
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

const IntRect& GPUFilter::getDestRect() const
{
    return m_DestRect;
}

const IntPoint& GPUFilter::getSrcSize() const
{
    return m_SrcSize;
}

FRect GPUFilter::getRelDestRect() const
{
    glm::vec2 srcSize(m_SrcSize);
    return FRect(m_DestRect.tl.x/srcSize.x, m_DestRect.tl.y/srcSize.y,
            m_DestRect.br.x/srcSize.x, m_DestRect.br.y/srcSize.y);
}

void GPUFilter::draw(GLTexturePtr pTex)
{
    pTex->activate(GL_TEXTURE0);
    m_pProjection->draw();
}

const string& GPUFilter::getStdShaderCode() const
{
    static string sCode = 
        "void unPreMultiplyAlpha(inout vec4 color)\n"
        "{\n"
        "    if (color.a > 0.0) {\n"
        "       color.rgb /= color.a;\n"
        "    }\n"
        "}\n"
        "\n"
        "void preMultiplyAlpha(inout vec4 color)\n"
        "{\n"
        "    color.rgb *= color.a;\n"
        "}\n"
        "\n"
        "void rgb2hsl(vec4 rgba, out float h, out float s, out float l)\n"
        "{\n"
        "    float maxComp = max(rgba.r, max(rgba.g, rgba.b));\n"
        "    float minComp = min(rgba.r, min(rgba.g, rgba.b));\n"
        "    l = (maxComp+minComp)/2.0;\n"
        "    if (maxComp == minComp) {\n"
        "        s = 0.0;\n"
        "        h = 0.0;\n"
        "    } else {\n"
        "        float delta = maxComp-minComp;\n"
        "        if (l < 0.5) {\n"
        "            s = delta/(maxComp+minComp);\n"
        "        } else {\n"
        "            s = delta/(2.0-(maxComp+minComp));\n"
        "        }\n"
        "        if (rgba.r == maxComp) {\n"
        "            h = (rgba.g-rgba.b)/delta;\n"
        "            if (h < 0.0) {\n"
        "                h += 6.0;\n"
        "            }\n"
        "        } else if (rgba.g == maxComp) {\n"
        "            h = 2.0+(rgba.b-rgba.r)/delta;\n"
        "        } else {\n"
        "            h = 4.0+(rgba.r-rgba.g)/delta;\n"
        "        }\n"
        "        h *= 60.0;\n"
        "    }\n"
        "}\n"
        "vec3 hsl2rgb(float h, float s, float l)\n"
        "{\n"
        "    vec3 rgb = vec3(0.0, 0.0, 0.0);\n"
        "    float v;\n"
        "    if (l <= 0.5) {\n"
        "        v = l*(1.0+s);\n"
        "    } else {\n"
        "        v = l+s-l*s;\n"
        "    }\n"
        "    if (v > 0.0) {\n"
        "        float m = 2.0*l-v;\n"
        "        float sv = (v-m)/v;\n"
        "        h /= 60.0;\n"
        "        int sextant = int(h);\n"
        "        float fract = h-float(sextant);\n"
        "        float vsf = v * sv * fract;\n"
        "        float mid1 = m + vsf;\n"
        "        float mid2 = v - vsf;\n"
        "        if (sextant == 0) {\n"
        "            rgb.r = v;\n"
        "            rgb.g = mid1;\n"
        "            rgb.b = m;\n"
        "        } else if (sextant == 1) {\n"
        "            rgb.r = mid2;\n"
        "            rgb.g = v;\n"
        "            rgb.b = m;\n"
        "        } else if (sextant == 2) {\n"
        "            rgb.r = m;\n"
        "            rgb.g = v;\n"
        "            rgb.b = mid1;\n"
        "        } else if (sextant == 3) {\n"
        "            rgb.r = m;\n"
        "            rgb.g = mid2;\n"
        "            rgb.b = v;\n"
        "        } else if (sextant == 4) {\n"
        "            rgb.r = mid1;\n"
        "            rgb.g = m;\n"
        "            rgb.b = v;\n"
        "        } else if (sextant == 5) {\n"
        "            rgb.r = v;\n"
        "            rgb.g = m;\n"
        "            rgb.b = mid2;\n"
        "        }\n"
        "    }\n"
        "    return rgb;\n"
        "}\n";

    return sCode;
}

void dumpKernel(int width, float* pKernel)
{
    cerr << "  Kernel width: " << width << endl;
    float sum = 0;
    for (int i = 0; i < width; ++i) {
        sum += pKernel[i];
        cerr << "  " << pKernel[i] << endl;
    }
    cerr << "Sum of coefficients: " << sum << endl;
}

int GPUFilter::getBlurKernelRadius(float stdDev) const
{
    return int(ceil(stdDev*3));
}

GLTexturePtr GPUFilter::calcBlurKernelTex(float stdDev, float opacity) const
{
    AVG_ASSERT(opacity != -1);
    int kernelWidth;
    float* pKernel;
    if (stdDev == 0) {
        kernelWidth = 1;
        pKernel = new float[1];
        pKernel[0] = float(opacity);
    } else {
        float tempCoeffs[1024];
        int i=0;
        float coeff;
        do {
            coeff = float(exp(-i*i/(2*stdDev*stdDev))/sqrt(2*PI*stdDev*stdDev))
                    *float(opacity);
            tempCoeffs[i] = coeff;
            i++;
        } while (coeff > 0.005 && i < 1024);
        int kernelCenter = i - 2;
        kernelWidth = kernelCenter*2+1;
        pKernel = new float[kernelWidth];
        float sum = 0;
        for (int i = 0; i <= kernelCenter; ++i) {
            pKernel[kernelCenter+i] = tempCoeffs[i];
            sum += tempCoeffs[i];
            if (i != 0) {
                pKernel[kernelCenter-i] = tempCoeffs[i];
                sum += tempCoeffs[i];
            }
        }
        // Make sure the sum of coefficients is opacity despite the inaccuracies
        // introduced by using a kernel of finite size.
        for (int i = 0; i < kernelWidth; ++i) {
            pKernel[i] *= float(opacity)/sum;
        }
    }
//    dumpKernel(kernelWidth, pKernel);
    
    IntPoint size(kernelWidth, 1);
    GLTexturePtr pTex(new GLTexture(size, R32G32B32A32F));
    PBOPtr pFilterKernelPBO(new PBO(IntPoint(1024, 1), R32G32B32A32F, GL_STREAM_DRAW));
    pFilterKernelPBO->activate();
    void * pPBOPixels = glproc::MapBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, GL_WRITE_ONLY);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "GPUFilter::calcBlurKernelTex MapBuffer()");
    float * pCurFloat = (float*)pPBOPixels;
    for (int i = 0; i < kernelWidth; ++i) {
        for (int j = 0; j < 4; ++j) {
            *pCurFloat = pKernel[i];
            ++pCurFloat;
        }
    }
    glproc::UnmapBuffer(GL_PIXEL_UNPACK_BUFFER_EXT);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "GPUFilter::calcBlurKernelTex UnmapBuffer()");
   
    pFilterKernelPBO->moveToTexture(*pTex);

    delete[] pKernel;
    return pTex;
}

}
