//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
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

#ifndef _Filter3x3_H
#define _Filter3x3_H

#include "Filter.h"

#include "Pixel24.h"
#include "Pixel32.h"

#include <iostream>

namespace avg {

// Filter that applies a 3x3 kernel to the bitmap.
class Filter3x3 : public Filter
{
public:
    Filter3x3(double Mat[3][3]);
    virtual ~Filter3x3();
    virtual BitmapPtr apply(BitmapPtr pBmpSource) const;

private:
    template<class Pixel>
    void Filter3x3::convolveLine(const unsigned char * pSrc, unsigned char * pDest, 
            int lineLen, int stride) const;
    double m_Mat[3][3];
};

template<class Pixel>
void Filter3x3::convolveLine(const unsigned char * pSrc, unsigned char * pDest, 
        int lineLen, int stride) const 
{
    Pixel * pSrcPixel = (Pixel *)pSrc;
    Pixel * pDestPixel = (Pixel *)pDest;
    for (int x=0; x<lineLen; ++x) {
        double NewR = 0;
        double NewG = 0;
        double NewB = 0;

        for (int i=0; i<3; i++) {
            for (int j=0; j<3; j++) {
                unsigned char * pLineStart = (unsigned char *)pSrcPixel+i*stride;
                Pixel SrcPixel = *((Pixel *)pLineStart+j);
                NewR += SrcPixel.getR()*m_Mat[i][j];
                NewG += SrcPixel.getG()*m_Mat[i][j];
                NewB += SrcPixel.getB()*m_Mat[i][j];
            }
        }
        *pDestPixel = Pixel24((unsigned char)NewR, (unsigned char)NewG, 
                (unsigned char)NewB);
        
        pSrcPixel++;
        pDestPixel++;
    }
}

}

#endif

