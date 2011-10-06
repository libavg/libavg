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
//  Original author of this file is igor@c-base.org.
//

#include "FilterDistortion.h"

#include <iostream>
#include <math.h>

using namespace std;

namespace avg {

FilterDistortion::FilterDistortion(const IntPoint& srcSize,
        CoordTransformerPtr pTransformer)
    : m_SrcSize(srcSize),
      m_pTransformer(pTransformer) 
{
    // We use the same dimensions for both of src and dest and just crop.
    // for each pixel at (x,y) in the dest, m_pMap[x][y] contains an IntPoint that gives 
    // the coords in the src Bitmap. 
    m_pMap = new IntPoint[m_SrcSize.y*m_SrcSize.x];
    for (int y = 0; y < m_SrcSize.y; ++y) {
        for (int x = 0; x < m_SrcSize.x; ++x) {
            DPoint tmp = m_pTransformer->inverse_transform_point(DPoint(int(x),int(y)));
            IntPoint tmp2(int(tmp.x+0.5),int(tmp.y+0.5));
            if (tmp2.x < m_SrcSize.x && tmp2.y < m_SrcSize.y &&
                    tmp2.x >= 0 && tmp2.y >= 0)
            {
                m_pMap[y*m_SrcSize.x+x] = tmp2;
            } else {
                m_pMap[y*m_SrcSize.x+x] = IntPoint(0,0);
            }
        }
    }
}

FilterDistortion::~FilterDistortion()
{
    delete[] m_pMap;
}

BitmapPtr FilterDistortion::apply(BitmapPtr pBmpSource)
{
    BitmapPtr pDestBmp = BitmapPtr(new Bitmap(m_SrcSize, I8));
    unsigned char* pDestLine = pDestBmp->getPixels();
    unsigned char* pSrcPixels = pBmpSource->getPixels();
    unsigned char* pDestPixel = pDestLine;
    int destStride = pDestBmp->getStride();
    int srcStride = pBmpSource->getStride();
    IntPoint * pMapPos = m_pMap;
    for (int y = 0; y < m_SrcSize.y; ++y) {
        for(int x = 0; x < m_SrcSize.x; ++x) {
            *pDestPixel = pSrcPixels[pMapPos->x + srcStride*pMapPos->y];
            pDestPixel++;
            pMapPos++;
        }
        pDestLine+=destStride;
        pDestPixel = pDestLine;
    }
    return pDestBmp;
}

}
