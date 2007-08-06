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
//  Original author of this file is igor@c-base.org.
//

#include "FilterDistortion.h"

#include <iostream>
#include <math.h>

using namespace std;

namespace avg{

    FilterDistortion::FilterDistortion(IntPoint srcSize, CoordTransformerPtr coordtrans)
      : m_srcRect(0,0,srcSize.x,srcSize.y),
        m_pTrafo(coordtrans) 
    {
        //we use the same dimensions for both of src and dest and just crop...
        //for each pixel at (x,y) in the dest
        //m_pMap[x][y] contains a IntPoint that gives the coords in the src Bitmap 
        m_pMap = new IntPoint[m_srcRect.Height()*m_srcRect.Width()];
        int w = m_srcRect.Width();

        for(int y=0;y<srcSize.y;++y){
            for(int x=0;x<srcSize.x;++x){
                DPoint tmp = m_pTrafo->inverse_transform_point(DPoint(int(x),int(y)));
                IntPoint tmp2(int(tmp.x+0.5),int(tmp.y+0.5));
                if(m_srcRect.Contains(tmp2)){
                    m_pMap[y*w+x] = tmp2;
                } else {
                    m_pMap[y*w+x] = IntPoint(0,0);
/*
                    IntPoint SrcPoint(tmp2);
                    if (SrcPoint.x < m_srcRect.tl.x) {
                        SrcPoint.x = m_srcRect.tl.x;
                    } 
                    if (SrcPoint.x >= m_srcRect.br.x) {
                        SrcPoint.x = m_srcRect.br.x-1;
                    } 
                    if (SrcPoint.y < m_srcRect.tl.y) {
                        SrcPoint.y = m_srcRect.tl.y;
                    } 
                    if (SrcPoint.y >= m_srcRect.br.y) {
                        SrcPoint.y = m_srcRect.br.y-1;
                    }
                    m_pMap[y*w+x] = SrcPoint;
*/
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
        BitmapPtr res = BitmapPtr(new Bitmap(*pBmpSource));
        unsigned char *p = res->getPixels();
        unsigned char *src = pBmpSource->getPixels();
        unsigned char *pLine = p;
        int destStride = res->getStride();
        int srcStride = pBmpSource->getStride();
        int w=m_srcRect.Width();
        int h=m_srcRect.Height();
        IntPoint *pMapPos = m_pMap;
        for(int y=m_srcRect.tl.y;y<h;++y){
            for(int x=m_srcRect.tl.x;x<w;++x){
                *pLine = src[pMapPos->x + srcStride*pMapPos->y];
                pLine++;
                pMapPos++;
            }
            p+=destStride;
            pLine = p;
        }
        return res;
    }

}
