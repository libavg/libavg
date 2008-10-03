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

#include "Bitmap.h"
#include "Pixel32.h"
#include "Pixel24.h"
#include "Pixel16.h"
#include "Pixel8.h"
#include "Filter3x3.h"

#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/ObjectCounter.h"

#include <Magick++.h>
#include <assert.h>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <stdlib.h>

using namespace Magick;
using namespace std;

namespace avg {

template<class Pixel>
void createTrueColorCopy(Bitmap& Dest, const Bitmap & Src);

Bitmap::Bitmap(IntPoint Size, PixelFormat PF, const std::string& sName)
    : m_Size(Size),
      m_PF(PF),
      m_pBits(0),
      m_bOwnsBits(true),
      m_sName(sName)
{
//    cerr << "Bitmap::Bitmap(" << Size << ", " << getPixelFormatString(m_PF) << ", " 
//        << sName << ")" << endl;
    ObjectCounter::get()->incRef(&typeid(*this));
    allocBits();
}

Bitmap::Bitmap(IntPoint Size, PixelFormat PF, unsigned char * pBits, 
        int Stride, bool bCopyBits, const std::string& sName)
    : m_Size(Size),
      m_PF(PF),
      m_pBits(0),
      m_sName(sName)
{
    ObjectCounter::get()->incRef(&typeid(*this));
//    cerr << "Bitmap::Bitmap(" << Size << ", " << getPixelFormatString(m_PF) << ", " 
//        << (void *)pBits << ", " << Stride << ", " << bCopyBits << ", "
//        << sName << ")" << endl;
    initWithData(pBits, Stride, bCopyBits);
}

Bitmap::Bitmap(const Bitmap& Orig)
    : m_Size(Orig.getSize()),
      m_PF(Orig.getPixelFormat()),
      m_pBits(0),
      m_bOwnsBits(Orig.m_bOwnsBits),
      m_sName(Orig.getName()+" copy")
{
//    cerr << "Bitmap::Bitmap(Bitmap), Name: " << m_sName << endl;
    ObjectCounter::get()->incRef(&typeid(*this));
    initWithData(const_cast<unsigned char *>(Orig.getPixels()), Orig.getStride(), 
            m_bOwnsBits);
}

Bitmap::Bitmap(const Bitmap& Orig, bool bOwnsBits)
    : m_Size(Orig.getSize()),
      m_PF(Orig.getPixelFormat()),
      m_pBits(0),
      m_bOwnsBits(bOwnsBits),
      m_sName(Orig.getName()+" copy")
{
//    cerr << "Bitmap::Bitmap(Bitmap), Name: " << m_sName << endl;
    ObjectCounter::get()->incRef(&typeid(*this));
    initWithData(const_cast<unsigned char *>(Orig.getPixels()), Orig.getStride(), 
            m_bOwnsBits);
}

// Creates a bitmap that is a rectangle in another bitmap. The pixels are
// still owned by the original bitmap.
Bitmap::Bitmap(Bitmap& Orig, const IntRect& Rect)
    : m_Size(Rect.size()),
      m_PF(Orig.getPixelFormat()),
      m_pBits(0),
      m_bOwnsBits(false)
{
//    cerr << "Bitmap::Bitmap(Bitmap, " << Rect << "), Name: " << m_sName << endl;
    ObjectCounter::get()->incRef(&typeid(*this));
    assert(Rect.br.x <= Orig.getSize().x);
    assert(Rect.br.y <= Orig.getSize().y);
    if (!Orig.getName().empty()) {
        m_sName = Orig.getName()+" part";
    } else {
        m_sName = "";
    }
    unsigned char * pRegionStart = Orig.getPixels()+Rect.tl.y*Orig.getStride()+
            Rect.tl.x*getBytesPerPixel();
    initWithData(pRegionStart, Orig.getStride(), false);
}

Bitmap::Bitmap(const std::string& sURI)
    : m_pBits(0),
      m_sName(sURI)
{
// TODO: This function loads grayscale images as RGB. That is because Magick++
// provides no reliable way to determine whether a bitmap is grayscale or not. Maybe
// the imagemagick C interface is less buggy? 
// It also returns RGB bitmaps, but I think nearly everywhere in libavg, the bytes
// are swapped and BGR is used.
//    cerr << "Bitmap::Bitmap(" << sURI << ")" << endl;
    Image Img;
    try {
        Img.read(sURI);
    } catch( Magick::Warning &e) {
        cerr << e.what() << endl;
    } catch( Magick::ErrorConfigure &) {
//        cerr << e.what() << endl;
    }
    PixelPacket * pSrcPixels = Img.getPixels(0, 0, Img.columns(), Img.rows());
    m_Size = IntPoint(Img.columns(), Img.rows());
    if (Img.matte()) {
        m_PF = R8G8B8A8;
    } else {
        m_PF = R8G8B8X8;
    }
    allocBits();
    for (int y=0; y<m_Size.y; ++y) {
        Pixel32 * pDestLine = (Pixel32 *)(m_pBits+m_Stride*y);
        PixelPacket * pSrcLine = pSrcPixels+y*Img.columns();
        if (m_PF == R8G8B8A8) {
            for (int x=0; x<m_Size.x; ++x) {
                *pDestLine = Pixel32(pSrcLine->red, pSrcLine->green, 
                        pSrcLine->blue, 255-pSrcLine->opacity);
                pSrcLine++;
                pDestLine++;
            }
        } else {
            for (int x=0; x<m_Size.x; ++x) {
                *pDestLine = Pixel32(pSrcLine->red, pSrcLine->green, 
                        pSrcLine->blue, 255);
                pSrcLine++;
                pDestLine++;
            }
        }
    }
    m_bOwnsBits = true;
    ObjectCounter::get()->incRef(&typeid(*this));
}

Bitmap::~Bitmap()
{
//    cerr << "Bitmap::~Bitmap(), Name: " << m_sName << endl;
    ObjectCounter::get()->decRef(&typeid(*this));
    if (m_bOwnsBits) {
        delete[] m_pBits;
        m_pBits = 0;
    }
}

Bitmap &Bitmap::operator= (const Bitmap &Orig)
{
//    cerr << "Bitmap::operator=()" << endl;
    if (this != &Orig) {
        if (m_bOwnsBits) {
            delete[] m_pBits;
            m_pBits = 0;
        }
        m_Size = Orig.getSize();
        m_PF = Orig.getPixelFormat();
        m_bOwnsBits = Orig.m_bOwnsBits;
        m_sName = Orig.getName();
        initWithData(const_cast<unsigned char *>(Orig.getPixels()), Orig.getStride(),
                m_bOwnsBits);
    }
    return *this;
}

void Bitmap::copyPixels(const Bitmap & Orig)
{
//    cerr << "Bitmap::copyPixels(): " << getPixelFormatString(Orig.getPixelFormat()) << "->" 
//            << getPixelFormatString(m_PF) << endl;
    if (&Orig == this) {
        return;
    }
    if (Orig.getPixelFormat() == m_PF) {
        const unsigned char * pSrc = Orig.getPixels();
        unsigned char * pDest = m_pBits;
        int Height = min(Orig.getSize().y, m_Size.y);
        int LineLen = min(Orig.getLineLen(), getLineLen());
        int SrcStride = Orig.getStride();
        for (int y=0; y<Height; ++y) {
            memcpy(pDest, pSrc, LineLen);
            pDest += m_Stride;
            pSrc += SrcStride;
        }
    } else {
        switch (Orig.getPixelFormat()) {
            case YCbCr422:
            case YUYV422:
            case YCbCr411:
            case YCbCr420p:
                switch(m_PF) {
                    case B8G8R8X8:
                        YCbCrtoBGR(Orig);
                        break;
                    case I8:
                        YCbCrtoI8(Orig);
                    default: {
                            Bitmap TempBmp(getSize(), B8G8R8X8, "TempColorConversion");
                            TempBmp.YCbCrtoBGR(Orig);
                            copyPixels(TempBmp);
                        }
                        break;
                }
                break;
            case I16:
                if (m_PF == I8) {
                    I16toI8(Orig);
                } else {
                    Bitmap TempBmp(getSize(), I8, "TempColorConversion");
                    TempBmp.I16toI8(Orig);
                    copyPixels(TempBmp);
                }
                break;
            case I8:
                switch(m_PF) {
                    case I16:
                        I8toI16(Orig);
                        break;
                    case B8G8R8X8:
                    case B8G8R8A8:
                    case R8G8B8X8:
                    case R8G8B8A8:
                    case B8G8R8:
                    case R8G8B8:
                        I8toRGB(Orig);
                        break;
                    default: 
                        // Unimplemented conversion.
                        assert(false);
                }
                break;
            case BAYER8_GBRG:
                switch(m_PF) {
                    // Bayer patterns are saved as I8 bitmaps. So simply copy that
                    case I8:
                        {
                            const unsigned char * pSrc = Orig.getPixels();
                            unsigned char * pDest = m_pBits;
                            int Height = min(Orig.getSize().y, m_Size.y);
                            int LineLen = min(Orig.getLineLen(), getLineLen());
                            int SrcStride = Orig.getStride();
                            for (int y=0; y<Height; ++y) {
                                memcpy(pDest, pSrc, LineLen);
                                pDest += m_Stride;
                                pSrc += SrcStride;
                            }
                        }
                        break;
                    case B8G8R8X8:
                    case B8G8R8A8:
                    case R8G8B8X8:
                    case R8G8B8A8:
                        BY8toRGBBilinear(Orig);
                        break;
                    default: 
                        // Unimplemented conversion.
                        assert(false);
                }
                break;
            default:
                switch(m_PF) {
                    case B8G8R8A8:
                    case B8G8R8X8:
                    case A8B8G8R8:
                    case X8B8G8R8:
                    case R8G8B8A8:
                    case R8G8B8X8:
                    case A8R8G8B8:
                    case X8R8G8B8:
                        createTrueColorCopy<Pixel32>(*this, Orig);
                        break;
                    case B8G8R8:
                    case R8G8B8:
                        createTrueColorCopy<Pixel24>(*this, Orig);
                        break;
                    case B5G6R5:
                    case R5G6B5:
                        createTrueColorCopy<Pixel16>(*this, Orig);
                        break;
                    case I8:
                        createTrueColorCopy<Pixel8>(*this, Orig);
                        break;
                    default:
                        // Unimplemented conversion.
                        cerr << "Can't convert " << Orig.getPixelFormatString() << " to " 
                                << getPixelFormatString() << endl;
                        assert(false);
                }
        }
    }
}

void Bitmap::save(const std::string& sFilename)
{
//    cerr << "Bitmap::save()" << endl;
    string sPF;
    BitmapPtr pBmp;
    Magick::StorageType ChannelFormat = Magick::CharPixel;
    int AlphaOffset = -1;
    switch(m_PF) {
        case B5G6R5:
        case B8G8R8:
        case B8G8R8X8:
        case X8B8G8R8:
            pBmp = BitmapPtr(new Bitmap(m_Size, B8G8R8));
            pBmp->copyPixels(*this);
            sPF = "BGR";
            break;
        case R5G6B5:
        case R8G8B8:
        case R8G8B8X8:
        case X8R8G8B8:
            pBmp = BitmapPtr(new Bitmap(m_Size, R8G8B8));
            pBmp->copyPixels(*this);
            sPF = "RGB";
            break;
        case B8G8R8A8:
            pBmp = BitmapPtr(new Bitmap(*this));
            AlphaOffset = 3;
            sPF = "BGRA";
            break;
        case A8B8G8R8:
            pBmp = BitmapPtr(new Bitmap(*this));
            AlphaOffset = 0;
            sPF = "ABGR";
            break;
        case R8G8B8A8:
            pBmp = BitmapPtr(new Bitmap(*this));
            AlphaOffset = 3;
            sPF = "RGBA";
            break;
        case A8R8G8B8:
            pBmp = BitmapPtr(new Bitmap(*this));
            AlphaOffset = 0;
            sPF = "ARGB";
            break;
        case I16:   
            pBmp = BitmapPtr(new Bitmap(*this));
            ChannelFormat = Magick::ShortPixel;
            sPF = "I";
            break;
        case I8:
            pBmp = BitmapPtr(new Bitmap(*this));
            sPF = "I";
            break;
        default:
            cerr << "Unsupported pixel format " << getPixelFormatString(m_PF) 
                    << endl;
            assert(false);
    }
    if (AlphaOffset != -1) {
        int Stride = pBmp->getStride();
        unsigned char * pLine = pBmp->getPixels();
        for (int y=0; y<m_Size.y; ++y) {
            unsigned char * pPixel = pLine;
            for (int x=0; x<m_Size.x; ++x) {
                *(pPixel+AlphaOffset) = 255-*(pPixel+AlphaOffset);
                pPixel+=4;
            }
            pLine += Stride;
        }
    }
    Magick::Image Img(m_Size.x, m_Size.y, sPF, ChannelFormat, pBmp->getPixels());
    Img.write(sFilename);
}

IntPoint Bitmap::getSize() const
{
    return m_Size;
}

int Bitmap::getStride() const
{
    return m_Stride;
}

PixelFormat Bitmap::getPixelFormat() const
{
    return m_PF;
}

void Bitmap::setPixelFormat(PixelFormat PF)
{
    m_PF = PF;
}

std::string Bitmap::getPixelFormatString() const
{
    return getPixelFormatString(m_PF);
}
    
std::string Bitmap::getPixelFormatString(PixelFormat PF)
{
    switch (PF) {
        case B5G6R5:
            return "B5G6R5";
        case B8G8R8:
            return "B8G8R8";
        case B8G8R8A8:
            return "B8G8R8A8";
        case B8G8R8X8:
            return "B8G8R8X8";
        case A8B8G8R8:
            return "A8B8G8R8";
        case X8B8G8R8:
            return "X8B8G8R8";
        case R5G6B5:
            return "R5G6B5";
        case R8G8B8:
            return "R8G8B8";
        case R8G8B8A8:
            return "R8G8B8A8";
        case R8G8B8X8:
            return "R8G8B8X8";
        case A8R8G8B8:
            return "A8R8G8B8";
        case X8R8G8B8:
            return "X8R8G8B8";
        case I8:
            return "I8";
        case I16:
            return "I16";
        case YCbCr411:
            return "YCbCr411";
        case YCbCr422:
            return "YCbCr422";
        case YUYV422:
            return "YUYV422";
        case YCbCr420p:
            return "YCbCr420p";
        case YCbCrJ420p:
            return "YCbCrJ420p";
        case BAYER8_GBRG:
            return "BAYER8_GBRG";
        default:
            return "Unknown";
    }
}

unsigned char * Bitmap::getPixels()
{
    return m_pBits;
}

const unsigned char * Bitmap::getPixels() const
{
    return m_pBits;
}

string Bitmap::getPixelsAsString() const
{
    return string((char*)m_pBits, getMemNeeded());
}

void Bitmap::setPixels(const unsigned char * pPixels)
{
    memcpy(m_pBits, pPixels, getMemNeeded());
}

void Bitmap::setPixelsFromString(const std::string& sPixels)
{
    memcpy(m_pBits, sPixels.c_str(), getMemNeeded());
}


const std::string& Bitmap::getName() const
{
    return m_sName;
}

bool Bitmap::ownsBits() const
{
    return m_bOwnsBits;
}

int Bitmap::getBytesPerPixel() const
{
    return getBytesPerPixel(m_PF);
}

int Bitmap::getBytesPerPixel(PixelFormat PF)
{
    switch (PF) {
        case A8B8G8R8:
        case X8B8G8R8:
        case A8R8G8B8:
        case X8R8G8B8:
        case B8G8R8A8:
        case B8G8R8X8:
        case R8G8B8A8:
        case R8G8B8X8:
            return 4;
        case R8G8B8:
        case B8G8R8:
            return 3;
        case B5G6R5:
        case R5G6B5:
        case I16:
            return 2;
        case I8:
        case BAYER8_GBRG:
            return 1;
        case YUYV422:
        case YCbCr422:
            return 2;
        default:
            fatalError("Bitmap::getBytesPerPixel(): Unknown format.");
            return 0;
    }
}

int Bitmap::getLineLen() const
{
    if (m_PF == YCbCr411) {
        return int(m_Size.x*1.5);
    } else {
        return m_Size.x*getBytesPerPixel();
    }
}

int Bitmap::getMemNeeded() const
{
    // This assumes a positive value for stride.
    return m_Stride*m_Size.y;
}

bool Bitmap::hasAlpha() const
{
    return (m_PF == B8G8R8A8 || m_PF == R8G8B8A8 || m_PF == A8B8G8R8 ||
            m_PF == A8R8G8B8);
}

HistogramPtr Bitmap::getHistogram(int Stride) const
{
    assert (m_PF == I8);
    HistogramPtr pHist(new Histogram(256,0));
    const unsigned char * pSrcLine = m_pBits;
    for (int y=0; y < m_Size.y; y+=Stride) {
        const unsigned char * pSrc = pSrcLine;
        for (int x=0; x<m_Size.x; x+=Stride) {
            (*pHist)[(*pSrc)]++;
            pSrc+=Stride;
        }
        pSrcLine += m_Stride*Stride;
    }
    return pHist;
}

void Bitmap::getMinMax(int Stride, int& min, int& max) const
{
    assert (m_PF == I8);
    const unsigned char * pSrcLine = m_pBits;
    min = 255;
    max = 0;
    for (int y=0; y < m_Size.y; y+=Stride) {
        const unsigned char * pSrc = pSrcLine;
        for (int x=0; x<m_Size.x; x+=Stride) {
            if (*pSrc < min) {
                min = *pSrc;
            }
            if (*pSrc > max) {
                max = *pSrc;
            }
            pSrc+=Stride;
        }
        pSrcLine += m_Stride*Stride;
    }
}

bool Bitmap::operator ==(const Bitmap & otherBmp)
{
    // We allow Name, Stride and bOwnsBits to be different here, since we're looking for
    // equal value only.
    if (m_Size != otherBmp.m_Size || m_PF != otherBmp.m_PF)
    {
        return false;
    }

    const unsigned char * pSrc = otherBmp.getPixels();
    unsigned char * pDest = m_pBits;
    int LineLen = getLineLen();
    for (int y=0; y<getSize().y; ++y) {
        switch(m_PF) {
            case R8G8B8X8:
            case B8G8R8X8:
                for (int x=0; x<getSize().x; ++x) {
                    const unsigned char * pSrcPixel = pSrc+x*getBytesPerPixel();
                    unsigned char * pDestPixel = pDest+x*getBytesPerPixel();
                    if (*((Pixel24*)(pDestPixel)) != *((Pixel24*)(pSrcPixel))) {
                        return false;
                    }
                }
                break;
            default:
                if (memcmp(pDest, pSrc, LineLen) != 0) {
                    return false;
                }
        }
        pDest += m_Stride;
        pSrc += otherBmp.getStride();
    }
    return true;
}

Bitmap * Bitmap::subtract(const Bitmap *pOtherBmp)
{
    assert(m_PF == pOtherBmp->getPixelFormat() && m_Size == pOtherBmp->getSize());
    Bitmap * pResultBmp = new Bitmap(m_Size, m_PF);
    const unsigned char * pSrcLine1 = pOtherBmp->getPixels();
    const unsigned char * pSrcLine2 = m_pBits;
    unsigned char * pDestLine = pResultBmp->getPixels();
    for (int y=0; y<getSize().y; ++y) {
        switch(m_PF) {
            case I16: 
                {
                    const unsigned short * pSrc1 = (const unsigned short *)pSrcLine1;
                    const unsigned short * pSrc2 = (const unsigned short *)pSrcLine2;
                    unsigned short * pDest= (unsigned short *)pDestLine;
                    for (int x=0; x<m_Size.x; ++x) {
                        *pDest = abs(*pSrc1-*pSrc2);
                        pSrc1++;
                        pSrc2++;
                        pDest++;
                    }
                }
                break;
            default:
                {
                    const unsigned char * pSrc1 = pSrcLine1;
                    const unsigned char * pSrc2 = pSrcLine2;
                    unsigned char * pDest= pDestLine;
                    for (int x=0; x<getLineLen(); ++x) {
                        *pDest = abs(*pSrc1-*pSrc2);
                        pSrc1++;
                        pSrc2++;
                        pDest++;
                    }
                }
        }
        pSrcLine1 += getStride();
        pSrcLine2 += pOtherBmp->getStride();
        pDestLine += pResultBmp->getStride();
    }
    return pResultBmp;
}

double Bitmap::getAvg() const
{
    double sum = 0;
    unsigned char * pSrc = m_pBits;
    int componentsPerPixel = getBytesPerPixel();
    for (int y=0; y<getSize().y; ++y) {
        switch(m_PF) {
            case R8G8B8X8:
            case B8G8R8X8:
                {
                    Pixel32 * pSrcPixel = (Pixel32 *)pSrc;
                    for (int x=0; x<m_Size.x; ++x) {
                        sum += pSrcPixel->getR()+pSrcPixel->getG()+pSrcPixel->getB();
                        pSrcPixel++;
                    }
                    componentsPerPixel = 3;
                }
                break;
            case I16:
                {
                    componentsPerPixel = 1;
                    unsigned short * pSrcPixel = (unsigned short *)pSrc;
                    for (int x=0; x<m_Size.x; ++x) {
                        sum += *pSrcPixel;
                        pSrcPixel++;
                    }
                }
                break;
            default:
                {
                    unsigned char * pSrcComponent = pSrc;
                    for (int x=0; x<getLineLen(); ++x) {
                        sum += *pSrcComponent;
                        pSrcComponent++;
                    }
                }
        }
        pSrc += m_Stride;
    }
    sum /= componentsPerPixel;
    return sum/(getSize().x*getSize().y);
}

double Bitmap::getStdDev() const
{
    double average = getAvg();
    double sum = 0;

    unsigned char * pSrc = m_pBits;
    int componentsPerPixel = getBytesPerPixel();
    for (int y=0; y<getSize().y; ++y) {
        switch(m_PF) {
            case R8G8B8X8:
            case B8G8R8X8:
                {
                    componentsPerPixel = 3;
                    Pixel32 * pSrcPixel = (Pixel32 *)pSrc;
                    for (int x=0; x<m_Size.x; ++x) {
                        sum += sqr(pSrcPixel->getR()-average);
                        sum += sqr(pSrcPixel->getG()-average);
                        sum += sqr(pSrcPixel->getB()-average);
                        pSrcPixel++;
                    }
                }
                break;
            case I16:
                {
                    componentsPerPixel = 1;
                    unsigned short * pSrcPixel = (unsigned short *)pSrc;
                    for (int x=0; x<m_Size.x; ++x) {
                        sum += sqr(*pSrcPixel-average);
                        pSrcPixel++;
                    }
                }
                break;
            default:
                {
                    unsigned char * pSrcComponent = pSrc;
                    for (int x=0; x<getLineLen(); ++x) {
                        sum += sqr(*pSrcComponent-average);
                        pSrcComponent++;
                    }
                }
        }
        pSrc += m_Stride;
    }
    sum /= componentsPerPixel;
    sum /= (getSize().x*getSize().y);
    return sqrt(sum);
}

void Bitmap::dump(bool bDumpPixels) const
{
    cerr << "Bitmap: " << m_sName << endl;
    cerr << "  m_Size: " << m_Size.x << "x" << m_Size.y << endl;
    cerr << "  m_Stride: " << m_Stride << endl;
    cerr << "  m_PF: " << getPixelFormatString(m_PF) << endl;
    cerr << "  m_pBits: " << (void *)m_pBits << endl;
    cerr << "  m_bOwnsBits: " << m_bOwnsBits << endl;
    if (bDumpPixels) {
        cerr << "  Pixel data: " << endl;
        for (int y=0; y<m_Size.y; ++y) {
            unsigned char * pLine = m_pBits+m_Stride*y;
            cerr << "    ";
            for (int x=0; x<m_Size.x; ++x) {
                unsigned char * pPixel = pLine+getBytesPerPixel()*x;
                cerr << "[";
                for (int i=0; i<getBytesPerPixel(); ++i) {
                    cerr << hex << setw(2) << (int)(pPixel[i]) << " ";
                }
                cerr << "]";
            }
            cerr << endl;
        }
        cerr << dec;
    }
}

void Bitmap::initWithData(unsigned char * pBits, int Stride, bool bCopyBits)
{
//    cerr << "Bitmap::initWithData()" << endl;
    if (m_PF == YCbCr422 || m_PF == YCbCr420p) {
        if (m_Size.x%2 == 1) {
            AVG_TRACE(Logger::WARNING, "Odd size for YCbCr bitmap.");
            m_Size.x++;
        }
        if (m_Size.y%2 == 1) {
            AVG_TRACE(Logger::WARNING, "Odd size for YCbCr bitmap.");
            m_Size.y++;
        }
        if (m_Size.x%2 == 1 || m_Size.y%2 == 1) {
            AVG_TRACE(Logger::ERROR, "Odd size for YCbCr bitmap.");
        }
    }
    if (bCopyBits) {
        allocBits();
        if (m_Stride == Stride && Stride == (m_Size.x*getBytesPerPixel())) {
            memcpy(m_pBits, pBits, Stride*m_Size.y);
        } else {
            for (int y=0; y<m_Size.y; ++y) {
                memcpy(m_pBits+m_Stride*y, pBits+Stride*y, Stride);
            }
        }
        m_bOwnsBits = true;
    } else {
        m_pBits = pBits;
        m_Stride = Stride;
        m_bOwnsBits = false;
    }
}

void Bitmap::allocBits()
{
    assert(!m_pBits);
//    cerr << "Bitmap::allocBits():" << m_Size <<  endl;
    m_Stride = getLineLen();
    if (m_PF == YCbCr422 || m_PF == YCbCr420p) {
        if (m_Size.x%2 == 1) {
            AVG_TRACE(Logger::WARNING, "Odd width for YCbCr bitmap.");
            m_Size.x++;
        }
        if (m_Size.y%2 == 1) {
            AVG_TRACE(Logger::WARNING, "Odd height for YCbCr bitmap.");
            m_Size.y++;
        }
        //XXX: We allocate more than nessesary here because ffmpeg seems to
        // overwrite memory after the bits - probably during yuv conversion.
        // Yuck.
        m_pBits = new unsigned char[(m_Stride+1)*(m_Size.y+1)];
    } else {
        m_pBits = new unsigned char[m_Stride*m_Size.y];
    }
}

inline void YUVtoBGR32Pixel(Pixel32* pDest, int y, int u, int v)
{
    // u = Cb, v = Cr
    int u1 = u - 128;
    int v1 = v - 128;
    int tempy = 298*(y-16);
    int b = (tempy + 516 * u1           ) >> 8;
    int g = (tempy - 100 * u1 - 208 * v1) >> 8;
    int r = (tempy            + 409 * v1) >> 8;

    if (b<0) b = 0;
    if (b>255) b= 255;
    if (g<0) g = 0;
    if (g>255) g= 255;
    if (r<0) r = 0;
    if (r>255) r= 255;
    pDest->set(b,g,r,255);
}

void YUYV422toBGR32Line(const unsigned char* pSrcLine, Pixel32 * pDestLine, int Width)
{
    Pixel32 * pDestPixel = pDestLine;
    
    // We need the previous and next values to interpolate between the
    // sampled u and v values.
    int v = *(pSrcLine+3);
    int v0; // Previous v
    int u;
    int u1; // Next u;
    const unsigned char * pSrcPixels = pSrcLine;

    for (int x = 0; x < Width/2-1; x++) {
        // Two pixels at a time.
        // Source format is YUYV.
        u = pSrcPixels[1];
        v0 = v;
        v = pSrcPixels[3];
        u1 = pSrcPixels[5];

        YUVtoBGR32Pixel(pDestPixel, pSrcPixels[0], u, (v0+v)/2);
        YUVtoBGR32Pixel(pDestPixel+1, pSrcPixels[2], (u+u1)/2, v);

        pSrcPixels+=4;
        pDestPixel+=2;
    }
    // Last pixels.
    u = pSrcPixels[1];
    v0 = v;
    v = pSrcPixels[3];
    YUVtoBGR32Pixel(pDestPixel, pSrcPixels[0], u, v0/2+v/2);
    YUVtoBGR32Pixel(pDestPixel+1, pSrcPixels[2], u, v);
}
 
void UYVY422toBGR32Line(const unsigned char* pSrcLine, Pixel32 * pDestLine, int Width)
{
    Pixel32 * pDestPixel = pDestLine;
    
    // We need the previous and next values to interpolate between the
    // sampled u and v values.
    int v = *(pSrcLine+2);
    int v0; // Previous v
    int u;
    int u1; // Next u;
    const unsigned char * pSrcPixels = pSrcLine;

    for (int x = 0; x < Width/2-1; x++) {
        // Two pixels at a time.
        // Source format is UYVY.
        u = pSrcPixels[0];
        v0 = v;
        v = pSrcPixels[2];
        u1 = pSrcPixels[4];

        YUVtoBGR32Pixel(pDestPixel, pSrcPixels[1], u, (v0+v)/2);
        YUVtoBGR32Pixel(pDestPixel+1, pSrcPixels[3], (u+u1)/2, v);

        pSrcPixels+=4;
        pDestPixel+=2;
    }
    // Last pixels.
    u = pSrcPixels[0];
    v0 = v;
    v = pSrcPixels[2];
    YUVtoBGR32Pixel(pDestPixel, pSrcPixels[1], u, v0/2+v/2);
    YUVtoBGR32Pixel(pDestPixel+1, pSrcPixels[3], u, v);
}
 
void YUV411toBGR32Line(const unsigned char* pSrcLine, Pixel32 * pDestLine, int Width)
{
    Pixel32 * pDestPixel = pDestLine;
    
    // We need the previous and next values to interpolate between the
    // sampled u and v values.
    int v = *(pSrcLine+3);
    int v0; // Previous v
    int v1; // Next v;
    int u;
    int u1; // Next u;
    const unsigned char * pSrcPixels = pSrcLine;

    for (int x = 0; x < Width/4; x++) {
        // Four pixels at a time.
        // Source format is UYYVYY.
        u = pSrcPixels[0];
        v0 = v;
        v = pSrcPixels[3];

        if (x < Width/4-1) {
            u1 = pSrcPixels[6];
            v1 = pSrcPixels[9];
        } else {
            u1 = u;
            v1 = v;
        }

        YUVtoBGR32Pixel(pDestPixel, pSrcPixels[1], u, v0/2+v/2);
        YUVtoBGR32Pixel(pDestPixel+1, pSrcPixels[2], (u*3)/4+u1/4, v0/4+(v*3)/4);
        YUVtoBGR32Pixel(pDestPixel+2, pSrcPixels[4], u/2+u1/2, v);
        YUVtoBGR32Pixel(pDestPixel+3, pSrcPixels[5], u/4+(u1*3)/4, (v*3)/4+v1/4);

        pSrcPixels+=6;
        pDestPixel+=4;
    }
}

void Bitmap::YCbCrtoBGR(const Bitmap& Orig)
{
    assert(m_PF==B8G8R8X8);
    const unsigned char * pSrc = Orig.getPixels();
    Pixel32 * pDest = (Pixel32*)m_pBits;
    int Height = min(Orig.getSize().y, m_Size.y);
    int Width = min(Orig.getSize().x, m_Size.x);
    int StrideInPixels = m_Stride/getBytesPerPixel();
    int TotalPixels = Width * Height;
    switch(Orig.m_PF) {
        case YCbCr422:
            for (int y=0; y<Height; ++y) {
                UYVY422toBGR32Line(pSrc, pDest, Width);
                pDest += StrideInPixels;
                pSrc += Orig.getStride();
            }
            break;
        case YUYV422:
            for (int y=0; y<Height; ++y) {
                YUYV422toBGR32Line(pSrc, pDest, Width);
                pDest += StrideInPixels;
                pSrc += Orig.getStride();
            }
            break;
        case YCbCr411:
            for (int y=0; y<Height; ++y) {
                YUV411toBGR32Line(pSrc, pDest, Width);
                pDest += StrideInPixels;
                pSrc += Orig.getStride();
            }
            break;
        case YCbCr420p: // this is a planar format
            for (int y=0; y<Height; ++y) {
                // Fast conversion algo with no interpolation
                // TODO: interlacing aware interpolation
                int lineoffs = Width * y;
                int emioffs = (y/2) * (Width/2);
                for (int x=0; x<Width; ++x) {
                    // planes order: Y(1:1), Cb(1:4), Cr(1:4).
                    // Offsets: Cb @ imgSize, Cr @ 5/4 * imgSize
                    YUVtoBGR32Pixel(pDest + lineoffs + x,
                        pSrc[lineoffs + x],
                        pSrc[emioffs + x/2 + TotalPixels],
                        pSrc[emioffs + x/2 + TotalPixels + TotalPixels/4]);
                }
            }
            break;
        default:
            // This routine shouldn't be called with other pixel formats.
            assert(false);
    }
}
    
void YUYV422toI8Line(const unsigned char* pSrcLine, unsigned char * pDestLine, int Width)
{
    const unsigned char * pSrc = pSrcLine;
    unsigned char * pDest = pDestLine;
    
    for (int x = 0; x < Width; x++) {
        *pDest = *pSrc;
        pDest++;
        pSrc+=2;
    }
}
 
void YUV411toI8Line(const unsigned char* pSrcLine, unsigned char * pDestLine, int Width)
{
    const unsigned char * pSrc = pSrcLine;
    unsigned char * pDest = pDestLine;
    
    for (int x = 0; x < Width/2; x++) {
        *pDest++ = *pSrc++;
        *pDest++ = *pSrc++;
        pSrc++;
    }
}
 
void Bitmap::YCbCrtoI8(const Bitmap& Orig)
{
    assert(m_PF==I8);
    const unsigned char * pSrc = Orig.getPixels();
    unsigned char * pDest = m_pBits;
    int Height = min(Orig.getSize().y, m_Size.y);
    int Width = min(Orig.getSize().x, m_Size.x);
    switch(Orig.m_PF) {
        case YCbCr422:
            for (int y=0; y<Height; ++y) {
                // src shifted by one byte to account for UYVY to YUYV 
                // difference in pixel order.
                YUYV422toI8Line(pSrc+1, pDest, Width);
                pDest += m_Stride;
                pSrc += Orig.getStride();
            }
            break;
        case YUYV422:
            for (int y=0; y<Height; ++y) {
                YUYV422toI8Line(pSrc, pDest, Width);
                pDest += m_Stride;
                pSrc += Orig.getStride();
            }
            break;
        case YCbCr411:
            for (int y=0; y<Height; ++y) {
                YUV411toI8Line(pSrc, pDest, Width);
                pDest += m_Stride;
                pSrc += Orig.getStride();
            }
            break;
        case YCbCr420p: 
            // Just take the first plane.
            for (int y=0; y<Height; ++y) {
                memcpy(pDest, pSrc, m_Stride);
                pDest += m_Stride;
                pSrc += Orig.getStride();
            }
            break;
        default:
            // This routine shouldn't be called with other pixel formats.
            assert(false);
    }
}

void Bitmap::I16toI8(const Bitmap& Orig)
{
    assert(m_PF == I8);
    assert(Orig.getPixelFormat() == I16);
    const unsigned short * pSrc = (const unsigned short *)Orig.getPixels();
    unsigned char * pDest = m_pBits;
    int Height = min(Orig.getSize().y, m_Size.y);
    int Width = min(Orig.getSize().x, m_Size.x);
    int SrcStrideInPixels = Orig.getStride()/Orig.getBytesPerPixel();
    for (int y=0; y<Height; ++y) {
        const unsigned short * pSrcPixel = pSrc;
        unsigned char * pDestPixel = pDest;
        for (int x=0; x<Width; ++x) {
            *pDestPixel++ = *pSrcPixel++ >> 8;
        }
        pDest += m_Stride;
        pSrc += SrcStrideInPixels;
    }
}

void Bitmap::I8toI16(const Bitmap& Orig)
{
    assert(m_PF == I16);
    assert(Orig.getPixelFormat() == I8);
    const unsigned char * pSrc = Orig.getPixels();
    unsigned short * pDest = (unsigned short *)m_pBits;
    int Height = min(Orig.getSize().y, m_Size.y);
    int Width = min(Orig.getSize().x, m_Size.x);
    int DestStrideInPixels = m_Stride/getBytesPerPixel();
    for (int y=0; y<Height; ++y) {
        const unsigned char * pSrcPixel = pSrc;
        unsigned short * pDestPixel = pDest;
        for (int x=0; x<Width; ++x) {
            *pDestPixel++ = *pSrcPixel++ << 8;
        }
        pDest += DestStrideInPixels;
        pSrc += Orig.getStride();
    }
}

void Bitmap::I8toRGB(const Bitmap& Orig)
{
    assert(getBytesPerPixel() == 4 || getBytesPerPixel() == 3);
    assert(Orig.getPixelFormat() == I8);
    const unsigned char * pSrc = Orig.getPixels();
    int Height = min(Orig.getSize().y, m_Size.y);
    int Width = min(Orig.getSize().x, m_Size.x);
    if (getBytesPerPixel() == 4) {
        unsigned int * pDest = (unsigned int *)m_pBits;
        int DestStrideInPixels = m_Stride/getBytesPerPixel();
        for (int y=0; y<Height; ++y) {
            const unsigned char * pSrcPixel = pSrc;
            unsigned int * pDestPixel = pDest;
            for (int x=0; x<Width; ++x) {
                *pDestPixel = (((((255 << 8)+(*pSrcPixel)) << 8)+
                        *pSrcPixel) << 8) +(*pSrcPixel);
                pDestPixel ++;
                pSrcPixel++;
            }
            pDest += DestStrideInPixels;
            pSrc += Orig.getStride();
        }
    } else {
        unsigned char * pDest = m_pBits;
        for (int y=0; y<Height; ++y) {
            const unsigned char * pSrcPixel = pSrc;
            unsigned char * pDestPixel = pDest;
            for (int x=0; x<Width; ++x) {
                *pDestPixel++ = *pSrcPixel;
                *pDestPixel++ = *pSrcPixel;
                *pDestPixel++ = *pSrcPixel;
                pSrcPixel++;
            }
            pDest += getStride();
            pSrc += Orig.getStride();
        }
    }
}


// Nearest Neighbour Bayer Pattern de-mosaicking
// Code has been taken and adapted from libdc1394 Bayer conversion
// TODO: adapt it for RGB24, not just for RGB32
// TODO: add more CFA patterns (now only the GBRG is defined and used)
void Bitmap::BY8toRGBNearest(const Bitmap& Orig)
{
    assert(getBytesPerPixel() == 4);
    assert(Orig.getPixelFormat() == BAYER8_GBRG);

    int Height = min(Orig.getSize().y, m_Size.y);
    int Width = min(Orig.getSize().x, m_Size.x);

    const int SrcStride = Width;
    const int DestStride = 4 * Width;
    int width = Width;
    int height = Height;

    // CFA Pattern selection: BGGR: blue=-1, swg=0; GRBG: blue=1, swg=1
    // Assuming GBRG
    int blue = 1;
    int greenFirst = 1;

    const unsigned char *pSrcPixel = Orig.getPixels();
    unsigned char *pDestPixel = (unsigned char *) getPixels();

    pDestPixel += 1;
    width -= 1;
    height -= 1;

    while (--height) {

        const unsigned char *pSrcEndBoundary = pSrcPixel + width;

        if (greenFirst) {
            pDestPixel[-blue] = pSrcPixel[1];
            pDestPixel[0] = pSrcPixel[SrcStride + 1];
            pDestPixel[blue] = pSrcPixel[SrcStride];
            pDestPixel[2] = 255; // Alpha channel
            ++pSrcPixel;
            pDestPixel += 4;
        }

        if (blue > 0) {
            while (pSrcPixel <= pSrcEndBoundary - 2) {
                pDestPixel[-1] = pSrcPixel[0];
                pDestPixel[0] = pSrcPixel[1];
                pDestPixel[1] = pSrcPixel[SrcStride + 1];
                pDestPixel[2] = 255; // Alpha channel

                pDestPixel[3] = pSrcPixel[2];
                pDestPixel[4] = pSrcPixel[SrcStride + 2];
                pDestPixel[5] = pSrcPixel[SrcStride + 1];
                pDestPixel[6] = 255; // Alpha channel
                
                pSrcPixel += 2;
                pDestPixel += 8;
            }
        } else {
            while (pSrcPixel <= pSrcEndBoundary - 2) {
                pDestPixel[1] = pSrcPixel[0];
                pDestPixel[0] = pSrcPixel[1];
                pDestPixel[-1] = pSrcPixel[SrcStride + 1];
                
                pDestPixel[6] = 255; // Alpha channel
                pDestPixel[5] = pSrcPixel[2];
                pDestPixel[4] = pSrcPixel[SrcStride + 2];
                pDestPixel[3] = pSrcPixel[SrcStride + 1];
                pDestPixel[2] = 255; // Alpha channel

                pSrcPixel += 2;
                pDestPixel += 8;
            }
        }

        if (pSrcPixel < pSrcEndBoundary) {
            pDestPixel[-blue] = pSrcPixel[0];
            pDestPixel[0] = pSrcPixel[1];
            pDestPixel[blue] = pSrcPixel[SrcStride + 1];
            pDestPixel[2] = 255; // Alpha channel
            ++pSrcPixel;
            pDestPixel += 4;
        }

        pSrcPixel -= width;
        pDestPixel -= width * 4;

        blue = -blue;
        greenFirst = !greenFirst;

        pSrcPixel += SrcStride;
        pDestPixel += DestStride;
    }
}

// Bilinear Bayer Pattern de-mosaicking
// Code has been taken and adapted from libdc1394 Bayer conversion
// Original source is OpenCV Bayer pattern decoding
// TODO: adapt it for RGB24, not just for RGB32
// TODO: add more CFA patterns (now only the GBRG is defined and used)
void Bitmap::BY8toRGBBilinear(const Bitmap& Orig)
{
    assert(getBytesPerPixel() == 4);
    assert(Orig.getPixelFormat() == BAYER8_GBRG);

    int Height = min(Orig.getSize().y, m_Size.y);
    int Width = min(Orig.getSize().x, m_Size.x);

    const int SrcStride = Width;
    const int doubleSrcStride = SrcStride * 2;
    const int DestStride = 4 * Width;
    int width = Width;
    int height = Height;

    // CFA Pattern selection: BGGR: blue=-1, greenFirst=0; GRBG: blue=1, greenFirst=1
    // Assuming GBRG
    int blue = 1;
    int greenFirst = 1;

    const unsigned char *pSrcPixel = Orig.getPixels();
    unsigned char *pDestPixel = (unsigned char *) getPixels();

    pDestPixel += DestStride + 4 + 1;
    height -= 2;
    width -= 2;

    while(height--) {
        int t0, t1;
        const unsigned char *pSrcEndBoundary = pSrcPixel + width;

        if (greenFirst) {
            t0 = (pSrcPixel[1] + pSrcPixel[doubleSrcStride + 1] + 1) >> 1;
            t1 = (pSrcPixel[SrcStride] + pSrcPixel[SrcStride + 2] + 1) >> 1;
            pDestPixel[-blue] = (unsigned char) t0;
            pDestPixel[0] = pSrcPixel[SrcStride + 1];
            pDestPixel[blue] = (unsigned char) t1;
            pDestPixel[2] = 255; // Alpha channel
            ++pSrcPixel;
            pDestPixel += 4;
        }
                
        if (blue > 0) {
            while(pSrcPixel <= pSrcEndBoundary - 2) {
                t0 = (pSrcPixel[0] + pSrcPixel[2] + pSrcPixel[doubleSrcStride] +
                      pSrcPixel[doubleSrcStride + 2] + 2) >> 2;
                t1 = (pSrcPixel[1] + pSrcPixel[SrcStride] +
                      pSrcPixel[SrcStride + 2] + pSrcPixel[doubleSrcStride + 1] +
                      2) >> 2;
                pDestPixel[-1] = (unsigned char) t0;
                pDestPixel[0] = (unsigned char) t1;
                pDestPixel[1] = pSrcPixel[SrcStride + 1];
                pDestPixel[2] = 255; // Alpha channel

                t0 = (pSrcPixel[2] + pSrcPixel[doubleSrcStride + 2] + 1) >> 1;
                t1 = (pSrcPixel[SrcStride + 1] + pSrcPixel[SrcStride + 3] +
                      1) >> 1;
                pDestPixel[3] = (unsigned char) t0;
                pDestPixel[4] = pSrcPixel[SrcStride + 2];
                pDestPixel[5] = (unsigned char) t1;
                pDestPixel[6] = 255; // Alpha channel
                
                pSrcPixel += 2;
                pDestPixel += 8;
            }
        } else {
            while(pSrcPixel <= pSrcEndBoundary - 2) {
                t0 = (pSrcPixel[0] + pSrcPixel[2] + pSrcPixel[doubleSrcStride] +
                      pSrcPixel[doubleSrcStride + 2] + 2) >> 2;
                t1 = (pSrcPixel[1] + pSrcPixel[SrcStride] +
                      pSrcPixel[SrcStride + 2] + pSrcPixel[doubleSrcStride + 1] +
                      2) >> 2;
                pDestPixel[1] = (unsigned char) t0;
                pDestPixel[0] = (unsigned char) t1;
                pDestPixel[-1] = pSrcPixel[SrcStride + 1];
                pDestPixel[2] = 255; // Alpha channel

                t0 = (pSrcPixel[2] + pSrcPixel[doubleSrcStride + 2] + 1) >> 1;
                t1 = (pSrcPixel[SrcStride + 1] + pSrcPixel[SrcStride + 3] +
                      1) >> 1;
                pDestPixel[5] = (unsigned char) t0;
                pDestPixel[4] = pSrcPixel[SrcStride + 2];
                pDestPixel[3] = (unsigned char) t1;
                pDestPixel[6] = 255; // Alpha channel
                
                pSrcPixel += 2;
                pDestPixel += 8;
            }
        }

        if (pSrcPixel < pSrcEndBoundary) {
            t0 = (pSrcPixel[0] + pSrcPixel[2] + pSrcPixel[doubleSrcStride] +
                  pSrcPixel[doubleSrcStride + 2] + 2) >> 2;
            t1 = (pSrcPixel[1] + pSrcPixel[SrcStride] +
                  pSrcPixel[SrcStride + 2] + pSrcPixel[doubleSrcStride + 1] +
                  2) >> 2;
            pDestPixel[-blue] = (unsigned char) t0;
            pDestPixel[0] = (unsigned char) t1;
            pDestPixel[blue] = pSrcPixel[SrcStride + 1];
            pDestPixel[2] = 255; // Alpha channel
            pSrcPixel++;
            pDestPixel += 4;
        }

        pSrcPixel -= width;
        pDestPixel -= width * 4;

        blue = -blue;
        greenFirst = !greenFirst;
        
        pSrcPixel += SrcStride;
        pDestPixel += DestStride;
    }
}

template<class DestPixel, class SrcPixel>
void createTrueColorCopy(Bitmap& Dest, const Bitmap & Src)
{
    SrcPixel * pSrcLine = (SrcPixel*) Src.getPixels();
    DestPixel * pDestLine = (DestPixel*) Dest.getPixels();
    int Height = min(Src.getSize().y, Dest.getSize().y);
    int Width = min(Src.getSize().x, Dest.getSize().x);
    for (int y = 0; y<Height; ++y) {
        SrcPixel * pSrcPixel = pSrcLine;
        DestPixel * pDestPixel = pDestLine;
        for (int x = 0; x < Width; ++x) {
            *pDestPixel = *pSrcPixel;
            ++pSrcPixel;
            ++pDestPixel;
        }
        pSrcLine = (SrcPixel *)((unsigned char *)pSrcLine + Src.getStride());
        pDestLine = (DestPixel *)((unsigned char *)pDestLine + Dest.getStride());
    }
}

template<>
void createTrueColorCopy<Pixel32, Pixel8>(Bitmap& Dest, const Bitmap & Src)
{
    const unsigned char * pSrcLine = Src.getPixels();
    unsigned char * pDestLine = Dest.getPixels();
    int Height = min(Src.getSize().y, Dest.getSize().y);
    int Width = min(Src.getSize().x, Dest.getSize().x);
    int SrcStride = Src.getStride();
    int DestStride = Dest.getStride();
    for (int y = 0; y<Height; ++y) {
        const unsigned char * pSrcPixel = pSrcLine;
        unsigned char * pDestPixel = pDestLine;
        for (int x = 0; x < Width; ++x) {
            pDestPixel[0] =
            pDestPixel[1] =
            pDestPixel[2] = *pSrcPixel;
            pDestPixel[3] = 255;
            ++pSrcPixel;
            pDestPixel+=4;
        }
        pSrcLine = pSrcLine + SrcStride;
        pDestLine = pDestLine + DestStride;
    }
}

template<class Pixel>
void createTrueColorCopy(Bitmap& Dest, const Bitmap & Src)
{
    switch(Src.getPixelFormat()) {
        case B8G8R8A8:
        case B8G8R8X8:
        case A8B8G8R8:
        case X8B8G8R8:
        case R8G8B8A8:
        case R8G8B8X8:
        case A8R8G8B8:
        case X8R8G8B8:
            createTrueColorCopy<Pixel, Pixel32>(Dest, Src);
            break;
        case B8G8R8:
        case R8G8B8:
            createTrueColorCopy<Pixel, Pixel24>(Dest, Src);
            break;
        case B5G6R5:
        case R5G6B5:
            createTrueColorCopy<Pixel, Pixel16>(Dest, Src);
            break;
        case I8:
        case BAYER8_GBRG:
            createTrueColorCopy<Pixel, Pixel8>(Dest, Src);
            break;
        default:
            // Unimplemented conversion.
            assert(false);
    }
}
    
};
