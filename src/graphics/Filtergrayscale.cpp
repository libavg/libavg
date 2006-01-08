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

#include "Filtergrayscale.h"

#include "Pixeldefs.h"

#include <iostream>

namespace avg {

using namespace std;
    
FilterGrayscale::FilterGrayscale() : Filter()
{
}

FilterGrayscale::~FilterGrayscale()
{

}

BitmapPtr FilterGrayscale::apply(BitmapPtr pBmpSrc) const
{
    PixelFormat PF = pBmpSrc->getPixelFormat();
    if (PF == I8) {
        return BitmapPtr(new Bitmap(*pBmpSrc));
    }
    BitmapPtr pBmpDest = BitmapPtr(new Bitmap(pBmpSrc->getSize(), I8,
             pBmpSrc->getName()));
    unsigned char * pSrcLine = pBmpSrc->getPixels();
    unsigned char * pDestLine = pBmpDest->getPixels();
    for (int y = 0; y<pBmpDest->getSize().y; ++y) {
        unsigned char * pSrcPixel = pSrcLine;
        unsigned char * pDstPixel = pDestLine;
        for (int x = 0; x < pBmpDest->getSize().x; ++x) {
            // For the coefficients used, see http://www.inforamp.net/~poynton/
            // Appoximations curtesy of libpng :-).
            if (PF == R8G8B8A8 || PF == R8G8B8X8 || PF == R8G8B8) {
                *pDstPixel = (unsigned char)((pSrcPixel[REDPOS]*54+
                        pSrcPixel[GREENPOS]*183+
                        pSrcPixel[BLUEPOS]*19)/256);
                pSrcPixel += pBmpSrc->getBytesPerPixel();
                ++pDstPixel;
            } else {
                *pDstPixel = (unsigned char)((pSrcPixel[BLUEPOS]*54+
                        pSrcPixel[GREENPOS]*183+
                        pSrcPixel[REDPOS]*19)/256);
                pSrcPixel += pBmpSrc->getBytesPerPixel();
                ++pDstPixel;
            }
        }
        pSrcLine = pSrcLine + pBmpSrc->getStride();
        pDestLine = pDestLine + pBmpDest->getStride();
    }
    return pBmpDest;
}

} // namespace
