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

#include "FilterDilation.h"

#include "../base/Exception.h"

#include <algorithm>

using namespace std;

namespace avg {
    
FilterDilation::FilterDilation()
  : Filter()
{
}

FilterDilation::~FilterDilation()
{

}

BitmapPtr FilterDilation::apply(BitmapPtr pSrcBmp) 
{
    AVG_ASSERT(pSrcBmp->getPixelFormat() == I8);
    IntPoint size = pSrcBmp->getSize();
    BitmapPtr pDestBmp = BitmapPtr(new Bitmap(size, I8, pSrcBmp->getName()));
    unsigned char * pSrcLine = pSrcBmp->getPixels();
    unsigned char * pNextSrcLine;
    unsigned char * pDestLine = pDestBmp->getPixels();
    for (int y = 0; y < size.y; y++) {
        pDestLine = pDestBmp->getPixels()+y*pDestBmp->getStride();
        unsigned char * pLastSrcLine = pSrcLine;
        pSrcLine = pSrcBmp->getPixels()+y*pSrcBmp->getStride();
        if (y < size.y-1) {
            pNextSrcLine = pSrcBmp->getPixels()+(y+1)*pSrcBmp->getStride();
        } else {
            pNextSrcLine = pSrcBmp->getPixels()+y*pSrcBmp->getStride();
        }
        pDestLine[0] = max(pSrcLine[0], max(pSrcLine[1], 
                max(pLastSrcLine[0], pNextSrcLine[0])));
        for (int x = 1; x < size.x-1; x++) { 
            pDestLine[x] = max(pSrcLine[x], max(pSrcLine[x-1], max(pSrcLine[x+1], 
                    max(pLastSrcLine[x], pNextSrcLine[x]))));
        }
        pDestLine[size.x-1] = max(pSrcLine[size.x-2], max(pSrcLine[size.x-1], 
                max(pLastSrcLine[size.x-1], pNextSrcLine[size.x-1])));
    }
    return pDestBmp;
}

} // namespace
