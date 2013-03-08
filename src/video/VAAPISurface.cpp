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

#include "VAAPISurface.h"

#include "VAAPIDecoder.h"
#include "VAAPIHelper.h"

#include "../base/Exception.h"

#include <iostream>

using namespace std;
    
namespace avg {

VAAPISurface::VAAPISurface(VASurfaceID surfaceID, VAAPIDecoder* pDecoder)
    : m_SurfaceID(surfaceID),
      m_bUsed(false)
{
    m_Size = pDecoder->getSize();
    m_pImage = pDecoder->getImage();
}

VASurfaceID VAAPISurface::getSurfaceID() const
{
    return m_SurfaceID;
}

void VAAPISurface::setUsed(bool bUsed)
{
    AVG_ASSERT(bUsed != m_bUsed);
    m_bUsed = bUsed;
}
    
bool VAAPISurface::isUsed() const
{
    return m_bUsed;
}

void VAAPISurface::getYUVBmps(BitmapPtr pBmpY, BitmapPtr pBmpU, BitmapPtr pBmpV)
{
    VAStatus status;

    status = vaSyncSurface(getVAAPIDisplay(), m_SurfaceID);
    AVG_ASSERT(status == VA_STATUS_SUCCESS);

    status = vaGetImage(getVAAPIDisplay(), m_SurfaceID, 0, 0, m_Size.x, m_Size.y,
            m_pImage->image_id);
    AVG_ASSERT(status == VA_STATUS_SUCCESS);

    void* pImgBuffer;
    status = vaMapBuffer(getVAAPIDisplay(), m_pImage->buf, &pImgBuffer);
    AVG_ASSERT(status == VA_STATUS_SUCCESS);

    BitmapPtr pSrcBmp(new Bitmap(m_Size, I8, 
            (uint8_t*)pImgBuffer + m_pImage->offsets[0], m_pImage->pitches[0], false));
    pBmpY->copyPixels(*pSrcBmp);
/*    
    pSrcBmp = BitmapPtr(new Bitmap(m_Size/2, I8, 
            (uint8_t*)pImgBuffer + m_pImage->offsets[1], m_pImage->pitches[1], false));
    pBmpU->copyPixels(*pSrcBmp);
    pSrcBmp = BitmapPtr(new Bitmap(m_Size/2, I8, 
            (uint8_t*)pImgBuffer + m_pImage->offsets[2], m_pImage->pitches[2], false));
    pBmpV->copyPixels(*pSrcBmp);
*/
    vaUnmapBuffer(getVAAPIDisplay(), m_pImage->buf);

    setUsed(false);
    
}

void VAAPISurface::getRGBBmp(BitmapPtr pBmp)
{
    AVG_ASSERT(m_Size == pBmp->getSize());
    
    IntPoint UVSize(m_Size.x/2, m_Size.y/2);
    BitmapPtr pBmpY(new Bitmap(m_Size, I8));
    BitmapPtr pBmpU(new Bitmap(UVSize, I8));
    BitmapPtr pBmpV(new Bitmap(UVSize, I8));
    getYUVBmps(pBmpY, pBmpU, pBmpV);
    pBmp->copyYUVPixels(*pBmpY, *pBmpU, *pBmpV, false);
}

}

