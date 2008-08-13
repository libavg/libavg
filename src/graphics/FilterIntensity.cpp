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

#include "FilterIntensity.h"

#include <math.h>

namespace avg {

FilterIntensity::FilterIntensity(int offset, double factor)
  : m_Offset(offset),
    m_Factor(factor)
{
}

FilterIntensity::~FilterIntensity()
{
}

void FilterIntensity::applyInPlace(BitmapPtr pBmp)
{
    assert(pBmp->getPixelFormat() == I8);
    unsigned char * pLine = pBmp->getPixels();
    IntPoint size = pBmp->getSize();
    for (int y = 0; y<size.y; ++y) {
        unsigned char * pPixel = pLine;
        for (int x = 0; x < size.x; ++x) {
            *pPixel = (*pPixel+m_Offset)*m_Factor;
            ++pPixel;
        }
        pLine = pLine + pBmp->getStride();
    }
}

} // namespace

