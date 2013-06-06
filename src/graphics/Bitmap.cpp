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

#include "Bitmap.h"
#include "Pixel24.h"
#include "Pixel16.h"
#include "Pixel8.h"
#include "Filter3x3.h"

#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/ObjectCounter.h"
#include "../base/StringHelper.h"
#include "../base/MathHelper.h"
#include "../base/FileHelper.h"
#include "../base/OSHelper.h"

#include <gdk-pixbuf/gdk-pixbuf.h>

#include <cstring>
#include <iostream>
#include <iomanip>
#include <stdlib.h>

using namespace std;

namespace avg {

template<class Pixel>
void createTrueColorCopy(Bitmap& destBmp, const Bitmap & srcBmp);

Bitmap::Bitmap(glm::vec2 size, PixelFormat pf, const UTF8String& sName, int stride)
    : m_Size(size),
      m_PF(pf),
      m_pBits(0),
      m_bOwnsBits(true),
      m_sName(sName)
{
    ObjectCounter::get()->incRef(&typeid(*this));
    allocBits(stride);
}

Bitmap::Bitmap(IntPoint size, PixelFormat pf, const UTF8String& sName, int stride)
    : m_Size(size),
      m_PF(pf),
      m_pBits(0),
      m_bOwnsBits(true),
      m_sName(sName)
{
    ObjectCounter::get()->incRef(&typeid(*this));
    allocBits(stride);
}

Bitmap::Bitmap(IntPoint size, PixelFormat pf, unsigned char* pBits, 
        int stride, bool bCopyBits, const UTF8String& sName)
    : m_Size(size),
      m_PF(pf),
      m_pBits(0),
      m_sName(sName)
{
    ObjectCounter::get()->incRef(&typeid(*this));
    initWithData(pBits, stride, bCopyBits);
}

Bitmap::Bitmap(const Bitmap& origBmp)
    : m_Size(origBmp.getSize()),
      m_PF(origBmp.getPixelFormat()),
      m_pBits(0),
      m_bOwnsBits(origBmp.m_bOwnsBits),
      m_sName(origBmp.getName()+" copy")
{
    ObjectCounter::get()->incRef(&typeid(*this));
    initWithData(const_cast<unsigned char *>(origBmp.getPixels()), origBmp.getStride(), 
            m_bOwnsBits);
}

Bitmap::Bitmap(const Bitmap& origBmp, bool bOwnsBits)
    : m_Size(origBmp.getSize()),
      m_PF(origBmp.getPixelFormat()),
      m_pBits(0),
      m_bOwnsBits(bOwnsBits),
      m_sName(origBmp.getName()+" copy")
{
    ObjectCounter::get()->incRef(&typeid(*this));
    initWithData(const_cast<unsigned char *>(origBmp.getPixels()), origBmp.getStride(), 
            m_bOwnsBits);
}

// Creates a bitmap that is a rectangle in another bitmap. The pixels are
// still owned by the original bitmap.
Bitmap::Bitmap(Bitmap& origBmp, const IntRect& rect)
    : m_Size(rect.size()),
      m_PF(origBmp.getPixelFormat()),
      m_pBits(0),
      m_bOwnsBits(false)
{
    ObjectCounter::get()->incRef(&typeid(*this));
    AVG_ASSERT(rect.br.x <= origBmp.getSize().x);
    AVG_ASSERT(rect.br.y <= origBmp.getSize().y);
    AVG_ASSERT(rect.tl.x >= 0 && rect.tl.y >= 0);
    if (!origBmp.getName().empty()) {
        m_sName = origBmp.getName()+" part";
    } else {
        m_sName = "";
    }
    unsigned char * pRegionStart = origBmp.getPixels()+rect.tl.y*origBmp.getStride()+
            rect.tl.x*getBytesPerPixel();
    initWithData(pRegionStart, origBmp.getStride(), false);
}

Bitmap::~Bitmap()
{
    ObjectCounter::get()->decRef(&typeid(*this));
    if (m_bOwnsBits) {
        delete[] m_pBits;
        m_pBits = 0;
    }
}

Bitmap &Bitmap::operator =(const Bitmap& origBmp)
{
    if (this != &origBmp) {
        if (m_bOwnsBits) {
            delete[] m_pBits;
            m_pBits = 0;
        }
        m_Size = origBmp.getSize();
        m_PF = origBmp.getPixelFormat();
        m_bOwnsBits = origBmp.m_bOwnsBits;
        m_sName = origBmp.getName();
        initWithData(const_cast<unsigned char *>(origBmp.getPixels()),
                origBmp.getStride(), m_bOwnsBits);
    }
    return *this;
}

void Bitmap::copyPixels(const Bitmap& origBmp)
{
//    cerr << "Bitmap::copyPixels(): " << getPixelFormatString(origBmp.getPixelFormat())
//            << "->" << getPixelFormatString(m_PF) << endl;
    if (&origBmp == this || origBmp.getPixels() == m_pBits) {
        return;
    }
    if (origBmp.getPixelFormat() == m_PF) {
        const unsigned char * pSrc = origBmp.getPixels();
        unsigned char * pDest = m_pBits;
        int height = min(origBmp.getSize().y, m_Size.y);
        int lineLen = min(origBmp.getLineLen(), getLineLen());
        int srcStride = origBmp.getStride();
        for (int y = 0; y < height; ++y) {
            memcpy(pDest, pSrc, lineLen);
            pDest += m_Stride;
            pSrc += srcStride;
        }
    } else {
        switch (origBmp.getPixelFormat()) {
            case YCbCr422:
            case YUYV422:
            case YCbCr411:
                switch(m_PF) {
                    case B8G8R8X8:
                        YCbCrtoBGR(origBmp);
                        break;
                    case I8:
                    case A8:
                        YCbCrtoI8(origBmp);
                    default: {
                            Bitmap TempBmp(getSize(), B8G8R8X8, "TempColorConversion");
                            TempBmp.YCbCrtoBGR(origBmp);
                            copyPixels(TempBmp);
                        }
                        break;
                }
                break;
            case I16:
                if (m_PF == I8 || m_PF == A8) {
                    I16toI8(origBmp);
                } else {
                    Bitmap TempBmp(getSize(), I8, "TempColorConversion");
                    TempBmp.I16toI8(origBmp);
                    copyPixels(TempBmp);
                }
                break;
            case I8:
            case A8:
                switch(m_PF) {
                    case I16:
                        I8toI16(origBmp);
                        break;
                    case B8G8R8X8:
                    case B8G8R8A8:
                    case R8G8B8X8:
                    case R8G8B8A8:
                    case B8G8R8:
                    case R8G8B8:
                        I8toRGB(origBmp);
                        break;
                    default: 
                        // Unimplemented conversion.
                        AVG_ASSERT(false);
                }
                break;
            case BAYER8_RGGB:
            case BAYER8_GBRG:
            case BAYER8_GRBG:
            case BAYER8_BGGR:
                switch(m_PF) {
                    case I8:
                    case A8:
                        {
                            // Bayer patterns are saved as I8 bitmaps. 
                            // So simply copy that.
                            const unsigned char * pSrc = origBmp.getPixels();
                            unsigned char * pDest = m_pBits;
                            int height = min(origBmp.getSize().y, m_Size.y);
                            int lineLen = min(origBmp.getLineLen(), getLineLen());
                            int srcStride = origBmp.getStride();
                            for (int y = 0; y < height; ++y) {
                                memcpy(pDest, pSrc, lineLen);
                                pDest += m_Stride;
                                pSrc += srcStride;
                            }
                        }
                        break;
                    case B8G8R8X8:
                    case B8G8R8A8:
                    case R8G8B8X8:
                    case R8G8B8A8:
                        BY8toRGBBilinear(origBmp);
                        break;
                    default: 
                        // Unimplemented conversion.
                        AVG_ASSERT(false);
                }
                break;
            case R32G32B32A32F:
                if (getBytesPerPixel() == 4) {
                    FloatRGBAtoByteRGBA(origBmp);
                } else {
                    cerr << "Can't convert " << origBmp.getPixelFormat() << " to " 
                            << getPixelFormat() << endl;
                    AVG_ASSERT(false);
                }
                break;
            default:
                switch(m_PF) {
                    case R32G32B32A32F:
                        if (origBmp.getBytesPerPixel() == 4) {
                            ByteRGBAtoFloatRGBA(origBmp);
                        } else {
                            cerr << "Can't convert " << origBmp.getPixelFormat() <<
                                    " to " << getPixelFormat() << endl;
                            AVG_ASSERT(false);
                        }
                        break;
                    case B8G8R8A8:
                    case B8G8R8X8:
                    case A8B8G8R8:
                    case X8B8G8R8:
                    case R8G8B8A8:
                    case R8G8B8X8:
                    case A8R8G8B8:
                    case X8R8G8B8:
                        createTrueColorCopy<Pixel32>(*this, origBmp);
                        break;
                    case B8G8R8:
                    case R8G8B8:
                        createTrueColorCopy<Pixel24>(*this, origBmp);
                        break;
                    case B5G6R5:
                    case R5G6B5:
                        createTrueColorCopy<Pixel16>(*this, origBmp);
                        break;
                    case I8:
                    case A8:
                        createTrueColorCopy<Pixel8>(*this, origBmp);
                        break;
                    default:
                        // Unimplemented conversion.
                        cerr << "Can't convert " << origBmp.getPixelFormat() << " to " <<
                                getPixelFormat() << endl;
                        AVG_ASSERT(false);
                }
        }
    }
}

#if defined(__SSE__) || defined(_WIN32)
ostream& operator<<(ostream& os, const __m64 &val)
{
    unsigned char * pVal = (unsigned char *)(&val);
    for (int i = 0; i < 8; ++i) {
        os << hex << setw(2) << setfill('0') << int(pVal[i]);
        if (i%2 == 1) {
            os << " ";
        }
        if (i%4 == 3) {
            os << " ";
        }
    }
    return os;
}
#endif

#define YUV_TO_RGB_UNPACK    \
    /* Input: r, g, b contain 4 words each of the u and v inputs for the color */    \
    /* channels. ylo and yhi contain 4 words each of the y input. */    \
\
    /* duplicate u and v channels and add y    \
     * each of r,g, b in the form [s1(16), s2(16), s3(16), s4(16)]    \
     * first interleave, so tmp is [s1(16), s1(16), s2(16), s2(16)]    \
     * then add y, then interleave again    \
     * then pack with saturation, to get the desired output of    \
     *   [s1(8), s1(8), s2(8), s2(8), s3(8), s3(8), s4(8), s4(8)]    \
     */    \
    tmp = _m_punpckhwd(r, r);    \
    tmp = _m_paddsw(tmp, yhi);    \
    tmp2 = _m_punpcklwd(r, r);    \
    tmp2 = _m_paddsw(tmp2, ylo);    \
    r = _m_packuswb(tmp2, tmp);    \
    \
    tmp = _m_punpckhwd(g, g);    \
    tmp2 = _m_punpcklwd(g, g);    \
    tmp = _m_paddsw(tmp, yhi);    \
    tmp2 = _m_paddsw(tmp2, ylo);    \
    g = _m_packuswb(tmp2, tmp);    \
    \
    tmp = _m_punpckhwd(b, b);    \
    tmp2 = _m_punpcklwd(b, b);    \
    tmp = _m_paddsw(tmp, yhi);    \
    tmp2 = _m_paddsw(tmp2, ylo);    \
    b = _m_packuswb(tmp2, tmp);    \
    \
    /* now we have 8 8-bit r, g and b samples.  we want these to be packed    \
     * into 32-bit values.    \
     */    \
    imm = _mm_set1_pi32(0xFFFFFFFF);    \
    tmp = _m_punpcklbw(b, r);    \
    tmp2 = _m_punpcklbw(g, imm);    \
    *o++ = _m_punpcklbw(tmp, tmp2);    \
    *o++ = _m_punpckhbw(tmp, tmp2);    \
    tmp = _m_punpckhbw(b, r);    \
    tmp2 = _m_punpckhbw(g, imm);    \
    *o++ = _m_punpcklbw(tmp, tmp2);    \
    *o++ = _m_punpckhbw(tmp, tmp2);


void Bitmap::copyYUVPixels(const Bitmap& yBmp, const Bitmap& uBmp, const Bitmap& vBmp,
        bool bJPEG)
{
    int height = min(yBmp.getSize().y, m_Size.y);
    int width = min(yBmp.getSize().x, m_Size.x);

    int yStride = yBmp.getStride();
    int uStride = uBmp.getStride();
    int vStride = vBmp.getStride();
    int destStride = m_Stride/getBytesPerPixel();
    Pixel32 * pDestLine = (Pixel32*)m_pBits;

#if defined(__SSE__) || defined(_WIN32)
#pragma pack(16)
    // Original SSE conversion code taken from liboggplay: oggplay_sse_x86.c
    int               i;     
    const unsigned char   * ptry;
    const unsigned char   * ptru;
    const unsigned char   * ptrv;

    register __m64    *o;
    register __m64    y, ylo, yhi;
    register __m64    zero, ut, vt, imm;
    register __m64    r, g, b;
    register __m64    tmp, tmp2;

    zero = _mm_setzero_si64(); 

    ptry = yBmp.getPixels();
    ptru = uBmp.getPixels();
    ptrv = vBmp.getPixels();

    for (i = 0; i < height; i++) {
        int j;
        o = (__m64*)pDestLine;
        pDestLine += destStride;
        if (bJPEG) {
            for (j = 0; j < width; j += 8) {
                // ylo and yhi contain 4 pixels each
                y = *(__m64*)(&(ptry[j]));
                ylo = _m_punpcklbw(y, zero);
               
                yhi = _m_punpckhbw(y, zero);

                ut = _m_from_int(*(int *)(ptru + j/2));
                vt = _m_from_int(*(int *)(ptrv + j/2));

                ut = _m_punpcklbw(ut, zero);
                vt = _m_punpcklbw(vt, zero);

                /* subtract 128 from u and v */ 
                imm = _mm_set1_pi16(128);
                ut = _m_psubw(ut, imm);
                vt = _m_psubw(vt, imm);

                /* transfer and multiply into r, g, b registers */
                imm = _mm_set1_pi16(-44);
                g = _m_pmullw(ut, imm);
                imm = _mm_set1_pi16(113);
                b = _m_pmullw(ut, imm);
                imm = _mm_set1_pi16(179);
                r = _m_pmullw(vt, imm);
                imm = _mm_set1_pi16(-91);
                imm = _m_pmullw(vt, imm);
                g = _m_paddsw(g, imm);

                /* shift r, g and b registers to the right */
                r = _m_psrawi(r, 7);
                g = _m_psrawi(g, 7);
                b = _m_psrawi(b, 6);
                YUV_TO_RGB_UNPACK

            }
        } else {
            for (j = 0; j < width; j += 8) {

                // y' = (298*(y-16))
                // ylo and yhi contain 4 pixels each
                y = *(__m64*)(&(ptry[j]));
                ylo = _m_punpcklbw(y, zero);
                imm = _mm_set1_pi16(16);
                ylo = _m_psubusw(ylo, imm);
                imm = _mm_set1_pi16(149);
                ylo = _m_pmullw(ylo, imm);
                ylo = _mm_srli_pi16(ylo, 7);
               
                yhi = _m_punpckhbw(y, zero);
                imm = _mm_set1_pi16(16);
                yhi = _m_psubusw(yhi, imm);
                imm = _mm_set1_pi16(149);
                yhi = _m_pmullw(yhi, imm);
                yhi = _mm_srli_pi16(yhi, 7);

                ut = _m_from_int(*(int *)(ptru + j/2));
                vt = _m_from_int(*(int *)(ptrv + j/2));

                ut = _m_punpcklbw(ut, zero);
                vt = _m_punpcklbw(vt, zero);

                /* subtract 128 from u and v */ 
                imm = _mm_set1_pi16(128);
                ut = _m_psubw(ut, imm);
                vt = _m_psubw(vt, imm);

                /* transfer and multiply into r, g, b registers */
                imm = _mm_set1_pi16(-50);
                g = _m_pmullw(ut, imm);
                imm = _mm_set1_pi16(129);
                b = _m_pmullw(ut, imm);
                imm = _mm_set1_pi16(204);
                r = _m_pmullw(vt, imm);
                imm = _mm_set1_pi16(-104);
                imm = _m_pmullw(vt, imm);
                g = _m_paddsw(g, imm);

                /* shift r, g and b registers to the right */
                r = _m_psrawi(r, 7);
                g = _m_psrawi(g, 7);
                b = _m_psrawi(b, 6);
                YUV_TO_RGB_UNPACK
            }
        }
        if (i & 0x1) {
            ptru += uStride;
            ptrv += vStride;
        }
        ptry += yStride;
    }
    _m_empty();
#pragma pack()
#else
    AVG_ASSERT(m_PF==B8G8R8X8);
    const unsigned char * pYSrc = yBmp.getPixels();
    const unsigned char * pUSrc = uBmp.getPixels();
    const unsigned char * pVSrc = vBmp.getPixels();

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            YUVtoBGR32Pixel(pDestLine + x, pYSrc[x], pUSrc[x/2], pVSrc[x/2]);
        }
        pDestLine += destStride;
        pYSrc += yStride;
        if (y % 2 == 1) {
            pUSrc += uStride;
            pVSrc += vStride;
        }
    }
