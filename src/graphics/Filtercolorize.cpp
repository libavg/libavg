//
// $Id$
//

#include "Filtercolorize.h"
#include "Filtergrayscale.h"
#include "Pixel24.h"

#include <math.h>

namespace avg {

// From Foley, van Dam p. 596 incl. addendum fixes.
unsigned char hls_value(double n1, double n2, double hue)
{
  if (hue>360)
    hue-=360;
  if (hue<0)   
    hue+=360;
  
  double rv;
  if (hue<60)  
    rv = n1+(n2-n1)*hue/60.0;
  else if (hue<180) 
    rv = n2;
  else if (hue<240) 
    rv = n1+(n2-n1)*(240.0-hue)/60.0;
  else 
    rv = n1;

  return (unsigned char)(rv*255);
}

Pixel24 hls2rgb (double h, double l, double s)
{
    double m1, m2;
    l /= 255;
    s /= 100;
    // Warning: Foley, van Dam has a typo on the next line!
    m2 = (l<=0.5)?(l*(1.0+s)):(l+s-l*s);
    m1 = 2.0*l-m2;
    if (s<0.001) {
        return Pixel24((unsigned char)(l*255), (unsigned char)(l*255), 
                (unsigned char)(l*255));
    } else {
        return Pixel24(hls_value(m1,m2,h+120.0),
                hls_value(m1,m2,h),
                hls_value(m1,m2,h-120.0));
    }
}

FilterColorize::FilterColorize(double Hue, double Saturation)
  : m_Hue(Hue),
    m_Saturation(Saturation)
{
}

FilterColorize::~FilterColorize()
{

}

void FilterColorize::applyInPlace(BitmapPtr pBmp) const
{
    BitmapPtr pTempBmp = FilterGrayscale().apply(pBmp);
    Pixel24 ColorTable[256];
    for (int i=0; i<256; i++) {
        ColorTable[i] = hls2rgb(m_Hue, i, m_Saturation);
    }

    unsigned char * pSrcLine = pTempBmp->getPixels();
    unsigned char * pDestLine = pBmp->getPixels();
    for (int y = 0; y<pTempBmp->getSize().y; ++y) {
        unsigned char * pSrcPixel = pSrcLine;
        switch (pBmp->getPixelFormat()) {
            case R8G8B8A8:
            case R8G8B8X8:
                {
                    Pixel32 * pDestPixel = (Pixel32 *)pDestLine;
                    for (int x = 0; x < pTempBmp->getSize().x; ++x) {
                        *pDestPixel = ColorTable[*pSrcPixel];
                        ++pSrcPixel;
                        ++pDestPixel;
                    }
                }
                break;
            case R8G8B8:
                {
                    Pixel24 * pDestPixel = (Pixel24 *)pDestLine;
                    for (int x = 0; x < pTempBmp->getSize().x; ++x) {
                        *pDestPixel = ColorTable[*pSrcPixel];
                        ++pSrcPixel;
                        ++pDestPixel;
                    }
                }
                break;
            default:
                assert(false);
        }
        pSrcLine = pSrcLine + pTempBmp->getStride();
        pDestLine = pDestLine + pBmp->getStride();
    }
}

} // namespace

