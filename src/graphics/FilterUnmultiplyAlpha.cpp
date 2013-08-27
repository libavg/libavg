//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2011 Ulrich von Zadow
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
        unsigned char * pPixel = pBmp->getPixels()+y*pBmp->getStride();
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
}

}
