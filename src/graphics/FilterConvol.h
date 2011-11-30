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

#ifndef _FilterConvol_H
#define _FilterConvol_H

#include "../api.h"
#include "Filter.h"

#include "Pixel8.h"
#include "Pixel24.h"
#include "Pixel32.h"

#include <iostream>

namespace avg {

// Filter that applies a 3x3 kernel to the bitmap.
template<class Pixel>
class AVG_API FilterConvol : public Filter
{
public:
    FilterConvol(float *Mat,int n,int m, int offset=0);
    virtual ~FilterConvol();
    virtual BitmapPtr apply(BitmapPtr pBmpSource);

private:
    void convolveLine(const unsigned char* pSrc, unsigned char* pDest, 
            int lineLen, int stride, int offset = 0) const;
    int m_N;
    int m_M;
    int m_Offset;
    float *m_Mat;
};
template <class Pixel>
void FilterConvol<Pixel>::convolveLine(const unsigned char* pSrc, unsigned char* pDest, 
        int lineLen, int stride, int offset) const 
{
    Pixel * pSrcPixel = (Pixel *)pSrc;
    Pixel * pDestPixel = (Pixel *)pDest;
    for (int x=0; x<lineLen; ++x) {
        float NewR = 0;
        float NewG = 0;
        float NewB = 0;

        for (int i=0; i<m_N; i++) {
            for (int j=0; j<m_N; j++) {
                unsigned char * pLineStart = (unsigned char *)pSrcPixel+i*stride;
                Pixel SrcPixel = *((Pixel *)pLineStart+j);
                NewR += SrcPixel.getR()*m_Mat[m_N*i+j];
                NewG += SrcPixel.getG()*m_Mat[m_N*i+j];
                NewB += SrcPixel.getB()*m_Mat[m_N*i+j];
            }
        }
        *pDestPixel = Pixel((unsigned char)(NewR+offset), (unsigned char)(NewG+offset), 
                (unsigned char)(NewB+offset));
        
        pSrcPixel++;
        pDestPixel++;
    }
}
template <>
void FilterConvol<Pixel8>::convolveLine(const unsigned char* pSrc, unsigned char* pDest, 
        int lineLen, int stride, int offset) const 
{
    Pixel8 * pSrcPixel = (Pixel8 *)pSrc;
    Pixel8 * pDestPixel = (Pixel8 *)pDest;
    for (int x=0; x<lineLen; ++x) {
        float New = 0;

        for (int i=0; i<m_N; i++) {
            for (int j=0; j<m_N; j++) {
                unsigned char * pLineStart = (unsigned char *)pSrcPixel+i*stride;
                Pixel8 SrcPixel = *((Pixel8 *)pLineStart+j);
                New += SrcPixel.get()*m_Mat[m_N*i+j];
            }
        }
        *pDestPixel = Pixel8((unsigned char)(New+offset));
        
        pSrcPixel++;
        pDestPixel++;
    }
}
template <class Pixel>
FilterConvol<Pixel>::FilterConvol(float *Mat, int n, int m, int offset)
  : Filter(),
    m_N(n),
    m_M(m),
    m_Offset(offset)
{
    m_Mat = new float[n*m];
    for (int y=0; y<n; y++) {
        for (int x=0; x<m; x++) {
            m_Mat[m_N*y+x] = Mat[m_N*y+x];
        }
    }
}
template <class Pixel>
FilterConvol<Pixel>::~FilterConvol()
{
    delete[] m_Mat;

}
template <class Pixel>
BitmapPtr FilterConvol<Pixel>::apply(BitmapPtr pBmpSource) 
{
    IntPoint NewSize(pBmpSource->getSize().x-m_N+1, pBmpSource->getSize().y-m_M+1);
    BitmapPtr pNewBmp(new Bitmap(NewSize, pBmpSource->getPixelFormat(),
            pBmpSource->getName()+"_filtered"));
            
    for (int y = 0; y < NewSize.y; y++) {
        const unsigned char * pSrc = pBmpSource->getPixels()+y*pBmpSource->getStride();
        unsigned char * pDest = pNewBmp->getPixels()+y*pNewBmp->getStride();
        convolveLine(pSrc, pDest, NewSize.x, pBmpSource->getStride(), m_Offset);
    }
    return pNewBmp;
}


}

#endif

