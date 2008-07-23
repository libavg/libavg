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

#include "FilterNormalize.h"
#include "FilterIntensity.h"

#include <math.h>

namespace avg {

FilterNormalize::FilterNormalize(int stride)
    : m_Stride(stride)
{
}

FilterNormalize::~FilterNormalize()
{
}

void FilterNormalize::applyInPlace(BitmapPtr pBmp)
{
    int min;
    int max;
    pBmp->getMinMax(m_Stride, min, max);
    if (m_Stride > 1) {
        min -= 2;
        max += 2;
    }
    double factor = 255./(max-min);
    if (factor > 10) {
        factor = 10;
    }
    FilterIntensity(-min, factor).applyInPlace(pBmp); 
}

} // namespace

