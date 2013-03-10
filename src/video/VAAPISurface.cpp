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

#include "../base/Exception.h"
#include "../graphics/GLTexture.h"

#include <va/va_glx.h>
#include <iostream>

using namespace std;
    
namespace avg {

VAAPISurface::VAAPISurface(VASurfaceID surfaceID, VAAPIDecoder* pDecoder)
    : m_SurfaceID(surfaceID),
      m_RefCount(0)
{
    m_Size = pDecoder->getSize();
    m_pImage = pDecoder->getImage();
}

VASurfaceID VAAPISurface::getSurfaceID() const
{
    return m_SurfaceID;
}

void VAAPISurface::incRef()
{
    AVG_ASSERT(m_RefCount < 2);
    m_RefCount++;
}

void VAAPISurface::decRef()
{
    AVG_ASSERT(m_RefCount > 0);
    m_RefCount--;
}
 
bool VAAPISurface::isUsed() const
{
    return m_RefCount > 0;
}

void VAAPISurface::getYUVBmps(BitmapPtr pBmpY, BitmapPtr pBmpU, BitmapPtr pBmpV)
{
    VAStatus status;

    status = vaGetImage(VAAPIDecoder::getDisplay(), m_SurfaceID, 0, 0, m_Size.x, m_Size.y,
            m_pImage->image_id);
    VAAPIDecoder::checkError(status);

    void* pImgBuffer;
    status = vaMapBuffer(VAAPIDecoder::getDisplay(), m_pImage->buf, &pImgBuffer);
    VAAPIDecoder::checkError(status);

    switch (m_pImage->format.fourcc) {
        case VA_FOURCC_YV12:
            {
                BitmapPtr pSrcBmp(new Bitmap(m_Size, I8, 
                        (uint8_t*)pImgBuffer + m_pImage->offsets[0], 
                        m_pImage->pitches[0], false));
                pBmpY->copyPixels(*pSrcBmp);
                pSrcBmp = BitmapPtr(new Bitmap(m_Size/2, I8, 
                            (uint8_t*)pImgBuffer + m_pImage->offsets[2],
                            m_pImage->pitches[2], false));
                pBmpU->copyPixels(*pSrcBmp);
                pSrcBmp = BitmapPtr(new Bitmap(m_Size/2, I8, 
                            (uint8_t*)pImgBuffer + m_pImage->offsets[1], 
                            m_pImage->pitches[1], false));
                pBmpV->copyPixels(*pSrcBmp);
            }
            break;
        case VA_FOURCC_NV12:
            {
                BitmapPtr pSrcBmp(new Bitmap(m_Size, I8, 
                        (uint8_t*)pImgBuffer + m_pImage->offsets[0], 
                        m_pImage->pitches[0], false));
                pBmpY->copyPixels(*pSrcBmp);

                pSrcBmp = BitmapPtr(new Bitmap(IntPoint(m_Size.x, m_Size.y/2), I8, 
                            (uint8_t*)pImgBuffer + m_pImage->offsets[1],
                            m_pImage->pitches[1], false));
                
                splitInterleaved(pBmpU, pBmpV, pSrcBmp);
            }
            break;
        default:
            AVG_ASSERT(false);
    }
    vaUnmapBuffer(VAAPIDecoder::getDisplay(), m_pImage->buf);
}

void VAAPISurface::getRGBBmp(BitmapPtr pBmp)
{
    AVG_ASSERT(m_Size == pBmp->getSize());
    VAStatus status;
    status = vaGetImage(VAAPIDecoder::getDisplay(), m_SurfaceID, 0, 0, m_Size.x, m_Size.y,
            m_pImage->image_id);
    VAAPIDecoder::checkError(status);

    void* pImgBuffer;
    status = vaMapBuffer(VAAPIDecoder::getDisplay(), m_pImage->buf, &pImgBuffer);
    VAAPIDecoder::checkError(status);

    AVG_ASSERT(m_pImage->format.fourcc == VA_FOURCC_BGRA);
    BitmapPtr pSrcBmp(new Bitmap(m_Size, R8G8B8X8, 
            (uint8_t*)pImgBuffer + m_pImage->offsets[0], m_pImage->pitches[0], false));
    pBmp->copyPixels(*pSrcBmp);
    // Make sure the alpha channel is white.
    // TODO: This is slow. Make OpenGL do it.
    unsigned char * pLine = pBmp->getPixels();
    IntPoint size = pBmp->getSize();
    for (int y = 0; y < size.y; ++y) {
        unsigned char * pPixel = pLine;
        for (int x = 0; x < size.x; ++x) {
            pPixel[3] = 0xFF;
            pPixel += 4;
        }
        pLine = pLine + pBmp->getStride();
    }
}

void VAAPISurface::copyToTexture(GLTexturePtr pTex)
{
    void* pVAGLSurface;
    VAStatus status;
    status = vaCreateSurfaceGLX(VAAPIDecoder::getDisplay(), GL_TEXTURE_2D, pTex->getID(), 
            &pVAGLSurface);
    VAAPIDecoder::checkError(status);
    status = vaCopySurfaceGLX(VAAPIDecoder::getDisplay(), pVAGLSurface, m_SurfaceID, 
            VA_FRAME_PICTURE);
    VAAPIDecoder::checkError(status);
    status = vaDestroySurfaceGLX(VAAPIDecoder::getDisplay(), pVAGLSurface);
    VAAPIDecoder::checkError(status);
}

void VAAPISurface::splitInterleaved(BitmapPtr pBmpU, BitmapPtr pBmpV, BitmapPtr pSrcBmp)
{
    unsigned char * pULine = pBmpU->getPixels();
    unsigned char * pVLine = pBmpV->getPixels();
    unsigned char * pSrcLine = pSrcBmp->getPixels();
    for (int y=0; y<pBmpU->getSize().y; y++) {
        for (int x=0; x<pBmpU->getSize().x; x++) {    
            pULine[x] = pSrcLine[x*2];
            pVLine[x] = pSrcLine[x*2+1];
        }
        pULine += pBmpU->getStride();
        pVLine += pBmpV->getStride();
        pSrcLine += pSrcBmp->getStride();
    }
}

}

