//
// $Id$
//

#include "Filtergrayscale.h"

#include "Pixeldefs.h"

#include <iostream>

namespace avg {

using namespace std;
    
FilterGrayscale::FilterGrayscale() : Filter()
{
}

FilterGrayscale::~FilterGrayscale()
{

}

BitmapPtr FilterGrayscale::apply(BitmapPtr pBmpSrc) const
{
    PixelFormat PF = pBmpSrc->getPixelFormat();
    if (PF == I8) {
        return BitmapPtr(new Bitmap(*pBmpSrc));
    }
    assert (PF == R8G8B8A8 || PF == R8G8B8X8 || PF == R8G8B8);

    BitmapPtr pBmpDest = BitmapPtr(new Bitmap(pBmpSrc->getSize(), I8,
             pBmpSrc->getName()));
    unsigned char * pSrcLine = pBmpSrc->getPixels();
    unsigned char * pDestLine = pBmpDest->getPixels();
    for (int y = 0; y<pBmpDest->getSize().y; ++y) {
        unsigned char * pSrcPixel = pSrcLine;
        unsigned char * pDstPixel = pDestLine;
        for (int x = 0; x < pBmpDest->getSize().x; ++x) {
            // For the coefficients used, see http://www.inforamp.net/~poynton/
            // Appoximations curtesy of libpng :-).
            *pDstPixel = (unsigned char)((pSrcPixel[REDPOS]*54+
                    pSrcPixel[GREENPOS]*183+
                    pSrcPixel[BLUEPOS]*19)/256);
            pSrcPixel += pBmpSrc->getBytesPerPixel();
            ++pDstPixel;
        }
        pSrcLine = pSrcLine + pBmpSrc->getStride();
        pDestLine = pDestLine + pBmpDest->getStride();
    }
    return pBmpDest;
}

} // namespace
