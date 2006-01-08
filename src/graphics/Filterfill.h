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


#ifndef _Filterfill_H_
#define _Filterfill_H_

#include "Filter.h"
#include "Filterfillrect.h"
#include "Rect.h"

namespace avg {

// Filter that fills a bitmap with a color.
template<class PixelC> 
class FilterFill : public Filter
{
public:
  FilterFill (const PixelC& Color);
  virtual ~FilterFill();
  virtual void applyInPlace(BitmapPtr pBmp) const;

private:
  PixelC m_Color;
};

template<class PixelC>
FilterFill<PixelC>::FilterFill(const PixelC& Color)
    : m_Color (Color)
{
}

template<class PixelC>
FilterFill<PixelC>::~FilterFill()
{
}

template<class PixelC>
void FilterFill<PixelC>::applyInPlace(BitmapPtr pBmp) const
{
  FilterFillRect<PixelC> RectFilter(
          IntRect(0, 0, pBmp->getSize().x, pBmp->getSize().y), m_Color);
  RectFilter.applyInPlace(pBmp);
}

}

#endif
