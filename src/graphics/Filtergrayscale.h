//
// $Id$
//

#ifndef _Filtergrayscale_H_
#define _Filtergrayscale_H_

#include "Filter.h"
#include "Bitmap.h"

namespace avg {

// Creates a grayscale version of a 32 bpp bitmap.
class FilterGrayscale : public Filter
{
public:
  FilterGrayscale();
  virtual ~FilterGrayscale();
  virtual BitmapPtr apply(BitmapPtr pBmpSource) const;

private:
};

} // namespace

#endif

