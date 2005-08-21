//
// $Id$
//

#include "Filterfliprgb.h"
#include "Pixeldefs.h"

#include <assert.h>

namespace avg {
    
FilterFlipRGB::FilterFlipRGB()
  : Filter()
{
}

FilterFlipRGB::~FilterFlipRGB()
{

}

void FilterFlipRGB::applyInPlace(BitmapPtr pBmp) const
{
    PixelFormat PF = pBmp->getPixelFormat();
    switch(PF) {
        case B8G8R8A8:
            pBmp->setPixelFormat(R8G8B8A8);
            break;
        case B8G8R8X8:
            pBmp->setPixelFormat(R8G8B8X8);
            break;
        case R8G8B8A8:
            pBmp->setPixelFormat(B8G8R8A8);
            break;
        case R8G8B8X8:
            pBmp->setPixelFormat(B8G8R8X8);
            break;
        case R8G8B8:
            pBmp->setPixelFormat(B8G8R8);
            break;
        case B8G8R8:
            pBmp->setPixelFormat(R8G8B8);
            break;
        default:
            // Only 24 and 32 bpp supported.
            assert(false);
    }

    for (int y = 0; y < pBmp->getSize().y; y++) {
        unsigned char * pLine = pBmp->getPixels()+y*pBmp->getStride();
        if (pBmp->getBytesPerPixel() == 4) {
            for (int x = 0; x < pBmp->getSize().x; x++) { 
                unsigned char tmp = pLine[x*4+REDPOS];
                pLine[x*4+REDPOS] = pLine[x*4+BLUEPOS];
                pLine[x*4+BLUEPOS] = tmp;
            }
        } else {
            for (int x = 0; x < pBmp->getSize().x; x++) { 
                unsigned char tmp = pLine[x*3+REDPOS];
                pLine[x*3+REDPOS] = pLine[x*3+BLUEPOS];
                pLine[x*3+BLUEPOS] = tmp;
            } 
        }
    }
}

} // namespace
