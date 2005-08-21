//
// $Id$
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
