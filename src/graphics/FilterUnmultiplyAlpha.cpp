//
//  libavg - Media Playback Engine.
//  Copyright (C) 2003-2021 Ulrich von Zadow
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

#include "FilterUnmultiplyAlpha.h"
#include "Pixeldefs.h"

#include "../base/Exception.h"
#include "../base/ScopeTimer.h"

namespace avg {
    
FilterUnmultiplyAlpha::FilterUnmultiplyAlpha()
  : Filter()
{
}

FilterUnmultiplyAlpha::~FilterUnmultiplyAlpha()
{

}

static ProfilingZoneID ProfilingZone("FilterUnmultiplyAlpha");

void FilterUnmultiplyAlpha::applyInPlace(BitmapPtr pBmp) 
{
    ScopeTimer Timer(ProfilingZone);
    AVG_ASSERT(pBmp->getBytesPerPixel() == 4);
    IntPoint size = pBmp->getSize();
    for (int y = 0; y < size.y; y++) {
        unsigned char * pPixel = pBmp->getPixels() + y*pBmp->getStride();
        for (int x = 0; x < size.x; x++) { 
            int alpha = *(pPixel+ALPHAPOS);
            if (alpha != 0) {
                *(pPixel+REDPOS) = (int(*(pPixel+REDPOS))*255)/alpha;
                *(pPixel+GREENPOS) = (int(*(pPixel+GREENPOS))*255)/alpha;
                *(pPixel+BLUEPOS) = (int(*(pPixel+BLUEPOS))*255)/alpha;
            }
            pPixel += 4;
        }
    }
    // The color values of transparent pixels are used in bilinear texture
    // filtering in certain conditions. To avoid artifacts, we transfer the
    // color from a neighboring non-transparent pixel.
    for (int y = 1; y < size.y-1; y++) {
        int stride = pBmp->getStride();
        unsigned char * pPixel = pBmp->getPixels() + y*stride + 4;
        for (int x = 1; x < size.x-1; x++) {
            int alpha = *(pPixel+ALPHAPOS);
            if (alpha == 0) {
                unsigned char * pSrcPixel = pPixel;
                if (*(pPixel+4+ALPHAPOS) != 0) {
                    pSrcPixel = pPixel+4;
                } else if (*(pPixel+stride+4+ALPHAPOS) != 0) {
                    pSrcPixel = pPixel+stride+4;
                } else if (*(pPixel+stride+ALPHAPOS) != 0) {
                    pSrcPixel = pPixel+stride;
                } else if (*(pPixel+stride-4+ALPHAPOS) != 0) {
                    pSrcPixel = pPixel+stride-4;
                } else if (*(pPixel-4+ALPHAPOS) != 0) {
                    pSrcPixel = pPixel-4;
                } else if (*(pPixel-stride-4+ALPHAPOS) != 0) {
                    pSrcPixel = pPixel-stride-4;
                } else if (*(pPixel-stride+ALPHAPOS) != 0) {
                    pSrcPixel = pPixel-stride;
                } else if (*(pPixel-stride+4+ALPHAPOS) != 0) {
                    pSrcPixel = pPixel-stride+4;
                }

                *(pPixel+REDPOS) = *(pSrcPixel+REDPOS);
                *(pPixel+GREENPOS) = *(pSrcPixel+GREENPOS);
                *(pPixel+BLUEPOS) = *(pSrcPixel+BLUEPOS);
            }
            pPixel += 4;
        }
    }
}

}
