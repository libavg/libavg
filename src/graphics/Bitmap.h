//
// $Id$
//

#ifndef _Bitmap_H_
#define _Bitmap_H_

#include "Point.h"
#include "Rect.h"

#include <boost/shared_ptr.hpp>
#include <string>

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
    I8, YCbCr422
} PixelFormat;
    
class Bitmap
{
public:
    Bitmap(IntPoint Size, PixelFormat PF, const std::string& sName="");
    Bitmap(IntPoint Size, PixelFormat PF, unsigned char * pBits, 
            int Stride, bool bCopyBits, const std::string& sName="");
    Bitmap(const Bitmap& Orig);
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
    static std::string getPixelFormatString(PixelFormat PF);
    unsigned char * getPixels();
    const unsigned char * getPixels() const;
    bool ownsBits() const;
    const std::string& getName() const;
    int getBytesPerPixel() const;
    static int getBytesPerPixel(PixelFormat PF);
    bool hasAlpha() const;

    bool operator ==(const Bitmap & otherBmp);
    void dump(bool bDumpPixels=false);

private:
    void initWithData(unsigned char * pBits, int Stride, bool bCopyBits);
    void allocBits();

    IntPoint m_Size;
    int m_Stride;
    PixelFormat m_PF;
    unsigned char * m_pBits;
    bool m_bOwnsBits;
    std::string m_sName;
};

typedef boost::shared_ptr<Bitmap> BitmapPtr;

}
#endif
