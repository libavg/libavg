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

#include "FilterMask.h"
#include "Pixeldefs.h"

#include "../base/Exception.h"

#include <stdio.h>

namespace avg {
   
using namespace std;

FilterMask::FilterMask(BitmapPtr pMaskBmp)
    : Filter(),
      m_pMaskBmp(pMaskBmp)
{
    AVG_ASSERT(m_pMaskBmp->getPixelFormat() == I8);
}

FilterMask::~FilterMask()
{

}

void FilterMask::applyInPlace(BitmapPtr pBmp) 
{
    IntPoint size = pBmp->getSize();
    AVG_ASSERT(size == m_pMaskBmp->getSize());
    for (int y = 0; y < size.y; y++) {
        unsigned char * pMaskLine = m_pMaskBmp->getPixels()+y*m_pMaskBmp->getStride();
        unsigned char * pLine = pBmp->getPixels()+y*pBmp->getStride();
        switch (pBmp->getBytesPerPixel()) {
            case 4:
                for (int x = 0; x < size.x; x++) { 
                    unsigned char src = *(pMaskLine+x);
                    unsigned char * pPixel = pLine + x*4;
                    *pPixel = (*pPixel*src)/255;
                    *(pPixel+1) = (*(pPixel+1)*src)/255;
                    *(pPixel+2) = (*(pPixel+2)*src)/255;
                }
                break;
            case 3:
                for (int x = 0; x < size.x; x++) { 
                    unsigned char src = *(pMaskLine+x);
                    unsigned char * pPixel = pLine + x*3;
                    *pPixel = (*pPixel*src)/255;
                    *(pPixel+1) = (*(pPixel+1)*src)/255;
                    *(pPixel+2) = (*(pPixel+2)*src)/255;
                }
                break;
            case 1:
                for (int x = 0; x < size.x; x++) { 
                    unsigned char src = *(pMaskLine+x);
                    unsigned char * pPixel = pLine + x;
                    *pPixel = (*pPixel*src)/255;
                }
                break;
            default:
                AVG_ASSERT(false);
        }
    }
}

}