#endif
}

void Bitmap::save(const UTF8String& sFilename)
{
    Bitmap* pTempBmp;
    switch (m_PF) {
        case B8G8R8X8:
            pTempBmp = new Bitmap(m_Size, R8G8B8);
            for (int y = 0; y < m_Size.y; y++) {
                unsigned char * pSrcLine = m_pBits+y * m_Stride;
                unsigned char * pDestLine = pTempBmp->getPixels() + 
                        y*pTempBmp->getStride();
                for (int x = 0; x < m_Size.x; x++) { 
                    pDestLine[x*3] = pSrcLine[x*4 + 2];
                    pDestLine[x*3 + 1] = pSrcLine[x*4 + 1];
                    pDestLine[x*3 + 2] = pSrcLine[x*4];
                }
            }
            break;
        case B8G8R8A8:
            pTempBmp = new Bitmap(m_Size, R8G8B8A8);
            for (int y = 0; y < m_Size.y; y++) {
                unsigned char * pSrcLine = m_pBits+y * m_Stride;
                unsigned char * pDestLine = pTempBmp->getPixels() + 
                        y*pTempBmp->getStride();
                for (int x = 0; x < m_Size.x; x++) { 
                    pDestLine[x*4] = pSrcLine[x*4 + 2];
                    pDestLine[x*4 + 1] = pSrcLine[x*4 + 1];
                    pDestLine[x*4 + 2] = pSrcLine[x*4];
                    pDestLine[x*4 + 3] = pSrcLine[x*4+3];
                }
            }
            break;
        default:
            if (hasAlpha()) {
                pTempBmp = new Bitmap(m_Size, R8G8B8A8);
            } else {
                pTempBmp = new Bitmap(m_Size, R8G8B8);
            }
            pTempBmp->copyPixels(*this);
    }
    GdkPixbuf* pPixBuf = gdk_pixbuf_new_from_data(pTempBmp->getPixels(), 
            GDK_COLORSPACE_RGB, pTempBmp->hasAlpha(), 8, m_Size.x, m_Size.y, 
            pTempBmp->getStride(), 0, 0);

    string sExt = getExtension(sFilename);

    GError* pError = 0;
    gboolean bOk = gdk_pixbuf_save(pPixBuf, sFilename.c_str(), sExt.c_str(), &pError, 
            NULL);
    g_object_unref(pPixBuf);
    if (!bOk) {
        string sErr = pError->message;
        g_error_free(pError);
        throw Exception(AVG_ERR_FILEIO, sErr);
    }

    delete pTempBmp;
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

void Bitmap::setPixelFormat(PixelFormat pf)
{
    m_PF = pf;
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

void Bitmap::setPixelsFromString(const string& sPixels)
{
    memcpy(m_pBits, sPixels.c_str(), getMemNeeded());
}


const string& Bitmap::getName() const
{
    return m_sName;
}

bool Bitmap::ownsBits() const
{
    return m_bOwnsBits;
}

int Bitmap::getBytesPerPixel() const
{
    return avg::getBytesPerPixel(m_PF);
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
    return pixelFormatHasAlpha(m_PF);
}

HistogramPtr Bitmap::getHistogram(int stride) const
{
    AVG_ASSERT (getBytesPerPixel() == 1);
    HistogramPtr pHist(new Histogram(256,0));
    const unsigned char * pSrcLine = m_pBits;
    for (int y = 0; y < m_Size.y; y += stride) {
        const unsigned char * pSrc = pSrcLine;
        for (int x = 0; x < m_Size.x; x += stride) {
            (*pHist)[(*pSrc)]++;
            pSrc += stride;
        }
        pSrcLine += m_Stride*stride;
    }
    return pHist;
}

void Bitmap::getMinMax(int stride, int& min, int& max) const
{
    AVG_ASSERT (getBytesPerPixel() == 1);
    const unsigned char * pSrcLine = m_pBits;
    min = 255;
    max = 0;
    for (int y = 0; y < m_Size.y; y += stride) {
        const unsigned char * pSrc = pSrcLine;
        for (int x = 0; x < m_Size.x; x += stride) {
            if (*pSrc < min) {
                min = *pSrc;
            }
            if (*pSrc > max) {
                max = *pSrc;
            }
            pSrc += stride;
        }
        pSrcLine += m_Stride*stride;
    }
}

void Bitmap::setAlpha(const Bitmap& alphaBmp)
{
    AVG_ASSERT(hasAlpha());
    AVG_ASSERT(alphaBmp.getBytesPerPixel() == 1);
    unsigned char * pLine = m_pBits;
    const unsigned char * pAlphaLine = alphaBmp.getPixels();
    for (int y = 0; y < m_Size.y; y++) {
        unsigned char * pPixel = pLine;
        const unsigned char * pAlphaPixel = pAlphaLine;
        for (int x = 0; x < m_Size.x; x++) {
            pPixel[ALPHAPOS] = *pAlphaPixel;
            pPixel+=4;
            pAlphaPixel++;
        }
        pLine += m_Stride;
        pAlphaLine += alphaBmp.getStride();
    }
}

Pixel32 Bitmap::getPythonPixel(const glm::vec2& pos)
{
    IntPoint intPos(pos);
    if (intPos.x < 0 || intPos.y < 0 || intPos.x >= m_Size.x || intPos.y >= m_Size.y) {
        stringstream ss;
        ss << "Bitmap.getPixel(): intPos " << intPos << 
                " is out of range. Bitmap size is " << m_Size << endl;
        throw Exception(AVG_ERR_OUT_OF_RANGE, ss.str());
    }
    const unsigned char * pPixel = m_pBits+intPos.y*m_Stride+intPos.x*getBytesPerPixel();
    switch(getPixelFormat()) {
        case R8G8B8A8:
            return Pixel32(pPixel[0], pPixel[1], pPixel[2], pPixel[3]);
        case R8G8B8X8:
            return Pixel32(pPixel[0], pPixel[1], pPixel[2], 255);
        case R8G8B8:
            return Pixel32(pPixel[0], pPixel[1], pPixel[2]);
        case B8G8R8A8:
            return Pixel32(pPixel[2], pPixel[1], pPixel[0], pPixel[3]);
        case B8G8R8X8:
            return Pixel32(pPixel[2], pPixel[1], pPixel[0], 255);
        case B8G8R8:
            return Pixel32(pPixel[2], pPixel[1], pPixel[0]);
        case I8:
        case A8:
            return Pixel32(pPixel[0], pPixel[0], pPixel[0]);
        default:
            cerr << getPixelFormat() << endl;
            AVG_ASSERT(false);
            return Pixel32();
    }
}

bool Bitmap::operator ==(const Bitmap& otherBmp)
{
    // We allow Name, Stride and bOwnsBits to be different here, since we're looking for
    // equal value only.
    if (m_Size != otherBmp.m_Size || m_PF != otherBmp.m_PF) {
        return false;
    }

    const unsigned char * pSrc = otherBmp.getPixels();
    unsigned char * pDest = m_pBits;
    int lineLen = getLineLen();
    for (int y = 0; y < getSize().y; ++y) {
        switch(m_PF) {
            case R8G8B8X8:
            case B8G8R8X8:
                for (int x = 0; x < getSize().x; ++x) {
                    const unsigned char * pSrcPixel = pSrc+x*getBytesPerPixel();
                    unsigned char * pDestPixel = pDest+x*getBytesPerPixel();
                    if (*((Pixel24*)(pDestPixel)) != *((Pixel24*)(pSrcPixel))) {
                        return false;
                    }
                }
                break;
            default:
                if (memcmp(pDest, pSrc, lineLen) != 0) {
                    return false;
                }
        }
        pDest += m_Stride;
        pSrc += otherBmp.getStride();
    }
    return true;
}

BitmapPtr Bitmap::subtract(const Bitmap& otherBmp)
{
    if (m_PF != otherBmp.getPixelFormat()) {
        throw Exception(AVG_ERR_UNSUPPORTED, 
                string("Bitmap::subtract: pixel formats differ(")
                + getPixelFormatString(m_PF)+", "
                + getPixelFormatString(otherBmp.getPixelFormat())+")");
    }
    if (m_Size != otherBmp.getSize()) {
        throw Exception(AVG_ERR_UNSUPPORTED, 
                string("Bitmap::subtract: bitmap sizes differ (this=")
                + toString(m_Size) + ", other=" + toString(otherBmp.getSize()) + ")");
    }
    BitmapPtr pResultBmp = BitmapPtr(new Bitmap(m_Size, m_PF));
    const unsigned char * pSrcLine1 = otherBmp.getPixels();
    const unsigned char * pSrcLine2 = m_pBits;
    unsigned char * pDestLine = pResultBmp->getPixels();
    int stride = getStride();
    int lineLen = getLineLen();

    for (int y = 0; y < getSize().y; ++y) {
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
                    for (int x=0; x<lineLen; ++x) {
                        *pDest = abs(*pSrc1-*pSrc2);
                        pSrc1++;
                        pSrc2++;
                        pDest++;
                    }
                }
        }
        pSrcLine1 += stride;
        pSrcLine2 += stride;
        pDestLine += stride;
    }
    return pResultBmp;
}
    
