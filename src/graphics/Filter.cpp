//
// $Id$
//

#include "Filter.h"
#include "Bitmap.h"

#include <iostream>

using namespace std;

namespace avg {

Filter::Filter()
{
}

void Filter::applyInPlace(BitmapPtr pBmp) const
{
    *pBmp = *(apply(pBmp));
}

BitmapPtr Filter::apply(BitmapPtr pBmpSource) const
{
    BitmapPtr pBmpDest = BitmapPtr(new Bitmap(*pBmpSource));
    applyInPlace (pBmpDest);
    return pBmpDest;
}

} // namespace
