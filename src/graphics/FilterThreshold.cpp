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

#include "FilterThreshold.h"
#include "Pixeldefs.h"

#include "../base/Exception.h"

#include <stdio.h>

namespace avg {
   
using namespace std;

FilterThreshold::FilterThreshold(int threshold)
    : Filter(),
      m_Threshold(threshold)
{
}

FilterThreshold::~FilterThreshold()
{

}

void FilterThreshold::applyInPlace(BitmapPtr pBmp) 
{
    IntPoint size = pBmp->getSize();
    AVG_ASSERT(pBmp->getPixelFormat() == I8);
    for (int y = 0; y < size.y; y++) {
        unsigned char * pLine = pBmp->getPixels()+y*pBmp->getStride();
        for (int x = 0; x < size.x; x++) { 
            unsigned char * pPixel = pLine + x;
            if (*pPixel >= m_Threshold) {
                *pPixel = 255;
            } else {
                *pPixel = 0;
            }
        }
    }
}

} // namespace
