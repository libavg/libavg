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

#include "FilterflipX.h"
#include "Pixel32.h"

#include "../base/Exception.h"

namespace avg {

FilterFlipX::FilterFlipX() 
    : Filter()
{
}

FilterFlipX::~FilterFlipX()
{
}

BitmapPtr FilterFlipX::apply(BitmapPtr pBmpSource) 
{
    IntPoint Size = pBmpSource->getSize();
    BitmapPtr pBmpDest(new Bitmap (Size, 
                pBmpSource->getPixelFormat(), pBmpSource->getName()));

    unsigned char* pSrcLine = pBmpSource->getPixels();
    unsigned char* pDestLine = pBmpDest->getPixels();
    
    for (int y = 0; y<Size.y; y++) {
        switch (pBmpSource->getBytesPerPixel()) {
            case 1: {
                    unsigned char * pSrc = pSrcLine;
                    unsigned char * pDest = pDestLine+Size.x-1;
                    for (int x = 0; x<Size.x; x++) {
                        *pDest = *pSrc;
                        pSrc++;
                        pDest--;
                    }
                }
                break;
            case 4: {
                    Pixel32 * pSrc = (Pixel32*)pSrcLine;
                    Pixel32 * pDest = (Pixel32*)pDestLine+Size.x-1;
                    for (int x = 0; x<Size.x; x++) {
                        *pDest = *pSrc;
                        pSrc++;
                        pDest--;
                    }
                }
                break;
            default: 
                AVG_ASSERT(false);
        } 
        pSrcLine += pBmpSource->getStride();
        pDestLine += pBmpDest->getStride();
    }
    return pBmpDest;
}

} // namespace
