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


#ifndef _Filterfillrect_H_
#define _Filterfillrect_H_

#include "Filter.h"

namespace avg {

#include "Filter.h"
#include "Rect.h"

// Filter that fills a rectangle in a Bitmap with a color. 
template<class PixelC> 
    class FilterFillRect: public Filter
{
public:
    FilterFillRect (IntRect Rect, const PixelC& Color);
    virtual ~FilterFillRect();
    virtual void applyInPlace(BitmapPtr pBmp) const;

private:
    PixelC m_Color;
    IntRect m_Rect;
};

template<class PixelC>
FilterFillRect<PixelC>::FilterFillRect (IntRect Rect, const PixelC& Color)
{
  m_Rect = Rect;
  m_Color = Color;
}

template<class PixelC>
FilterFillRect<PixelC>::~FilterFillRect ()
{
}

template<class PixelC>
void FilterFillRect<PixelC>::applyInPlace (BitmapPtr pBmp) const
{
    for (int y=m_Rect.tl.y; y<m_Rect.br.y; ++y) {
        PixelC * pLine = (PixelC*)(pBmp->getPixels()+y*pBmp->getStride());
        for (int x=m_Rect.tl.x; x<m_Rect.br.x; ++x) {
            pLine[x] = m_Color;
        }
    }
}

}
#endif
