//
// $Id$
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
