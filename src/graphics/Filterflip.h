//
// $Id$
//


#ifndef _Filterflip_H_
#define _Filterflip_H_

#include "Filter.h"

namespace avg {

// Flips a bitmap upside-down
class FilterFlip: public Filter
{
public:
  FilterFlip();
  virtual ~FilterFlip();
  virtual BitmapPtr apply(BitmapPtr pBmpSource) const;
};

} // namespace

#endif

