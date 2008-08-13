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

#include "FilterBlur.h"
#include "Filterfill.h"
#include "Pixel8.h"
#include "Bitmap.h"

#include <iostream>
#include <math.h>

using namespace std;

namespace avg {
    
FilterBlur::FilterBlur()
{
}

FilterBlur::~FilterBlur()
{
}

BitmapPtr FilterBlur::apply(BitmapPtr pBmpSrc)
{
    assert(pBmpSrc->getPixelFormat() == I8);
    
    IntPoint Size(pBmpSrc->getSize().x-2, pBmpSrc->getSize().y-2);
    BitmapPtr pDestBmp = BitmapPtr(new Bitmap(Size, I8, pBmpSrc->getName()));
    int SrcStride = pBmpSrc->getStride();
    int DestStride = pDestBmp->getStride();
    unsigned char * pSrcLine = pBmpSrc->getPixels()+SrcStride+1;
    unsigned char * pDestLine = pDestBmp->getPixels();
    for (int y = 0; y<Size.y; ++y) {
        unsigned char * pSrcPixel = pSrcLine;
        unsigned char * pDestPixel = pDestLine;
        for (int x = 0; x < Size.x; ++x) {
            *pDestPixel = (*(pSrcPixel-1) + *(pSrcPixel)*4 + *(pSrcPixel+1)
                    +*(pSrcPixel-SrcStride)+*(pSrcPixel+SrcStride)+4)/8;
            ++pSrcPixel;
            ++pDestPixel;
        }
        pSrcLine += SrcStride;
        pDestLine += DestStride;
    }
    return pDestBmp;
}

}