void Bitmap::blt(const Bitmap& otherBmp, const IntPoint& pos)
{
    AVG_ASSERT(getBytesPerPixel() == 4);
    if (pos.x < 0 || pos.y < 0) {
        throw Exception(AVG_ERR_UNSUPPORTED, 
                string("Bitmap::blt: pos < 0 is not supported."));
    }
    
    IntRect destRect(pos.x, pos.y, pos.x+otherBmp.getSize().x, 
            pos.y+otherBmp.getSize().y);
    destRect.intersect(IntRect(IntPoint(0,0), getSize()));
    for (int y = 0; y < destRect.height(); y++) {
        unsigned char * pSrcPixel = getPixels()+(pos.y+y)*getStride()+pos.x*4;
        const unsigned char * pOtherPixel = otherBmp.getPixels()+
                y*otherBmp.getStride(); 
        if (otherBmp.hasAlpha()) {
            for (int x = 0; x < destRect.width(); x++) {
                int srcAlpha = 255-pOtherPixel[3];
                pSrcPixel[0] = 
                        (srcAlpha*pSrcPixel[0]+int(pOtherPixel[3])*pOtherPixel[0])/255;
                pSrcPixel[1] = 
                        (srcAlpha*pSrcPixel[1]+int(pOtherPixel[3])*pOtherPixel[1])/255;
                pSrcPixel[2] = 
                        (srcAlpha*pSrcPixel[2]+int(pOtherPixel[3])*pOtherPixel[2])/255;
                pSrcPixel += 4;
                pOtherPixel += 4;
            }
        } else {
            for (int x = 0; x < destRect.width(); x++) {
                *(Pixel32*)pSrcPixel = *(Pixel32*)pOtherPixel;
                pSrcPixel[3] = 255;
                pSrcPixel += 4;
                pOtherPixel += 4;
            }
        }
    }
}

