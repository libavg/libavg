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

#include "Filterfliprgba.h"
#include "Pixeldefs.h"

#include <assert.h>

namespace avg {
    
FilterFlipRGBA::FilterFlipRGBA()
  : Filter()
{
}

FilterFlipRGBA::~FilterFlipRGBA()
{

}

void FilterFlipRGBA::applyInPlace(BitmapPtr pBmp) 
{
    assert(pBmp->getBytesPerPixel() == 4);
    IntPoint size = pBmp->getSize();
    for (int y = 0; y < size.y; y++) {
        unsigned char * pLine = pBmp->getPixels()+y*pBmp->getStride();
        for (int x = 0; x < size.x; x++) { 
            unsigned char tmp = pLine[x*4+REDPOS];
            pLine[x*4+REDPOS] = pLine[x*4+ALPHAPOS];
            pLine[x*4+ALPHAPOS] = tmp;
            tmp = pLine[x*4+BLUEPOS];
            pLine[x*4+BLUEPOS] = pLine[x*4+GREENPOS];
            pLine[x*4+GREENPOS] = tmp;
        }
    }
}

} // namespace
