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

#include "Filter3x3.h"
#include "Pixeldefs.h"

#include "../base/Exception.h"


namespace avg {
    
Filter3x3::Filter3x3(double Mat[3][3])
    : Filter()
{
    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
            m_Mat[y][x] = Mat[y][x];
        }
    }
}

Filter3x3::~Filter3x3()
{

}

BitmapPtr Filter3x3::apply(BitmapPtr pBmpSource) 
{
    IntPoint newSize(pBmpSource->getSize().x-2, pBmpSource->getSize().y-2);
    BitmapPtr pNewBmp(new Bitmap(newSize, pBmpSource->getPixelFormat(),
            pBmpSource->getName()+"_filtered"));
            
    for (int y = 0; y < newSize.y; y++) {
        const unsigned char * pSrc = pBmpSource->getPixels()+y*pBmpSource->getStride();
        unsigned char * pDest = pNewBmp->getPixels()+y*pNewBmp->getStride();
        switch (pBmpSource->getBytesPerPixel()) {
            case 4:
                convolveLine<Pixel32>(pSrc, pDest, newSize.x, pBmpSource->getStride());
                break;
            case 3:
                convolveLine<Pixel24>(pSrc, pDest, newSize.x, pBmpSource->getStride());
                break;
            default:
                AVG_ASSERT(false);
        }
    }
    return pNewBmp;
}

}
