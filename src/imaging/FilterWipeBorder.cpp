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

#include "FilterWipeBorder.h"

#include <iostream>
#include <math.h>

using namespace std;

namespace avg {

FilterWipeBorder::FilterWipeBorder(int numPixels)
    : m_NumPixels(numPixels)
{
}

FilterWipeBorder::~FilterWipeBorder()
{
}

void FilterWipeBorder::applyInPlace(BitmapPtr pBmp)
{
    assert(pBmp->getPixelFormat() == I8);
    if (m_NumPixels != 0) {
        int stride = pBmp->getStride();
        unsigned char * pPixels = pBmp->getPixels();
        IntPoint size = pBmp->getSize();
        IntPoint activeSize = pBmp->getSize()-IntPoint(2*m_NumPixels, 2*m_NumPixels);

        unsigned char * pSrcLine = pPixels+stride*m_NumPixels+m_NumPixels;
        for (int y=m_NumPixels-1; y >= 0; --y) {
            memcpy(pPixels+stride*y+m_NumPixels, pSrcLine, activeSize.x);
        }
        pSrcLine = pPixels+stride*(size.y-m_NumPixels-1)+m_NumPixels;
        for (int y=size.y-m_NumPixels; y < size.y; ++y) {
            memcpy(pPixels+stride*y+m_NumPixels, pSrcLine, activeSize.x);
        }

        for (int y = 0; y < size.y; ++y) {
            unsigned char src = *(pPixels+stride*y+m_NumPixels);
            memset(pPixels+stride*y, src, m_NumPixels);
            src = *(pPixels+stride*y+size.x-m_NumPixels-1);
            memset(pPixels+stride*y+size.x-m_NumPixels, src, m_NumPixels);
        }
    }
}

}
