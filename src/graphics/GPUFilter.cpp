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

#include <iostream>

using namespace std;

namespace avg {

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
        "  color.rgb /= color.a;\n"
        "}\n"
        "\n"
        "void preMultiplyAlpha(inout vec4 color)\n"
        "{\n"
        "  color.rgb *= color.a;\n"
        "}\n"
        "\n";

    return sCode;
}

} // namespace
