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

#include "Bitmap.h"
#include "Pixel32.h"
#include "Pixel24.h"
#include "Pixel16.h"
#include "Filter3x3.h"

#include "../base/Exception.h"
#include "../base/Logger.h"

#include <Magick++.h>
#include <assert.h>
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
      m_bOwnsBits(true),
      m_sName(sName)
{
//    cerr << "Bitmap::Bitmap(" << Size << ", " << getPixelFormatString(m_PF) << ", " 
//        << sName << ")" << endl;
    allocBits();
}

Bitmap::Bitmap(IntPoint Size, PixelFormat PF, unsigned char * pBits, 
        int Stride, bool bCopyBits, const std::string& sName)
    : m_Size(Size),
      m_PF(PF),
      m_sName(sName)
{
//    cerr << "Bitmap::Bitmap(" << Size << ", " << getPixelFormatString(m_PF) << ", " 
//        << (void *)pBits << ", " << Stride << ", " << bCopyBits << ", "
//        << sName << ")" << endl;
    initWithData(pBits, Stride, bCopyBits);
}

Bitmap::Bitmap(const Bitmap& Orig)
    : m_Size(Orig.getSize()),
      m_PF(Orig.getPixelFormat()),
      m_bOwnsBits(Orig.m_bOwnsBits),
      m_sName(Orig.getName()+" copy")
{
//    cerr << "Bitmap::Bitmap(Bitmap), Name: " << m_sName << endl;
    initWithData(const_cast<unsigned char *>(Orig.getPixels()), Orig.getStride(), 
            m_bOwnsBits);
}

