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

#include "FilterFastBandpass.h"
#include "FilterBlur.h"
#include "Filterfill.h"
#include "Pixel8.h"
#include "Bitmap.h"

#include <cstring>
#include <iostream>
#include <sstream>

using namespace std;

namespace avg {
    
FilterFastBandpass::FilterFastBandpass()
{
}

FilterFastBandpass::~FilterFastBandpass()
{
}

BitmapPtr FilterFastBandpass::apply(BitmapPtr pBmpSrc)
{
    assert(pBmpSrc->getPixelFormat() == I8);
    BitmapPtr pBmpDest = BitmapPtr(new Bitmap(pBmpSrc->getSize(), I8,
            pBmpSrc->getName()));
    int SrcStride = pBmpSrc->getStride();
    int DestStride = pBmpDest->getStride();
    unsigned char * pSrcLine = pBmpSrc->getPixels()+3*SrcStride;
    unsigned char * pDestLine = pBmpDest->getPixels()+3*DestStride;
    IntPoint size = pBmpDest->getSize();
    for (int y = 3; y<size.y-3; ++y) {
        unsigned char * pSrcPixel = pSrcLine+3;
        unsigned char * pDstPixel = pDestLine;
        *pDstPixel++ = 128;
        *pDstPixel++ = 128;
        *pDstPixel++ = 128;
        for (int x = 3; x < size.x-3; ++x) {
            // Convolution Matrix is
            //  0  0  0  0  0  0  0 
            //  0 -2  0  0  0 -2  0
            //  0  0  1  0  1  0  0
            //  0  0  0  4  0  0  0
            //  0  0  1  0  1  0  0
            //  0 -2  0  0  0 -2  0
            //  0  0  0  0  0  0  0 
//            *pDstPixel = 128 - int(*(pSrcPixel-3*SrcStride-3) + *(pSrcPixel-3*SrcStride+3) +
//                *(pSrcPixel+3*SrcStride-3) + *(pSrcPixel+3*SrcStride+3)+2)/4; 
            *pDstPixel = 128
                - int(*(pSrcPixel-2*SrcStride-2)*2 + *(pSrcPixel-2*SrcStride+2)*2 - 
                      *(pSrcPixel-SrcStride-1) - *(pSrcPixel-1*SrcStride+1) -
                      *(pSrcPixel+SrcStride-1) - *(pSrcPixel+1*SrcStride+1) + 
                      *(pSrcPixel+2*SrcStride-2)*2 + *(pSrcPixel+2*SrcStride+2)*2+2)/4
                + *pSrcPixel;
/*
            unsigned char *pSrc = pSrcPixel-3*SrcStride-3;
            int Dest = *pSrc;
            pSrc += 6;
            Dest += *pSrc;
            pSrc += SrcStride-5;
            Dest += *pSrc;
            pSrc += 4;
            Dest += *pSrc;
            pSrc += 4*SrcStride-4;
            Dest += *pSrc;
            pSrc += 4;
            Dest += *pSrc;
            pSrc += SrcStride-5;
            Dest += *pSrc;
            pSrc += 6;
            Dest += *pSrc;
            Dest /= 8;
            *pDstPixel = 128+(*pSrcPixel)-Dest;
*/
            ++pSrcPixel;
            ++pDstPixel;
        }
        *pDstPixel++ = 128;
        *pDstPixel++ = 128;
        *pDstPixel++ = 128;
        pSrcLine += SrcStride;
        pDestLine += DestStride;
    }
    // Set top and bottom borders.
    memset(pBmpDest->getPixels(), 128, DestStride*3);
    memset(pBmpDest->getPixels()+DestStride*(size.y-3), 128, DestStride*3);
    return pBmpDest;
}

}
