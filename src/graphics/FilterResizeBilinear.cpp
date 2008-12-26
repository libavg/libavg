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

#include "FilterResizeBilinear.h"
#include "Bitmap.h"
#include "TwoPassScale.h"

namespace avg {

FilterResizeBilinear::FilterResizeBilinear (const IntPoint& newSize)
    : m_NewSize(newSize)
{
}

BitmapPtr FilterResizeBilinear::apply(BitmapPtr pBmpSrc)
{
    int bpp = pBmpSrc->getBytesPerPixel();
    assert(bpp==4 || bpp==3);

    BitmapPtr pBmpDest = BitmapPtr(new Bitmap(m_NewSize, 
            pBmpSrc->getPixelFormat(), pBmpSrc->getName()+"_resized"));

    BilinearContribDef f(0.64);
    switch(bpp) {
        case 4:
            {
                TwoPassScale<CDataRGBA_UBYTE> sS(f);
                sS.Scale((CDataRGBA_UBYTE::PixelClass *) pBmpSrc->getPixels(), 
                        pBmpSrc->getSize(), pBmpSrc->getStride(), 
                        (CDataRGBA_UBYTE::PixelClass *) pBmpDest->getPixels(),
                        pBmpDest->getSize(), pBmpDest->getStride());
            }
            break;
        case 3:
            {
                TwoPassScale <CDataRGB_UBYTE> sS(f);
                sS.Scale((CDataRGB_UBYTE::PixelClass *) pBmpSrc->getPixels(), 
                        pBmpSrc->getSize(), pBmpSrc->getStride(), 
                        (CDataRGB_UBYTE::PixelClass *) pBmpDest->getPixels(),
                        pBmpDest->getSize(), pBmpDest->getStride());
            }
            break;
        default:
            assert(false);
    }
    return pBmpDest;
}

}

