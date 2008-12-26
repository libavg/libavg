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

#include "FilterGetAlpha.h"

#include "Pixeldefs.h"

#include <iostream>

namespace avg {

using namespace std;
    
FilterGetAlpha::FilterGetAlpha() : Filter()
{
}

FilterGetAlpha::~FilterGetAlpha()
{

}

BitmapPtr FilterGetAlpha::apply(BitmapPtr pBmpSrc) 
{
    PixelFormat pf = pBmpSrc->getPixelFormat();
    assert(pf == R8G8B8A8 || pf == B8G8R8A8);
    BitmapPtr pBmpDest = BitmapPtr(new Bitmap(pBmpSrc->getSize(), I8,
             pBmpSrc->getName()+"alpha"));
    unsigned char * pSrcLine = pBmpSrc->getPixels();
    unsigned char * pDestLine = pBmpDest->getPixels();
    IntPoint size = pBmpDest->getSize();
    for (int y = 0; y<size.y; ++y) {
        unsigned char * pSrcPixel = pSrcLine;
        unsigned char * pDstPixel = pDestLine;
        for (int x = 0; x < size.x; ++x) {
            *pDstPixel = pSrcPixel[ALPHAPOS];
            pSrcPixel += 4;
            ++pDstPixel;
        }
        pSrcLine = pSrcLine + pBmpSrc->getStride();
        pDestLine = pDestLine + pBmpDest->getStride();
    }
    return pBmpDest;
}

} // namespace
