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

#ifndef _Bitmap_H_
#define _Bitmap_H_

#include "../api.h"
#include "Pixel32.h"
#include "PixelFormat.h"

#include "../base/Point.h"
#include "../base/Rect.h"
#include "../base/UTF8String.h"

#include <boost/shared_ptr.hpp>

#if defined(__SSE__) || defined(_WIN32)
#include <xmmintrin.h>
#endif

#include <stdlib.h>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>

namespace avg {

typedef std::vector<int> Histogram;
typedef boost::shared_ptr<Histogram> HistogramPtr;

class AVG_API Bitmap
{
public:
    Bitmap(DPoint size, PixelFormat pf, const UTF8String& sName="", int stride=0);
    Bitmap(IntPoint size, PixelFormat pf, const UTF8String& sName="", int stride=0);
    Bitmap(IntPoint size, PixelFormat pf, unsigned char * pBits, 
            int stride, bool bCopyBits, const UTF8String& sName="");
    Bitmap(const Bitmap& origBmp);
    Bitmap(const Bitmap& origBmp, bool bOwnsBits);
    Bitmap(Bitmap& origBmp, const IntRect& rect);
    Bitmap(const UTF8String& sName);
    virtual ~Bitmap();

    Bitmap &operator =(const Bitmap & origBmp);
    
    // Does pixel format conversion if nessesary.
    void copyPixels(const Bitmap & origBmp);
    void copyYUVPixels(const Bitmap & yBmp, const Bitmap& uBmp, const Bitmap& vBmp,
            bool bJPEG);
    void save(const UTF8String& sName);
    
    IntPoint getSize() const;
    int getStride() const;
    PixelFormat getPixelFormat() const;
    void setPixelFormat(PixelFormat pf);
    unsigned char * getPixels();
    const unsigned char * getPixels() const;
    std::string getPixelsAsString() const;
    void setPixels(const unsigned char * pPixels);
    void setPixelsFromString(const std::string& sPixels);
    bool ownsBits() const;
    const std::string& getName() const;
    int getBytesPerPixel() const;
    static int getBytesPerPixel(PixelFormat pf);
    int getLineLen() const;
    int getMemNeeded() const;
    bool hasAlpha() const;
    HistogramPtr getHistogram(int stride = 1) const;
    void getMinMax(int stride, int& min, int& max) const;
    void setAlpha(const Bitmap& alphaBmp);

    Pixel32 getPythonPixel(const DPoint& pos);
    template<class PIXEL>
    void setPixel(const IntPoint& p, PIXEL color);
    template<class PIXEL>
    void drawLine(IntPoint p0, IntPoint p1, PIXEL color);

    Bitmap * subtract(const Bitmap* pOtherBmp);
    void blt(const Bitmap* pOtherBmp, const IntPoint& pos);
    double getAvg() const;
    double getChannelAvg(int channel) const;
    double getStdDev() const;

    bool operator ==(const Bitmap & otherBmp);
    void dump(bool bDumpPixels=false) const;

private:
    void initWithData(unsigned char * pBits, int stride, bool bCopyBits);
    void allocBits(int stride=0);
    void YCbCrtoBGR(const Bitmap& origBmp);
    void YCbCrtoI8(const Bitmap& origBmp);
    void I8toI16(const Bitmap& origBmp);
    void I8toRGB(const Bitmap& origBmp);
    void I16toI8(const Bitmap& origBmp);
    void BGRtoB5G6R5(const Bitmap& origBmp);
    void ByteRGBAtoFloatRGBA(const Bitmap& origBmp);
    void FloatRGBAtoByteRGBA(const Bitmap& origBmp);
    void BY8toRGBNearest(const Bitmap& origBmp);
    void BY8toRGBBilinear(const Bitmap& origBmp);

    IntPoint m_Size;
    int m_Stride;
    PixelFormat m_PF;
    unsigned char * m_pBits;
    bool m_bOwnsBits;
    UTF8String m_sName;

    static bool s_bMagickInitialized;
};

typedef boost::shared_ptr<Bitmap> BitmapPtr;

BitmapPtr YCbCr2RGBBitmap(BitmapPtr pYBmp, BitmapPtr pUBmp, BitmapPtr pVBmp);

template<class PIXEL>
void Bitmap::setPixel(const IntPoint& p, PIXEL color)
{
    *(PIXEL*)(&(m_pBits[p.y*m_Stride+p.x*getBytesPerPixel()])) = color;
}

// TODO: This is slow, and it clips incorrectly. Replace with external lib?
template<class PIXEL>
void Bitmap::drawLine(IntPoint p0, IntPoint p1, PIXEL color)
{
    IntRect BmpRect(IntPoint(0,0), m_Size);
    p0 = BmpRect.cropPoint(p0);
    p1 = BmpRect.cropPoint(p1);

    bool bSteep = abs(p1.y - p0.y) > abs(p1.x - p0.x);
    if (bSteep) {
        std::swap(p0.x, p0.y);
        std::swap(p1.x, p1.y);
    }
    if (p0.x > p1.x) {
        std::swap(p0, p1); 
    }
    int deltax = p1.x - p0.x;
    int deltay = abs(p1.y - p0.y);
    int error = -deltax/2;
    int ystep;
    int y = p0.y;
    if (p0.y < p1.y) {
        ystep = 1;
    } else {
        ystep = -1;
    }
    for (int x = p0.x; x <= p1.x; x++) {
        if (bSteep) {
            setPixel(IntPoint(y, x), color); 
        } else {
            setPixel(IntPoint(x, y), color);
        }
        error += deltay;
        if (error > 0) {
            y += ystep;
            error -= deltax;
        }
    }
}
#if defined(__SSE__) || defined(_WIN32)
std::ostream& operator<<(std::ostream& os, const __m64 &val);
#endif

}
#endif
