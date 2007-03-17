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

#include "FilterBandpass.h"
#include "Filterfill.h"
#include "Pixel8.h"
#include "Bitmap.h"

#include <iostream>
#include <math.h>

using namespace std;

namespace avg {
    
FilterBandpass::FilterBandpass(double lowWidth, double highWidth)
    : m_HighpassFilter(highWidth),
      m_LowpassFilter(lowWidth)
{
    m_FilterWidthDiff = int(ceil(highWidth))-int(ceil(lowWidth));
}

FilterBandpass::~FilterBandpass()
{
}

BitmapPtr FilterBandpass::apply(BitmapPtr pBmpSrc)
{
    BitmapPtr pLPBmp = m_LowpassFilter.apply(pBmpSrc);
    BitmapPtr pHPBmp = m_HighpassFilter.apply(pBmpSrc);

    IntPoint Size = pHPBmp->getSize();
    BitmapPtr pDestBmp = BitmapPtr(new Bitmap(Size, I8, pBmpSrc->getName()));

    int LPStride = pLPBmp->getStride();
    int HPStride = pHPBmp->getStride();
    int DestStride = pDestBmp->getStride();
    unsigned char * pLPLine = pLPBmp->getPixels()+m_FilterWidthDiff*LPStride;
    unsigned char * pHPLine = pHPBmp->getPixels();
    unsigned char * pDestLine = pDestBmp->getPixels();
    for (int y = 0; y<Size.y; ++y) {
        unsigned char * pLPPixel = pLPLine+m_FilterWidthDiff;
        unsigned char * pHPPixel = pHPLine;
        unsigned char * pDestPixel = pDestLine;
        for (int x = 0; x < Size.x; ++x) {
            *pDestPixel = (int(*pLPPixel)-*pHPPixel)+128;
            ++pLPPixel;
            ++pHPPixel;
            ++pDestPixel;
        }
        pLPLine += LPStride;
        pHPLine += HPStride;
        pDestLine += DestStride;
    }
    return pDestBmp;
}


}
