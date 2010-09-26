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

#include "FilterHighpass.h"
#include "Filterfill.h"
#include "Pixel8.h"
#include "Bitmap.h"

#include "../base/Exception.h"

#include <cstring>
#include <iostream>
#include <sstream>

using namespace std;

namespace avg {
    
FilterHighpass::FilterHighpass()
{
}

FilterHighpass::~FilterHighpass()
{
}

BitmapPtr FilterHighpass::apply(BitmapPtr pBmpSrc)
{
    AVG_ASSERT(pBmpSrc->getPixelFormat() == I8);
    BitmapPtr pBmpDest = BitmapPtr(new Bitmap(pBmpSrc->getSize(), I8,
            pBmpSrc->getName()));
    int srcStride = pBmpSrc->getStride();
    int destStride = pBmpDest->getStride();
    unsigned char * pSrcLine = pBmpSrc->getPixels()+3*srcStride;
    unsigned char * pDestLine = pBmpDest->getPixels()+3*destStride;
    IntPoint size = pBmpDest->getSize();
    for (int y = 3; y < size.y-3; ++y) {
        unsigned char * pSrcPixel = pSrcLine+3;
        unsigned char * pDstPixel = pDestLine;
        *pDstPixel++ = 128;
        *pDstPixel++ = 128;
        *pDstPixel++ = 128;
        for (int x = 3; x < size.x-3; ++x) {
            // Convolution Matrix is
            // -1  0  0   0  -1
            //  0 -1  0  -1   0
            //  0  0  8   0   0
            //  0 -1  0  -1   0
            // -1  0  0   0  -1
            // Actually, it's 7x7, but you get the idea.
            *pDstPixel = 128 - int(*(pSrcPixel-3*srcStride-3) 
                    + *(pSrcPixel-3*srcStride+3) + *(pSrcPixel+3*srcStride-3) 
                    + *(pSrcPixel+3*srcStride+3))/16; 
            *pDstPixel += 
                - int(*(pSrcPixel-2*srcStride-2) + *(pSrcPixel-2*srcStride+2) + 
                      *(pSrcPixel-srcStride-1) + *(pSrcPixel-1*srcStride+1) +
                      *(pSrcPixel+srcStride-1) + *(pSrcPixel+1*srcStride+1) + 
                      *(pSrcPixel+2*srcStride-2) + *(pSrcPixel+2*srcStride+2))/16
                + *(pSrcPixel)*3/4;

            ++pSrcPixel;
            ++pDstPixel;
        }
        *pDstPixel++ = 128;
        *pDstPixel++ = 128;
        *pDstPixel++ = 128;
        pSrcLine += srcStride;
        pDestLine += destStride;
    }
    // Set top and bottom borders.
    memset(pBmpDest->getPixels(), 128, destStride*3);
    memset(pBmpDest->getPixels()+destStride*(size.y-3), 128, destStride*3);
    return pBmpDest;
}

}
