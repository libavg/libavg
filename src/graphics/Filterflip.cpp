//
// $Id$
//

#include "Filterflip.h"

namespace avg {

FilterFlip::FilterFlip() : Filter()
{
}

FilterFlip::~FilterFlip()
{
}

BitmapPtr FilterFlip::apply(BitmapPtr pBmpSource) const
{
    IntPoint Size = pBmpSource->getSize();
    BitmapPtr pBmpDest = BitmapPtr(new Bitmap (Size, 
                pBmpSource->getPixelFormat(), pBmpSource->getName()));

    unsigned char* pSrcLine = pBmpSource->getPixels();
    unsigned char* pDestLine = pBmpDest->getPixels()+(Size.y-1)*pBmpDest->getStride();
    int LineLen = pBmpSource->getBytesPerPixel()*Size.x;

    for (int y = 0; y<Size.y; y++) {
        memcpy(pDestLine, pSrcLine, LineLen);
        pSrcLine += pBmpSource->getStride();
        pDestLine -= pBmpDest->getStride();
    }
    return pBmpDest;
}

} // namespace
