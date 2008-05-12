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

#include "GPUFilter.h"
#include "Bitmap.h"

#include "../base/ObjectCounter.h"

#include <iostream>

using namespace std;

namespace avg {

GPUFilter::GPUFilter(const IntPoint& size, PixelFormat pf)
    : m_pSrcPBO(new PBOImage(size, pf, GL_UNSIGNED_BYTE, true, false)),
      m_pDestFBO(new FBOImage(size, B8G8R8A8, GL_UNSIGNED_BYTE, false, true))
{
    ObjectCounter::get()->incRef(&typeid(*this));
}
    
GPUFilter::GPUFilter(PBOImagePtr pSrcPBO, FBOImagePtr pDestFBO)
    : m_pSrcPBO(pSrcPBO),
      m_pDestFBO(pDestFBO)
{
    ObjectCounter::get()->incRef(&typeid(*this));
}

GPUFilter::~GPUFilter()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

BitmapPtr GPUFilter::apply(BitmapPtr pBmpSource)
{
    m_pSrcPBO->setImage(pBmpSource);
    glViewport(0, 0, getSize().x, getSize().y);
    applyOnGPU();
    BitmapPtr pFilteredBmp = m_pDestFBO->getImage();
    BitmapPtr pDestBmp(new Bitmap(getSize(), pBmpSource->getPixelFormat()));
    if (pFilteredBmp->getPixelFormat() != pBmpSource->getPixelFormat()) {
        pDestBmp->copyPixels(*pFilteredBmp);
    } else {
        pDestBmp = pFilteredBmp;
    }
    return pDestBmp;
}

PixelFormat GPUFilter::getPF() const
{
    return m_pSrcPBO->getPF();
}

const IntPoint& GPUFilter::getSize() const
{
    return m_pSrcPBO->getSize();
}

PBOImagePtr GPUFilter::getSrcPBO()
{
    return m_pSrcPBO;
}

FBOImagePtr GPUFilter::getDestFBO()
{
    return m_pDestFBO;
}

} // namespace
