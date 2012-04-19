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

#ifndef _Filter3x3_H
#define _Filter3x3_H

#include "../api.h"
#include "Filter.h"

#include "Pixel8.h"
#include "Pixel24.h"
#include "Pixel32.h"

#include <iostream>

namespace avg {

// Filter that applies a 3x3 kernel to the bitmap.
class AVG_API Filter3x3 : public Filter
{
public:
    Filter3x3(float Mat[3][3]);
    virtual ~Filter3x3();
    virtual BitmapPtr apply(BitmapPtr pBmpSource);

private:
    template<class PIXEL>
    void convolveLine(const unsigned char * pSrc, unsigned char * pDest,
            int lineLen, int stride) const;
    float m_Mat[3][3];
};

template<class PIXEL>
void Filter3x3::convolveLine(const unsigned char * pSrc, unsigned char * pDest,
        int lineLen, int stride) const
{
    PIXEL * pSrcPixel = (PIXEL *)pSrc;
    PIXEL * pDestPixel = (PIXEL *)pDest;
    for (int x = 0; x < lineLen; ++x) {
        float newR = 0;
        float newG = 0;
        float newB = 0;

        for (int i = 0; i < 3; i++) {
            unsigned char * pLineStart = (unsigned char *)pSrcPixel+i*stride;
            for (int j = 0; j < 3; j++) {
                PIXEL SrcPixel = *((PIXEL *)pLineStart+j);
                newR += SrcPixel.getR()*m_Mat[i][j];
                newG += SrcPixel.getG()*m_Mat[i][j];
                newB += SrcPixel.getB()*m_Mat[i][j];
            }
        }
        *pDestPixel = PIXEL((unsigned char)newR, (unsigned char)newG,
                (unsigned char)newB);

        pSrcPixel++;
        pDestPixel++;
    }
}

}

#endif