float Bitmap::getAvg() const
{
    float sum = 0;
    unsigned char * pSrc = m_pBits;
    int componentsPerPixel = getBytesPerPixel();
    for (int y = 0; y < getSize().y; ++y) {
        switch(m_PF) {
            case R8G8B8X8:
            case B8G8R8X8:
                {
                    Pixel32 * pSrcPixel = (Pixel32 *)pSrc;
                    for (int x = 0; x < m_Size.x; ++x) {
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
                    for (int x = 0; x < m_Size.x; ++x) {
                        sum += *pSrcPixel;
                        pSrcPixel++;
                    }
                }
                break;
            case R8G8B8A8:
            case B8G8R8A8:
                {
                    Pixel32 * pSrcPixel = (Pixel32 *)pSrc;
                    for (int x = 0; x < m_Size.x; ++x) {
                        int a = pSrcPixel->getA();
                        if (a > 0) {
                            sum += ((pSrcPixel->getR()+pSrcPixel->getG()+
                                    pSrcPixel->getB())*a)/255+pSrcPixel->getA();
                        }
                        pSrcPixel++;
                    }
                    componentsPerPixel = 4;
                }
                break;
            default:
                {
                    unsigned char * pSrcComponent = pSrc;
                    for (int x = 0; x < getLineLen(); ++x) {
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

float Bitmap::getChannelAvg(int channel) const
{
    AVG_ASSERT(!pixelFormatIsPlanar(m_PF) && !pixelFormatIsBayer(m_PF) && !(m_PF == I16));
    int bytesPerPixel = getBytesPerPixel();
    AVG_ASSERT(channel < bytesPerPixel);
    float sum = 0;
    unsigned char * pSrcLine = m_pBits;
    for (int y = 0; y < getSize().y; ++y) {
        unsigned char * pSrcPixel = pSrcLine;
        for (int x = 0; x < m_Size.x; ++x) {
            sum += *(pSrcPixel+channel);
            pSrcPixel += bytesPerPixel;
        }
        pSrcLine += m_Stride;
    }
    return sum/(getSize().x*getSize().y);
}

float Bitmap::getStdDev() const
{
    float average = getAvg();
    float sum = 0;

    unsigned char * pSrc = m_pBits;
    int componentsPerPixel = getBytesPerPixel();
    for (int y = 0; y < getSize().y; ++y) {
        switch(m_PF) {
            case R8G8B8X8:
            case B8G8R8X8:
                {
                    componentsPerPixel = 3;
                    Pixel32 * pSrcPixel = (Pixel32 *)pSrc;
                    for (int x = 0; x < m_Size.x; ++x) {
                        sum += sqr(pSrcPixel->getR()-average);
                        sum += sqr(pSrcPixel->getG()-average);
                        sum += sqr(pSrcPixel->getB()-average);
                        pSrcPixel++;
                    }
                }
                break;
            case R8G8B8A8:
            case B8G8R8A8:
                {
                    componentsPerPixel = 4;
                    Pixel32 * pSrcPixel = (Pixel32 *)pSrc;
                    for (int x = 0; x < m_Size.x; ++x) {
                        int a = pSrcPixel->getA();
                        if (a > 0) {
                            sum += sqr((pSrcPixel->getR()*a)/255-average);
                            sum += sqr((pSrcPixel->getG()*a)/255-average);
                            sum += sqr((pSrcPixel->getB()*a)/255-average);
                            sum += sqr(pSrcPixel->getA()-average);
                        }
                        pSrcPixel++;
                    }
                }
                break;
            case I16:
                {
                    componentsPerPixel = 1;
                    unsigned short * pSrcPixel = (unsigned short *)pSrc;
                    for (int x = 0; x < m_Size.x; ++x) {
                        sum += sqr(*pSrcPixel-average);
                        pSrcPixel++;
                    }
                }
                break;
            default:
                {
                    unsigned char * pSrcComponent = pSrc;
                    for (int x = 0; x < getLineLen(); ++x) {
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
    IntPoint max;
    if (bDumpPixels) {
        max = m_Size;
    } else {
        max = IntPoint(16,1);
    }
    cerr << "  Pixel data: " << endl;
    for (int y = 0; y < max.y; ++y) {
        unsigned char * pLine = m_pBits+m_Stride*y;
        cerr << "    ";
        for (int x = 0; x < max.x; ++x) {
            if (m_PF == R32G32B32A32F) {
                float * pPixel = (float*)(pLine+getBytesPerPixel()*x);
                cerr << "[";
                for (int i = 0; i < 4; ++i) {
                    cerr << setw(4) << setprecision(2) << pPixel[i] << " ";
                }
                cerr << "]";
            } else {
                unsigned char * pPixel = pLine+getBytesPerPixel()*x;
                cerr << "[";
                for (int i = 0; i < getBytesPerPixel(); ++i) {
                    cerr << hex << setw(2) << (int)(pPixel[i]) << " ";
                }
                cerr << "]";
            }
        }
        cerr << endl;
    }
    cerr << dec;
}

int Bitmap::getPreferredStride(int width, PixelFormat pf)
{
    return (((width*avg::getBytesPerPixel(pf))-1)/4+1)*4;
}

void Bitmap::initWithData(unsigned char * pBits, int stride, bool bCopyBits)
{
//    cerr << "Bitmap::initWithData()" << endl;
    if (m_PF == YCbCr422) {
        if (m_Size.x%2 == 1) {
            AVG_LOG_WARNING("Odd size for YCbCr bitmap.");
            m_Size.x++;
        }
        if (m_Size.y%2 == 1) {
            AVG_LOG_WARNING("Odd size for YCbCr bitmap.");
            m_Size.y++;
        }
        if (m_Size.x%2 == 1 || m_Size.y%2 == 1) {
            AVG_LOG_ERROR("Odd size for YCbCr bitmap.");
        }
    }
    if (bCopyBits) {
        allocBits();
        if (m_Stride == stride && stride == (m_Size.x*getBytesPerPixel())) {
            memcpy(m_pBits, pBits, stride*m_Size.y);
        } else {
            for (int y = 0; y < m_Size.y; ++y) {
                memcpy(m_pBits+m_Stride*y, pBits+stride*y, m_Stride);
            }
        }
        m_bOwnsBits = true;
    } else {
        m_pBits = pBits;
        m_Stride = stride;
        m_bOwnsBits = false;
    }
}

void Bitmap::allocBits(int stride)
{
    AVG_ASSERT(!m_pBits);
    AVG_ASSERT(!pixelFormatIsPlanar(m_PF));
    AVG_ASSERT(m_Size.x > 0 && m_Size.y > 0);
//    cerr << "Bitmap::allocBits():" << m_Size <<  endl;
    if (stride == 0) {
        m_Stride = getPreferredStride(m_Size.x, m_PF);
    } else {
        m_Stride = stride;
    }
    if (m_PF == YCbCr422) {
        if (m_Size.x%2 == 1) {
            AVG_LOG_WARNING("Odd width for YCbCr bitmap.");
            m_Size.x++;
        }
        if (m_Size.y%2 == 1) {
            AVG_LOG_WARNING("Odd height for YCbCr bitmap.");
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

void YUYV422toBGR32Line(const unsigned char* pSrcLine, Pixel32* pDestLine, int width)
{
    Pixel32 * pDestPixel = pDestLine;
    
    // We need the previous and next values to interpolate between the
    // sampled u and v values.
    int v = *(pSrcLine+3);
    int v0; // Previous v
    int u;
    int u1; // Next u;
    const unsigned char * pSrcPixels = pSrcLine;

    for (int x = 0; x < width/2-1; x++) {
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
 
void UYVY422toBGR32Line(const unsigned char* pSrcLine, Pixel32* pDestLine, int width)
{
    Pixel32 * pDestPixel = pDestLine;
    
    // We need the previous and next values to interpolate between the
    // sampled u and v values.
    int v = *(pSrcLine+2);
    int v0; // Previous v
    int u;
    int u1; // Next u;
    const unsigned char * pSrcPixels = pSrcLine;

    for (int x = 0; x < width/2-1; x++) {
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
 
void YUV411toBGR32Line(const unsigned char* pSrcLine, Pixel32* pDestLine, int width)
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

    for (int x = 0; x < width/4; x++) {
        // Four pixels at a time.
        // Source format is UYYVYY.
        u = pSrcPixels[0];
        v0 = v;
        v = pSrcPixels[3];

        if (x < width/4-1) {
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

void Bitmap::YCbCrtoBGR(const Bitmap& origBmp)
{
    AVG_ASSERT(m_PF==B8G8R8X8);
    const unsigned char * pSrc = origBmp.getPixels();
    Pixel32 * pDest = (Pixel32*)m_pBits;
    int height = min(origBmp.getSize().y, m_Size.y);
    int width = min(origBmp.getSize().x, m_Size.x);
    int StrideInPixels = m_Stride/getBytesPerPixel();
    switch(origBmp.m_PF) {
        case YCbCr422:
            for (int y = 0; y < height; ++y) {
                UYVY422toBGR32Line(pSrc, pDest, width);
                pDest += StrideInPixels;
                pSrc += origBmp.getStride();
            }
            break;
        case YUYV422:
            for (int y = 0; y < height; ++y) {
                YUYV422toBGR32Line(pSrc, pDest, width);
                pDest += StrideInPixels;
                pSrc += origBmp.getStride();
            }
            break;
        case YCbCr411:
            for (int y = 0; y < height; ++y) {
                YUV411toBGR32Line(pSrc, pDest, width);
                pDest += StrideInPixels;
                pSrc += origBmp.getStride();
            }
            break;
        default:
            // This routine shouldn't be called with other pixel formats.
            AVG_ASSERT(false);
    }
}
    
void YUYV422toI8Line(const unsigned char* pSrcLine, unsigned char* pDestLine, int width)
{
    const unsigned char * pSrc = pSrcLine;
    unsigned char * pDest = pDestLine;
    for (int x = 0; x < width; x++) {
        *pDest = *pSrc;
        pDest++;
        pSrc+=2;
    }
}
 
void YUV411toI8Line(const unsigned char* pSrcLine, unsigned char* pDestLine, int width)
{
    const unsigned char * pSrc = pSrcLine;
    unsigned char * pDest = pDestLine;
    for (int x = 0; x < width/2; x++) {
        *pDest++ = *pSrc++;
        *pDest++ = *pSrc++;
        pSrc++;
    }
}
 
void Bitmap::YCbCrtoI8(const Bitmap& origBmp)
{
    AVG_ASSERT(origBmp.getBytesPerPixel() == 1);
    const unsigned char * pSrc = origBmp.getPixels();
    unsigned char * pDest = m_pBits;
    int height = min(origBmp.getSize().y, m_Size.y);
    int width = min(origBmp.getSize().x, m_Size.x);
    switch(origBmp.m_PF) {
        case YCbCr422:
            for (int y = 0; y < height; ++y) {
                // src shifted by one byte to account for UYVY to YUYV 
                // difference in pixel order.
                YUYV422toI8Line(pSrc+1, pDest, width);
                pDest += m_Stride;
                pSrc += origBmp.getStride();
            }
            break;
        case YUYV422:
            for (int y = 0; y < height; ++y) {
                YUYV422toI8Line(pSrc, pDest, width);
                pDest += m_Stride;
                pSrc += origBmp.getStride();
            }
            break;
        case YCbCr411:
            for (int y = 0; y < height; ++y) {
                YUV411toI8Line(pSrc, pDest, width);
                pDest += m_Stride;
                pSrc += origBmp.getStride();
            }
            break;
        default:
            // This routine shouldn't be called with other pixel formats.
            AVG_ASSERT(false);
    }
}

void Bitmap::I16toI8(const Bitmap& origBmp)
{
    AVG_ASSERT(getBytesPerPixel() == 1);
    AVG_ASSERT(origBmp.getPixelFormat() == I16);
    const unsigned short * pSrc = (const unsigned short *)origBmp.getPixels();
    unsigned char * pDest = m_pBits;
    int height = min(origBmp.getSize().y, m_Size.y);
    int width = min(origBmp.getSize().x, m_Size.x);
    int srcStrideInPixels = origBmp.getStride()/origBmp.getBytesPerPixel();
    for (int y = 0; y < height; ++y) {
        const unsigned short * pSrcPixel = pSrc;
        unsigned char * pDestPixel = pDest;
        for (int x=0; x<width; ++x) {
            *pDestPixel++ = *pSrcPixel++ >> 8;
        }
        pDest += m_Stride;
        pSrc += srcStrideInPixels;
    }
}

void Bitmap::I8toI16(const Bitmap& origBmp)
{
    AVG_ASSERT(m_PF == I16);
    AVG_ASSERT(origBmp.getBytesPerPixel() == 1);
    const unsigned char * pSrc = origBmp.getPixels();
    unsigned short * pDest = (unsigned short *)m_pBits;
    int height = min(origBmp.getSize().y, m_Size.y);
    int width = min(origBmp.getSize().x, m_Size.x);
    int destStrideInPixels = m_Stride/getBytesPerPixel();
    for (int y=0; y<height; ++y) {
        const unsigned char * pSrcPixel = pSrc;
        unsigned short * pDestPixel = pDest;
        for (int x=0; x<width; ++x) {
            *pDestPixel++ = *pSrcPixel++ << 8;
        }
        pDest += destStrideInPixels;
        pSrc += origBmp.getStride();
    }
}

void Bitmap::I8toRGB(const Bitmap& origBmp)
{
    AVG_ASSERT(getBytesPerPixel() == 4 || getBytesPerPixel() == 3);
    AVG_ASSERT(origBmp.getBytesPerPixel() == 1);
    const unsigned char * pSrc = origBmp.getPixels();
    int height = min(origBmp.getSize().y, m_Size.y);
    int width = min(origBmp.getSize().x, m_Size.x);
    if (getBytesPerPixel() == 4) {
        unsigned int * pDest = (unsigned int *)m_pBits;
        int destStrideInPixels = m_Stride/getBytesPerPixel();
        for (int y = 0; y < height; ++y) {
            const unsigned char * pSrcPixel = pSrc;
            unsigned int * pDestPixel = pDest;
            for (int x = 0; x < width; ++x) {
                *pDestPixel = (((((255 << 8)+(*pSrcPixel)) << 8)+
                        *pSrcPixel) << 8) +(*pSrcPixel);
                pDestPixel ++;
                pSrcPixel++;
            }
            pDest += destStrideInPixels;
            pSrc += origBmp.getStride();
        }
    } else {
        unsigned char * pDest = m_pBits;
        for (int y = 0; y < height; ++y) {
            const unsigned char * pSrcPixel = pSrc;
            unsigned char * pDestPixel = pDest;
            for (int x = 0; x < width; ++x) {
                *pDestPixel++ = *pSrcPixel;
                *pDestPixel++ = *pSrcPixel;
                *pDestPixel++ = *pSrcPixel;
                pSrcPixel++;
            }
            pDest += getStride();
            pSrc += origBmp.getStride();
        }
    }
}

void Bitmap::ByteRGBAtoFloatRGBA(const Bitmap& origBmp)
{
    AVG_ASSERT(getPixelFormat() == R32G32B32A32F);
    AVG_ASSERT(origBmp.getBytesPerPixel() == 4);
    const unsigned char * pSrc = origBmp.getPixels();
    int height = min(origBmp.getSize().y, m_Size.y);
    int width = min(origBmp.getSize().x, m_Size.x);
    float * pDest = (float *)m_pBits;
    for (int y = 0; y < height; ++y) {
        const unsigned char * pSrcPixel = pSrc;
        float * pDestPixel = pDest;
        for (int x = 0; x < width*4; ++x) {
            *pDestPixel = float(*pSrcPixel)/255;
            pDestPixel ++;
            pSrcPixel++;
        }
        pDest += m_Stride/sizeof(float);
        pSrc += origBmp.getStride();
    }
}

void Bitmap::FloatRGBAtoByteRGBA(const Bitmap& origBmp)
{
    AVG_ASSERT(getBytesPerPixel() == 4);
    AVG_ASSERT(origBmp.getPixelFormat() == R32G32B32A32F);
    const float * pSrc = (const float *)origBmp.getPixels();
    int height = min(origBmp.getSize().y, m_Size.y);
    int width = min(origBmp.getSize().x, m_Size.x);
    unsigned char * pDest = m_pBits;
    for (int y = 0; y < height; ++y) {
        const float * pSrcPixel = pSrc;
        unsigned char * pDestPixel = pDest;
        for (int x = 0; x < width*4; ++x) {
            *pDestPixel = (unsigned char)(*pSrcPixel*255+0.5);
            pDestPixel++;
            pSrcPixel++;
        }
        pDest += m_Stride;
        pSrc += origBmp.getStride()/sizeof(float);
    }
}

/*
// Nearest Neighbour Bayer Pattern de-mosaicking
// Code has been taken and adapted from libdc1394 Bayer conversion
// TODO: adapt it for RGB24, not just for RGB32
// TODO: add more CFA patterns (now only the GBRG is defined and used)
void Bitmap::BY8toRGBNearest(const Bitmap& origBmp)
{
    AVG_ASSERT(getBytesPerPixel() == 4);
    AVG_ASSERT(origBmp.getPixelFormat() == BAYER8_GBRG);

    int height = min(origBmp.getSize().y, m_Size.y);
    int width = min(origBmp.getSize().x, m_Size.x);

    const int srcStride = width;
    const int destStride = 4 * width;
    int width = width;
    int height = height;

    // CFA Pattern selection: BGGR: blue=-1, swg=0; GRBG: blue=1, swg=1
    // Assuming GBRG
    int blue = 1;
    int greenFirst = 1;

    const unsigned char *pSrcPixel = origBmp.getPixels();
    unsigned char *pDestPixel = (unsigned char *) getPixels();

    pDestPixel += 1;
    width -= 1;
    height -= 1;

    while (--height) {

        const unsigned char *pSrcEndBoundary = pSrcPixel + width;

        if (greenFirst) {
            pDestPixel[-blue] = pSrcPixel[1];
            pDestPixel[0] = pSrcPixel[srcStride + 1];
            pDestPixel[blue] = pSrcPixel[srcStride];
            pDestPixel[2] = 255; // Alpha channel
            ++pSrcPixel;
            pDestPixel += 4;
        }

        if (blue > 0) {
            while (pSrcPixel <= pSrcEndBoundary - 2) {
                pDestPixel[-1] = pSrcPixel[0];
                pDestPixel[0] = pSrcPixel[1];
                pDestPixel[1] = pSrcPixel[srcStride + 1];
                pDestPixel[2] = 255; // Alpha channel

                pDestPixel[3] = pSrcPixel[2];
                pDestPixel[4] = pSrcPixel[srcStride + 2];
                pDestPixel[5] = pSrcPixel[srcStride + 1];
                pDestPixel[6] = 255; // Alpha channel
                
                pSrcPixel += 2;
                pDestPixel += 8;
            }
        } else {
            while (pSrcPixel <= pSrcEndBoundary - 2) {
                pDestPixel[1] = pSrcPixel[0];
                pDestPixel[0] = pSrcPixel[1];
                pDestPixel[-1] = pSrcPixel[srcStride + 1];
                
                pDestPixel[6] = 255; // Alpha channel
                pDestPixel[5] = pSrcPixel[2];
                pDestPixel[4] = pSrcPixel[srcStride + 2];
                pDestPixel[3] = pSrcPixel[srcStride + 1];
                pDestPixel[2] = 255; // Alpha channel

                pSrcPixel += 2;
                pDestPixel += 8;
            }
        }

        if (pSrcPixel < pSrcEndBoundary) {
            pDestPixel[-blue] = pSrcPixel[0];
            pDestPixel[0] = pSrcPixel[1];
            pDestPixel[blue] = pSrcPixel[srcStride + 1];
            pDestPixel[2] = 255; // Alpha channel
            ++pSrcPixel;
            pDestPixel += 4;
        }

        pSrcPixel -= width;
        pDestPixel -= width * 4;

        blue = -blue;
        greenFirst = !greenFirst;

        pSrcPixel += srcStride;
        pDestPixel += destStride;
    }
}
*/

// Bilinear Bayer Pattern de-mosaicking
// Code has been taken and adapted from libdc1394 Bayer conversion
// Original source is OpenCV Bayer pattern decoding
// TODO: adapt it for RGB24, not just for RGB32
void Bitmap::BY8toRGBBilinear(const Bitmap& origBmp)
{
    AVG_ASSERT(getBytesPerPixel() == 4);
    AVG_ASSERT(pixelFormatIsBayer(origBmp.getPixelFormat()));

    int height = min(origBmp.getSize().y, m_Size.y);
    int width = min(origBmp.getSize().x, m_Size.x);

    const int srcStride = width;
    const int doubleSrcStride = srcStride * 2;
    const int destStride = 4 * width;

    // CFA Pattern selection
    PixelFormat pf = origBmp.getPixelFormat();
    int blue;
    int greenFirst;
    if (pf == BAYER8_BGGR || pf == BAYER8_GBRG) {
        blue = -1;
    } else {
        blue = 1;
    }

    if (pf == BAYER8_GBRG || pf == BAYER8_GRBG) {
        greenFirst = 1;
    } else {
        greenFirst = 0;
    }

    const unsigned char *pSrcPixel = origBmp.getPixels();
    unsigned char *pDestPixel = (unsigned char *) getPixels();

    pDestPixel += destStride + 4 + 1;
    height -= 2;
    width -= 2;

    while (height--) {
        int t0, t1;
        const unsigned char *pSrcEndBoundary = pSrcPixel + width;

        if (greenFirst) {
            t0 = (pSrcPixel[1] + pSrcPixel[doubleSrcStride + 1] + 1) >> 1;
            t1 = (pSrcPixel[srcStride] + pSrcPixel[srcStride + 2] + 1) >> 1;
            pDestPixel[-blue] = (unsigned char) t0;
            pDestPixel[0] = pSrcPixel[srcStride + 1];
            pDestPixel[blue] = (unsigned char) t1;
            pDestPixel[2] = 255; // Alpha channel
            ++pSrcPixel;
            pDestPixel += 4;
        }
                
        if (blue > 0) {
            while (pSrcPixel <= pSrcEndBoundary - 2) {
                t0 = (pSrcPixel[0] + pSrcPixel[2] + pSrcPixel[doubleSrcStride] +
                      pSrcPixel[doubleSrcStride + 2] + 2) >> 2;
                t1 = (pSrcPixel[1] + pSrcPixel[srcStride] +
                      pSrcPixel[srcStride + 2] + pSrcPixel[doubleSrcStride + 1] +
                      2) >> 2;
                pDestPixel[-1] = (unsigned char) t0;
                pDestPixel[0] = (unsigned char) t1;
                pDestPixel[1] = pSrcPixel[srcStride + 1];
                pDestPixel[2] = 255; // Alpha channel

                t0 = (pSrcPixel[2] + pSrcPixel[doubleSrcStride + 2] + 1) >> 1;
                t1 = (pSrcPixel[srcStride + 1] + pSrcPixel[srcStride + 3] +
                      1) >> 1;
                pDestPixel[3] = (unsigned char) t0;
                pDestPixel[4] = pSrcPixel[srcStride + 2];
                pDestPixel[5] = (unsigned char) t1;
                pDestPixel[6] = 255; // Alpha channel
                
                pSrcPixel += 2;
                pDestPixel += 8;
            }
        } else {
            while (pSrcPixel <= pSrcEndBoundary - 2) {
                t0 = (pSrcPixel[0] + pSrcPixel[2] + pSrcPixel[doubleSrcStride] +
                      pSrcPixel[doubleSrcStride + 2] + 2) >> 2;
                t1 = (pSrcPixel[1] + pSrcPixel[srcStride] +
                      pSrcPixel[srcStride + 2] + pSrcPixel[doubleSrcStride + 1] +
                      2) >> 2;
                pDestPixel[1] = (unsigned char) t0;
                pDestPixel[0] = (unsigned char) t1;
                pDestPixel[-1] = pSrcPixel[srcStride + 1];
                pDestPixel[2] = 255; // Alpha channel

                t0 = (pSrcPixel[2] + pSrcPixel[doubleSrcStride + 2] + 1) >> 1;
                t1 = (pSrcPixel[srcStride + 1] + pSrcPixel[srcStride + 3] +
                      1) >> 1;
                pDestPixel[5] = (unsigned char) t0;
                pDestPixel[4] = pSrcPixel[srcStride + 2];
                pDestPixel[3] = (unsigned char) t1;
                pDestPixel[6] = 255; // Alpha channel
                
                pSrcPixel += 2;
                pDestPixel += 8;
            }
        }

        if (pSrcPixel < pSrcEndBoundary) {
            t0 = (pSrcPixel[0] + pSrcPixel[2] + pSrcPixel[doubleSrcStride] +
                  pSrcPixel[doubleSrcStride + 2] + 2) >> 2;
            t1 = (pSrcPixel[1] + pSrcPixel[srcStride] +
                  pSrcPixel[srcStride + 2] + pSrcPixel[doubleSrcStride + 1] +
                  2) >> 2;
            pDestPixel[-blue] = (unsigned char) t0;
            pDestPixel[0] = (unsigned char) t1;
            pDestPixel[blue] = pSrcPixel[srcStride + 1];
            pDestPixel[2] = 255; // Alpha channel
            pSrcPixel++;
            pDestPixel += 4;
        }

        pSrcPixel -= width;
        pDestPixel -= width * 4;

        blue = -blue;
        greenFirst = !greenFirst;
        
        pSrcPixel += srcStride;
        pDestPixel += destStride;
    }
}

template<class DESTPIXEL, class SRCPIXEL>
void createTrueColorCopy(Bitmap& destBmp, const Bitmap& srcBmp)
{
    SRCPIXEL * pSrcLine = (SRCPIXEL*) srcBmp.getPixels();
    DESTPIXEL * pDestLine = (DESTPIXEL*) destBmp.getPixels();
    int height = min(srcBmp.getSize().y, destBmp.getSize().y);
    int width = min(srcBmp.getSize().x, destBmp.getSize().x);
    for (int y = 0; y < height; ++y) {
        SRCPIXEL * pSrcPixel = pSrcLine;
        DESTPIXEL * pDestPixel = pDestLine;
        for (int x = 0; x < width; ++x) {
            *pDestPixel = *pSrcPixel;
            ++pSrcPixel;
            ++pDestPixel;
        }
        pSrcLine = (SRCPIXEL *)((unsigned char *)pSrcLine + srcBmp.getStride());
        pDestLine = (DESTPIXEL *)((unsigned char *)pDestLine + destBmp.getStride());
    }
}

template<>
void createTrueColorCopy<Pixel32, Pixel8>(Bitmap& destBmp, const Bitmap& srcBmp)
{
    const unsigned char * pSrcLine = srcBmp.getPixels();
    unsigned char * pDestLine = destBmp.getPixels();
    int height = min(srcBmp.getSize().y, destBmp.getSize().y);
    int width = min(srcBmp.getSize().x, destBmp.getSize().x);
    int srcStride = srcBmp.getStride();
    int destStride = destBmp.getStride();
    for (int y = 0; y < height; ++y) {
        const unsigned char * pSrcPixel = pSrcLine;
        unsigned char * pDestPixel = pDestLine;
        for (int x = 0; x < width; ++x) {
            pDestPixel[0] =
            pDestPixel[1] =
            pDestPixel[2] = *pSrcPixel;
            pDestPixel[3] = 255;
            ++pSrcPixel;
            pDestPixel+=4;
        }
        pSrcLine = pSrcLine + srcStride;
        pDestLine = pDestLine + destStride;
    }
}

template<>
void createTrueColorCopy<Pixel8, Pixel32>(Bitmap& destBmp, const Bitmap& srcBmp)
{
    const unsigned char * pSrcLine = srcBmp.getPixels();
    unsigned char * pDestLine = destBmp.getPixels();
    int height = min(srcBmp.getSize().y, destBmp.getSize().y);
    int width = min(srcBmp.getSize().x, destBmp.getSize().x);
    int srcStride = srcBmp.getStride();
    int destStride = destBmp.getStride();
    bool bRedFirst = (srcBmp.getPixelFormat() == R8G8B8A8) || 
            (srcBmp.getPixelFormat() == R8G8B8X8);
    for (int y = 0; y<height; ++y) {
        const unsigned char * pSrcPixel = pSrcLine;
        unsigned char * pDestPixel = pDestLine;
        if (bRedFirst) {
            for (int x = 0; x < width; ++x) {
                *pDestPixel = ((pSrcPixel[0]*54+pSrcPixel[1]*183+pSrcPixel[2]*19)/256);
                pSrcPixel+=4;
                ++pDestPixel;
            }
        } else {
            for (int x = 0; x < width; ++x) {
                *pDestPixel = ((pSrcPixel[0]*19+pSrcPixel[1]*183+pSrcPixel[2]*54)/256);
                pSrcPixel+=4;
                ++pDestPixel;
            }
        }
        pSrcLine = pSrcLine + srcStride;
        pDestLine = pDestLine + destStride;
    }
}


template<class PIXEL>
void createTrueColorCopy(Bitmap& destBmp, const Bitmap& srcBmp)
{
    switch(srcBmp.getPixelFormat()) {
        case B8G8R8A8:
        case B8G8R8X8:
        case A8B8G8R8:
        case X8B8G8R8:
        case R8G8B8A8:
        case R8G8B8X8:
        case A8R8G8B8:
        case X8R8G8B8:
            createTrueColorCopy<PIXEL, Pixel32>(destBmp, srcBmp);
            break;
        case B8G8R8:
        case R8G8B8:
            createTrueColorCopy<PIXEL, Pixel24>(destBmp, srcBmp);
            break;
        case B5G6R5:
        case R5G6B5:
            createTrueColorCopy<PIXEL, Pixel16>(destBmp, srcBmp);
            break;
        case I8:
        case A8:
        case BAYER8_RGGB:
        case BAYER8_GBRG:
        case BAYER8_GRBG:
        case BAYER8_BGGR:
            createTrueColorCopy<PIXEL, Pixel8>(destBmp, srcBmp);
            break;
        default:
            // Unimplemented conversion.
            AVG_ASSERT(false);
    }
}

};
