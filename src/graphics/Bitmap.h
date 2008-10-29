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
#include "../base/Point.h"
#include "../base/Rect.h"

#include <boost/shared_ptr.hpp>
#include <stdlib.h>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>

namespace avg {

// The pixelformats are named in the order the components appear in memory.
//      I.e.: B8G8R8X8 is blue at byte 0, green at byte 1, red at byte 2,
//            unused byte 3.
// Pixelformats used:
//   - ImageMagick: R8G8B8X8 and R8G8B8A8
//   - OpenGL: Uses RGB ordering: I8 is GL_ALPHA, R8G8B8 is GL_RGB,
//          R8G8B8A8 is GL_RGBA
//   - DFB: Uses BGR ordering but calls it RGB. DSPF_RGB16 is B5G6R5, 
//          DSPF_RGB24 is B8G8R8, DSPF_RGB32 is B8G8R8X8, 
//          DSPF_ARGB is B8G8R8A8.
typedef enum {
    B5G6R5, B8G8R8, B8G8R8A8, B8G8R8X8, A8B8G8R8, X8B8G8R8,
    R5G6B5, R8G8B8, R8G8B8A8, R8G8B8X8, A8R8G8B8, X8R8G8B8,
    I8, I16, 
    YCbCr411,  // Interleaved YCbCr: Y,Y,Cb,Y,Y,Cr,... Effectively 12 bits per pixel.
    YCbCr422,  // Interleaved YCbCr: Cb,Y,Cr,Y,...
    YUYV422,   // Interleaved YCbCr, grey values first: Y,Cb,Y,Cr,...
    YCbCr420p, // Not really a valid pixel format. Signifies separate bitmaps
               // for Y, Cb and Cr components, with Cb and Cr half as big in 
               // both x and y dimensions. This is mpeg YCbCr, where the 
               // color components have values from 16...235.
    YCbCrJ420p,// Same as YCbCr420p, but this is the jpeg version with component
               // values in the range 0...255
    BAYER8_GBRG,
    NO_PIXELFORMAT
} PixelFormat;

typedef std::vector<int> Histogram;
typedef boost::shared_ptr<Histogram> HistogramPtr;

class AVG_API Bitmap
{
public:
    Bitmap(IntPoint Size, PixelFormat PF, const std::string& sName="");
    Bitmap(IntPoint Size, PixelFormat PF, unsigned char * pBits, 
            int Stride, bool bCopyBits, const std::string& sName="");
    Bitmap(const Bitmap& Orig);
    Bitmap(const Bitmap& Orig, bool bOwnsBits);
    Bitmap(Bitmap& Orig, const IntRect& Rect);
    Bitmap(const std::string& sURI);
    virtual ~Bitmap();

    Bitmap &operator= (const Bitmap & Orig);
    
    // Does pixel format conversion if nessesary.
    void copyPixels(const Bitmap & Orig);
    void save(const std::string& sName);
    
    IntPoint getSize() const;
    int getStride() const;
    PixelFormat getPixelFormat() const;
    void setPixelFormat(PixelFormat PF);
    std::string getPixelFormatString() const;
    static std::string getPixelFormatString(PixelFormat PF);
    unsigned char * getPixels();
    const unsigned char * getPixels() const;
    std::string getPixelsAsString() const;
    void setPixels(const unsigned char * pPixels);
    void setPixelsFromString(const std::string& sPixels);
    bool ownsBits() const;
    const std::string& getName() const;
    int getBytesPerPixel() const;
    static int getBytesPerPixel(PixelFormat PF);
    int getLineLen() const;
    int getMemNeeded() const;
    bool hasAlpha() const;
    HistogramPtr getHistogram(int Stride = 1) const;
    void getMinMax(int Stride, int& min, int& max) const;

    template<class Pixel>
    void setPixel(const IntPoint& p, Pixel Color);
    template<class Pixel>
    void drawLine(IntPoint p0, IntPoint p1, Pixel Color);

    Bitmap * subtract(const Bitmap* pOtherBmp);
    double getAvg() const;
    double getStdDev() const;

    bool operator ==(const Bitmap & otherBmp);
    void dump(bool bDumpPixels=false) const;

private:
    void initWithData(unsigned char * pBits, int Stride, bool bCopyBits);
    void allocBits();
    void YCbCrtoBGR(const Bitmap& Orig);
    void YCbCrtoI8(const Bitmap& Orig);
    void I8toI16(const Bitmap& Orig);
    void I8toRGB(const Bitmap& Orig);
    void I16toI8(const Bitmap& Orig);
    void BY8toRGBNearest(const Bitmap& Orig);
    void BY8toRGBBilinear(const Bitmap& Orig);

    IntPoint m_Size;
    int m_Stride;
    PixelFormat m_PF;
    unsigned char * m_pBits;
    bool m_bOwnsBits;
    std::string m_sName;
};

typedef boost::shared_ptr<Bitmap> BitmapPtr;

template<class Pixel>
void Bitmap::setPixel(const IntPoint& p, Pixel Color)
{
    *(Pixel*)(&(m_pBits[p.y*m_Stride+p.x*getBytesPerPixel()])) = Color;
}

// TODO: This is slow, and it clips incorrectly. Replace with external lib?
template<class Pixel>
void Bitmap::drawLine(IntPoint p0, IntPoint p1, Pixel Color)
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
            setPixel(IntPoint(y, x), Color); 
        } else {
            setPixel(IntPoint(x, y), Color);
        }
        error += deltay;
        if (error > 0) {
            y += ystep;
            error -= deltax;
        }
    }
}

}
#endif