// Creates a bitmap that is a rectangle in another bitmap. The pixels are
// still owned by the original bitmap.
Bitmap::Bitmap(Bitmap& Orig, const IntRect& Rect)
    : m_Size(Rect.Width(), Rect.Height()),
      m_PF(Orig.getPixelFormat()),
      m_bOwnsBits(false)
{
//    cerr << "Bitmap::Bitmap(Bitmap, " << Rect << "), Name: " << m_sName << endl;
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
    : m_sName(sURI)
{
//    cerr << "Bitmap::Bitmap(" << sURI << ")" << endl;
    Image Img;
    try {
        Img.read(sURI);
    } catch( Magick::WarningCoder &warning ) {
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
        for (int x=0; x<m_Size.x; ++x) {
            *pDestLine = Pixel32(pSrcLine->red, pSrcLine->green, 
                    pSrcLine->blue, 255-pSrcLine->opacity);
            pSrcLine++;
            pDestLine++;
        }
    }
    m_bOwnsBits = true;
}

Bitmap::~Bitmap()
{
//    cerr << "Bitmap::~Bitmap(), Name: " << m_sName << endl;
    if (m_bOwnsBits) {
        delete[] m_pBits;
    }
}

Bitmap &Bitmap::operator= (const Bitmap &Orig)
{
//    cerr << "Bitmap::operator=()" << endl;
    if (this != &Orig) {
        if (m_bOwnsBits) {
            delete[] m_pBits;
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
        int Width = min(Orig.getSize().x, m_Size.x);
        int LineLen = Width*getBytesPerPixel();
        for (int y=0; y<Height; ++y) {
            memcpy(pDest, pSrc, LineLen);
            pDest += m_Stride;
            pSrc += Orig.getStride();
        }
    } else {
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
            default:
                // Unimplemented conversion.
                assert(false);
        }
    }
}

void Bitmap::save(const std::string& sFilename)
{
//    cerr << "Bitmap::save()" << endl;
    string sPF;
    BitmapPtr pBmp;
    // TODO: Not all of these are tested.
    switch(m_PF) {
        case B5G6R5:
            pBmp = BitmapPtr(new Bitmap(m_Size, B8G8R8));
            pBmp->copyPixels(*this);
            sPF = "BGR";
            break;
        case B8G8R8:
            pBmp = BitmapPtr(new Bitmap(*this));
            sPF = "BGR";
            break;
        case B8G8R8A8:
            pBmp = BitmapPtr(new Bitmap(*this));
            sPF = "BGRA";
            break;
        case B8G8R8X8:
            pBmp = BitmapPtr(new Bitmap(m_Size, B8G8R8));
            pBmp->copyPixels(*this);
            sPF = "BGR";
            break;
        case A8B8G8R8:
            pBmp = BitmapPtr(new Bitmap(*this));
            sPF = "ABGR";
            break;
        case X8B8G8R8:
            pBmp = BitmapPtr(new Bitmap(m_Size, B8G8R8));
            pBmp->copyPixels(*this);
            sPF = "BGR";
            break;
        case R5G6B5:
            pBmp = BitmapPtr(new Bitmap(m_Size, R8G8B8));
            pBmp->copyPixels(*this);
            sPF = "RGB";
            break;
        case R8G8B8:
            pBmp = BitmapPtr(new Bitmap(*this));
            sPF = "RGB";
            break;
        case R8G8B8A8:
            pBmp = BitmapPtr(new Bitmap(*this));
            sPF = "RGBA";
            break;
        case R8G8B8X8:
            pBmp = BitmapPtr(new Bitmap(m_Size, R8G8B8, "temp copy"));
            pBmp->copyPixels(*this);
            sPF = "RGB";
            break;
        case A8R8G8B8:
            pBmp = BitmapPtr(new Bitmap(*this));
            sPF = "ARGB";
            break;
        case X8R8G8B8:
            pBmp = BitmapPtr(new Bitmap(m_Size, R8G8B8));
            pBmp->copyPixels(*this);
            sPF = "RGB";
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
    Magick::Image Img(m_Size.x, m_Size.y, sPF, Magick::CharPixel, pBmp->getPixels());
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

std::string Bitmap::getPixelFormatString()
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
        case YCbCr422:
            return "YCbCr422";
        case YCbCr420p:
            return "YCbCr420p";
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
            return 2;
        case I8:
            return 1;
        case YCbCr422:
            return 2;
        default:
            fatalError("Bitmap::getBytesPerPixel(): Unknown format.");
            return 0;
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
    int LineLen = getSize().x*getBytesPerPixel();
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

template<class Pixel>
void lineSubtract(const unsigned char * pSrc, unsigned char * pDest, int lineLen)
{
    Pixel * pSrcPixel = (Pixel *)pSrc;
    Pixel * pDestPixel = (Pixel *)pDest;
    for (int x=0; x<lineLen; ++x) {
        pDestPixel->setR(abs(pSrcPixel->getR()-pDestPixel->getR()));
        pDestPixel->setG(abs(pSrcPixel->getG()-pDestPixel->getG()));
        pDestPixel->setB(abs(pSrcPixel->getB()-pDestPixel->getB()));
        pSrcPixel++;
        pDestPixel++;
    }
}

void Bitmap::subtract(const Bitmap *pOtherBmp)
{
    const unsigned char * pSrc = pOtherBmp->getPixels();
    unsigned char * pDest = m_pBits;
    for (int y=0; y<getSize().y; ++y) {
        switch(m_PF) {
            case R8G8B8X8:
            case B8G8R8X8:
                lineSubtract<Pixel32>(pSrc, pDest, m_Size.x);
                break;
            case R8G8B8:
            case B8G8R8:
                lineSubtract<Pixel24>(pSrc, pDest, m_Size.x);
                break;
            default:
                // Unimplemented.
                assert(false);
        }
        pDest += m_Stride;
        pSrc += pOtherBmp->getStride();
    }
}

template<class Pixel>
int lineBrightPixels(const unsigned char * pSrc, int lineLen)
{
    Pixel * pSrcPixel = (Pixel *)pSrc;
    int Result = 0;
    for (int x=0; x<lineLen; ++x) {
        int Val = pSrcPixel->getR()+pSrcPixel->getG()+pSrcPixel->getB();
        if (Val > 256) {
            Result++;
        }
        pSrcPixel++;
    }
    return Result;
}

int Bitmap::getNumDifferentPixels(const Bitmap & otherBmp)
{
    // We allow Name, Stride and bOwnsBits to be different here, since we're looking for
    // equal value only.
    if (m_Size != otherBmp.m_Size || m_PF != otherBmp.m_PF)
    {
        return false;
    }

    BitmapPtr pTempBmp(new Bitmap(*this));
//    pTempBmp->dump(true);
    pTempBmp->subtract(&otherBmp);
//    pTempBmp->dump(true);
    double Matrix[3][3] = {
            {0.111,0.111,0.111},
            {0.111,0.111,0.111},
            {0.111,0.111,0.111}
    };
    Filter3x3(Matrix).applyInPlace(pTempBmp);

    int NumBrightPixels = 0;
    for (int y = 0; y < m_Size.y-2; y++) {
        const unsigned char * pLine = pTempBmp->getPixels()+y*pTempBmp->getStride();
        
        switch (pTempBmp->getBytesPerPixel()) {
            case 4:
                NumBrightPixels += lineBrightPixels<Pixel32>(pLine, m_Size.x-2);
                break;
            case 3:
                NumBrightPixels += lineBrightPixels<Pixel24>(pLine, m_Size.x-2);
                break;
            default:
                assert(false);
        }
    }
    return NumBrightPixels;
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
    if (m_PF == YCbCr422) {
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
        for (int y=0; y<m_Size.y; ++y) {
            memcpy(m_pBits+m_Stride*y, pBits+Stride*y, Stride);
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
//    cerr << "Bitmap::allocBits():" << m_Size <<  endl;
    
    if (m_PF == YCbCr422) {
        if (m_Size.x%2 == 1) {
            AVG_TRACE(Logger::WARNING, "Odd size for YCbCr bitmap.");
            m_Size.x++;
        }
        if (m_Size.y%2 == 1) {
            AVG_TRACE(Logger::WARNING, "Odd size for YCbCr bitmap.");
            m_Size.y++;
        }
        m_Stride = m_Size.x*getBytesPerPixel();
        //XXX: We allocate more than nessesary here because ffmpeg seems to
        // overwrite memory after the bits - probably during yuv conversion.
        // Yuck.
        m_pBits = new unsigned char[(m_Stride+1)*(m_Size.y+1)];
    } else {
        m_Stride = m_Size.x*getBytesPerPixel();
        m_pBits = new unsigned char[m_Stride*m_Size.y];
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
        default:
            // Unimplemented conversion.
            assert(false);
    }
}
    
};
