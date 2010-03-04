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

#include "FilterClearBorder.h"

#include "../base/Exception.h"

#include <iostream>
#include <math.h>
#include <string.h>

using namespace std;

namespace avg {

FilterClearBorder::FilterClearBorder(int numPixels)
    : m_NumPixels(numPixels)
{
}

FilterClearBorder::~FilterClearBorder()
{
}

void FilterClearBorder::applyInPlace(BitmapPtr pBmp)
{
    AVG_ASSERT(pBmp->getPixelFormat() == I8);
    AVG_ASSERT(m_NumPixels < pBmp->getSize().x);
    AVG_ASSERT(m_NumPixels < pBmp->getSize().y);
    if (m_NumPixels != 0) {
        int stride = pBmp->getStride();
        unsigned char * pPixels = pBmp->getPixels();
        IntPoint size = pBmp->getSize();
        IntPoint activeSize = pBmp->getSize()-IntPoint(2*m_NumPixels, 2*m_NumPixels);
        for (int y=m_NumPixels-1; y >= 0; --y) {
            memset(pPixels+stride*y+m_NumPixels, 0, activeSize.x);
        }
        for (int y=size.y-m_NumPixels; y < size.y; ++y) {
            memset(pPixels+stride*y+m_NumPixels, 0, activeSize.x);
        }

        for (int y = 0; y < size.y; ++y) {
            memset(pPixels+stride*y, 0, m_NumPixels);
            memset(pPixels+stride*y+size.x-m_NumPixels, 0, m_NumPixels);
        }
    }
}

}
