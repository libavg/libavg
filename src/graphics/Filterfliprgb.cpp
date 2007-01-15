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

#include "Filterfliprgb.h"
#include "Pixeldefs.h"

#include <assert.h>

namespace avg {
    
FilterFlipRGB::FilterFlipRGB()
  : Filter()
{
}

FilterFlipRGB::~FilterFlipRGB()
{

}

void FilterFlipRGB::applyInPlace(BitmapPtr pBmp) 
{
    PixelFormat PF = pBmp->getPixelFormat();
    switch(PF) {
        case B8G8R8A8:
            pBmp->setPixelFormat(R8G8B8A8);
            break;
        case B8G8R8X8:
            pBmp->setPixelFormat(R8G8B8X8);
            break;
        case R8G8B8A8:
            pBmp->setPixelFormat(B8G8R8A8);
            break;
        case R8G8B8X8:
            pBmp->setPixelFormat(B8G8R8X8);
            break;
        case R8G8B8:
            pBmp->setPixelFormat(B8G8R8);
            break;
        case B8G8R8:
            pBmp->setPixelFormat(R8G8B8);
            break;
        default:
            // Only 24 and 32 bpp supported.
            assert(false);
    }
    IntPoint size = pBmp->getSize();
    for (int y = 0; y < size.y; y++) {
        unsigned char * pLine = pBmp->getPixels()+y*pBmp->getStride();
        if (pBmp->getBytesPerPixel() == 4) {
            for (int x = 0; x < size.x; x++) { 
                unsigned char tmp = pLine[x*4+REDPOS];
                pLine[x*4+REDPOS] = pLine[x*4+BLUEPOS];
                pLine[x*4+BLUEPOS] = tmp;
            }
        } else {
            for (int x = 0; x < size.x; x++) { 
                unsigned char tmp = pLine[x*3+REDPOS];
                pLine[x*3+REDPOS] = pLine[x*3+BLUEPOS];
                pLine[x*3+BLUEPOS] = tmp;
            } 
        }
    }
}

} // namespace
