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
        bool bOwnFBO)
    : m_pSrcPBO(new PBOImage(size, pfSrc, pfSrc, true, false)),
      m_pDestPBO(new PBOImage(size, pfDest, pfDest, false, true))
{
    ObjectCounter::get()->incRef(&typeid(*this));
    if (bOwnFBO) {
        m_pFBO = FBOPtr(new FBO(size, pfDest, m_pDestPBO->getTexID()));
    }

    initVertexArray();
}
  
GPUFilter::GPUFilter(PBOImagePtr pSrcPBO, PBOImagePtr pDestPBO, bool bOwnFBO)
    : m_pSrcPBO(pSrcPBO),
      m_pDestPBO(pDestPBO)
{
    ObjectCounter::get()->incRef(&typeid(*this));
    if (bOwnFBO) {
        m_pFBO = FBOPtr(new FBO(m_pSrcPBO->getSize(), m_pDestPBO->getExtPF(), 
                m_pDestPBO->getTexID()));
    }
    initVertexArray();
}

GPUFilter::~GPUFilter()
{
    delete m_pVertexes;
    ObjectCounter::get()->decRef(&typeid(*this));
}

BitmapPtr GPUFilter::apply(BitmapPtr pBmpSource)
{
    AVG_ASSERT(m_pFBO);
    m_pSrcPBO->setImage(pBmpSource);
    apply();
    BitmapPtr pFilteredBmp = m_pDestPBO->getImage();
    BitmapPtr pDestBmp(new Bitmap(getSize(), pBmpSource->getPixelFormat()));
    if (pFilteredBmp->getPixelFormat() != pBmpSource->getPixelFormat()) {
        pDestBmp->copyPixels(*pFilteredBmp);
    } else {
        pDestBmp = pFilteredBmp;
    }
    return pDestBmp;
}

void GPUFilter::apply()
{
    AVG_ASSERT(m_pFBO);
    glViewport(0, 0, getSize().x, getSize().y);
    m_pFBO->activate();
    applyOnGPU();
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
    
void GPUFilter::draw(unsigned texID)
{
    glproc::ActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOImage::draw: glBindTexture()");
    m_pVertexes->draw();
}

void GPUFilter::setFBO(FBOPtr pFBO)
{
    AVG_ASSERT(!m_pFBO);
    m_pFBO = pFBO;
}

PBOImagePtr GPUFilter::getSrcPBO()
{
    return m_pSrcPBO;
}

PBOImagePtr GPUFilter::getDestPBO()
{
    return m_pDestPBO;
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
