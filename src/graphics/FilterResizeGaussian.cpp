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

#include "FilterResizeGaussian.h"
#include "Bitmap.h"
#include "TwoPassScale.h"

namespace avg {

FilterResizeGaussian::FilterResizeGaussian(const IntPoint& newSize, double radius)
    : m_NewSize(newSize),
      m_Radius(radius)
{
}

BitmapPtr FilterResizeGaussian::apply(BitmapPtr pBmpSrc)
{
    assert(pBmpSrc->getBytesPerPixel()==4);

    BitmapPtr pBmpDest = BitmapPtr(new Bitmap(m_NewSize, 
            pBmpSrc->getPixelFormat(), pBmpSrc->getName()+"_resized"));

    GaussianContribDef f(m_Radius);
    TwoPassScale<CDataRGBA_UBYTE> sS(f);
    sS.Scale((CDataRGBA_UBYTE::PixelClass *) pBmpSrc->getPixels(), 
            pBmpSrc->getSize(), pBmpSrc->getStride(), 
            (CDataRGBA_UBYTE::PixelClass *) pBmpDest->getPixels(),
            pBmpDest->getSize(), pBmpDest->getStride());
    return pBmpDest;
}

}

