//
// $Id$
//

#ifndef _Filterfliprgb_H
#define _Filterfliprgb_H

#include "Filter.h"

namespace avg {

// Filter that flips the R and B components of a bitmap.
class FilterFlipRGB : public Filter
{
public:
  FilterFlipRGB();
  virtual ~FilterFlipRGB();
  virtual void applyInPlace(BitmapPtr pBmp) const;

private:
};

}

#endif

