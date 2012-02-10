//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2011 Ulrich von Zadow
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  Current versions can be found at www.libavg.de
//

#include "Filtercolorize.h"
#include "Filtergrayscale.h"
#include "Pixel24.h"

#include "../base/Exception.h"

namespace avg {

// From Foley, van Dam p. 596 incl. addendum fixes.
unsigned char hls_value(float n1, float n2, float hue)
{
  if (hue>360)
    hue-=360;
  if (hue<0)   
    hue+=360;
  
  float rv;
  if (hue<60)  
    rv = n1+(n2-n1)*hue/60;
  else if (hue<180) 
    rv = n2;
  else if (hue<240) 
    rv = n1+(n2-n1)*(240-hue)/60;
  else 
    rv = n1;

  return (unsigned char)(rv*255);
}

Pixel24 hls2rgb (float h, float l, float s)
{
    float m1, m2;
    l /= 255;
    s /= 100;
    // Warning: Foley, van Dam has a typo on the next line!
    m2 = (l<=0.5f)?(l*(1.0f+s)):(l+s-l*s);
    m1 = 2.0f*l-m2;
    if (s<0.001f) {
        return Pixel24((unsigned char)(l*255), (unsigned char)(l*255), 
                (unsigned char)(l*255));
    } else {
        return Pixel24(hls_value(m1,m2,h+120),
                hls_value(m1,m2,h),
                hls_value(m1,m2,h-120));
    }
}

FilterColorize::FilterColorize(float hue, float saturation)
  : m_Hue(hue),
    m_Saturation(saturation)
{
}

FilterColorize::~FilterColorize()
{

}

void FilterColorize::applyInPlace(BitmapPtr pBmp)
{
    BitmapPtr pTempBmp (FilterGrayscale().apply(pBmp));
    Pixel32 colorTable[256];
    for (int i=0; i<256; i++) {
        colorTable[i] = hls2rgb(m_Hue, float(i), m_Saturation);
    }

    unsigned char * pSrcLine = pTempBmp->getPixels();
    unsigned char * pDestLine = pBmp->getPixels();
    IntPoint size = pTempBmp->getSize();
    for (int y = 0; y < size.y; ++y) {
        unsigned char * pSrcPixel = pSrcLine;
        switch (pBmp->getPixelFormat()) {
            case R8G8B8A8:
            case R8G8B8X8:
                {
                    Pixel32 * pDestPixel = (Pixel32 *)pDestLine;
                    for (int x = 0; x < size.x; ++x) {
                        *pDestPixel = colorTable[*pSrcPixel];
                        ++pSrcPixel;
                        ++pDestPixel;
                    }
                }
                break;
            case R8G8B8:
                {
                    Pixel24 * pDestPixel = (Pixel24 *)pDestLine;
                    for (int x = 0; x < size.x; ++x) {
                        *pDestPixel = colorTable[*pSrcPixel];
                        ++pSrcPixel;
                        ++pDestPixel;
                    }
                }
                break;
            case B8G8R8A8:
            case B8G8R8X8:
                {
                    Pixel32 * pDestPixel = (Pixel32 *)pDestLine;
                    for (int x = 0; x < size.x; ++x) {
                        *pDestPixel = colorTable[*pSrcPixel];
                        ++pSrcPixel;
                        ++pDestPixel;
                    }
                }
                break;
            case B8G8R8:
                {
                    Pixel24 * pDestPixel = (Pixel24 *)pDestLine;
                    for (int x = 0; x < size.x; ++x) {
                        *pDestPixel = colorTable[*pSrcPixel];
                        ++pSrcPixel;
                        ++pDestPixel;
                    }
                }
                break;
            default:
                AVG_ASSERT(false);
        }
        pSrcLine = pSrcLine + pTempBmp->getStride();
        pDestLine = pDestLine + pBmp->getStride();
    }
}

}

