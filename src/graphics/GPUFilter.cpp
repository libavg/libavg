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
        unsigned numTextures)
    : m_pSrcTex(new GLTexture(size, pfSrc)),
      m_pSrcPBO(new PBO(size, pfSrc, GL_STREAM_DRAW)),
      m_pFBO(new FBO(size, pfDest, numTextures))
{
    ObjectCounter::get()->incRef(&typeid(*this));
    initVertexArray();
}
  
GPUFilter::~GPUFilter()
{
    delete m_pVertexes;
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
    glViewport(0, 0, getSize().x, getSize().y);
    m_pFBO->activate();
    applyOnGPU(pSrcTex);
    m_pFBO->deactivate();
}

FBOPtr GPUFilter::getFBO()
{
    return m_pFBO;
}

const IntPoint& GPUFilter::getSize() const
{
    return m_pSrcPBO->getSize();
}

void GPUFilter::draw(GLTexturePtr pTex)
{
    pTex->activate(GL_TEXTURE0);
    m_pVertexes->draw();
}

GLTexturePtr GPUFilter::getDestTex(int i) const
{
    return m_pFBO->getTex(i);
}

void GPUFilter::initVertexArray()
{
    IntPoint size = m_pSrcPBO->getSize();
    // Create a minimal vertex array to be used for drawing.
    m_pVertexes = new VertexArray();
    m_pVertexes->appendPos(DPoint(0, 0), DPoint(0, 1));
    m_pVertexes->appendPos(DPoint(0, size.y), DPoint(0, 0));
    m_pVertexes->appendPos(DPoint(size.x, size.y), DPoint(1, 0));
    m_pVertexes->appendPos(DPoint(size.x, 0), DPoint(1, 1));
    m_pVertexes->appendQuadIndexes(1, 0, 2, 3);
}

} // namespace
