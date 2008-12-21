//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2008 Ulrich von Zadow
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


#ifndef _FilterFloodfill_H_
#define _FilterFloodfill_H_

#include "../api.h"
#include "Filter.h"

#include "../base/Point.h"

#include <stack>

namespace avg {

struct Segment {
    Segment(int iy, int ix1, int ix2, int idy)
        : y(iy),
          x1(ix1),
          x2(ix2),
          dy(idy)
    {}

    int y;
    int x1;
    int x2;
    int dy;
};

class ColorTester {
public:
    ColorTester(Pixel32 color)
      : m_Color(color)
    {
    }

    bool operator()(unsigned char * pPixel) 
    {
        return *(Pixel32*)(pPixel) == m_Color;
    }

private:
    Pixel32 m_Color;
};

// Flood fill filter. The comparison functor used to determine whether a pixel
// belongs to the area is passed as template parameter. Pixels in the flooded area
// are made transparent.
template<class PixelTester> 
class AVG_TEMPLATE_API FilterFloodfill : public Filter
{
public:
    FilterFloodfill(PixelTester tester, const IntPoint& pt);
    virtual ~FilterFloodfill();

    virtual void applyInPlace(BitmapPtr pBmp) ;

private:
    void pushSeg(BitmapPtr pBmp, std::stack<Segment>& segments, 
            int y, int x1, int x2, int dy);
    void setTransparent(unsigned char* pPixel);

    PixelTester m_Tester;
    IntPoint m_Pt;
};

template<class PixelTester>
FilterFloodfill<PixelTester>::FilterFloodfill(PixelTester tester, const IntPoint& pt)
    : m_Tester(tester),
      m_Pt(pt)
{
}

template<class PixelTester>
FilterFloodfill<PixelTester>::~FilterFloodfill()
{
}

template<class PixelTester>
void FilterFloodfill<PixelTester>::applyInPlace(BitmapPtr pBmp) 
{
    // Algorithm from Graphics Gems I, pp 296ff.
    std::stack<Segment> segments;
    unsigned char * pPixels = (unsigned char *)(pBmp->getPixels());
    int stride = pBmp->getStride();
    pushSeg(pBmp, segments, m_Pt.y, m_Pt.x, m_Pt.x, 1);
    pushSeg(pBmp, segments, m_Pt.y+1, m_Pt.x, m_Pt.x, -1);
    int l;
    while (!segments.empty()) {
        Segment seg = segments.top();
        seg.y += seg.dy;
        segments.pop();
        int x = seg.x1;
        unsigned char * pCurPixel = pPixels+x*4+seg.y*stride;
        for (; x>=0 && m_Tester(pCurPixel); x--) {
            setTransparent(pCurPixel);
            pCurPixel-=4;
        }
        if (x >= seg.x1) {
            goto skip;
        }
        l = x+1;
        if (l<seg.x1) {
            pushSeg(pBmp, segments, seg.y, l, seg.x1-1, -seg.dy);
        }
        x = seg.x1+1;
        do {
            pCurPixel = pPixels+x*4+seg.y*stride; 
            for (; x<pBmp->getSize().x && m_Tester(pCurPixel); x++) {
                setTransparent(pCurPixel);
                pCurPixel+=4;
            }
            pushSeg(pBmp, segments, seg.y, l, x-1, seg.dy);
            if (x>seg.x2+1) {
                pushSeg(pBmp, segments, seg.y, seg.x2+1, x-1, -seg.dy);
            }
skip:       x++;
            pCurPixel = pPixels+x*4+seg.y*stride;
            for (; x<=seg.x2 && !m_Tester(pCurPixel); x++) {
                pCurPixel+=4;
            }
            l = x;
        } while (x<=seg.x2);
         
    }
}

template<class PixelTester>
void FilterFloodfill<PixelTester>::pushSeg(BitmapPtr pBmp, std::stack<Segment>& segments, 
        int y, int x1, int x2, int dy)
{
    if (y+dy >= 0 && y+dy < pBmp->getSize().y) {
        segments.push(Segment(y, x1, x2, dy));
    }
}

template<class PixelTester>
void FilterFloodfill<PixelTester>::setTransparent(unsigned char* pPixel)
{
    *(Pixel32*)(pPixel) = Pixel32(0,0,255,0);
}


}

#endif
