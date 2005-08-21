//
// $Id$
//

#ifndef _Filter_H_
#define _Filter_H_

#include "Bitmap.h"

namespace avg {

// Base class for filters that operate on bitmaps. Derived classes need
// to override either the ApplyInPlace or the Apply function. The base-class
// versions of these functions simply implement one function in terms of the
// other.
class Filter
{
public:
  Filter();
  virtual ~Filter() {};

  // In-Place Apply. Applies the filter to pBmp. The base-class
  // version copies the bitmap after calling Apply (pBmp, pTempBmp).
  virtual void applyInPlace(BitmapPtr pBmp) const;  

  // Applies the Filter to pBmpSource and returns the result
  // The base-class version copies the bitmap before calling
  // applyInPlace.
  virtual BitmapPtr apply(BitmapPtr pBmpSource) const;
};

} // namespace
#endif

