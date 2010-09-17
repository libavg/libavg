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

#ifndef _PixelFormat_H_
#define _PixelFormat_H_

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
    I8, I16,
    A8,
    YCbCr411,  // Interleaved YCbCr: Y,Y,Cb,Y,Y,Cr,... Effectively 12 bits per pixel.
    YCbCr422,  // Interleaved YCbCr: Cb,Y,Cr,Y,...
    YUYV422,   // Interleaved YCbCr, grey values first: Y,Cb,Y,Cr,...
    YCbCr420p, // Not really a valid pixel format. Signifies separate bitmaps
               // for Y, Cb and Cr components, with Cb and Cr half as big in 
               // both x and y dimensions. This is mpeg YCbCr, where the 
               // color components have values from 16...235.
    YCbCrJ420p,// Same as YCbCr420p, but this is the jpeg version with component
               // values in the range 0...255
    YCbCrA420p,// YCbCr420p with an additional alpha bitmap at full resolution.
    BAYER8,    // Bayer pattern, unspecified RGB ordering. Not allowed in an actual
               // image.
    BAYER8_RGGB,
    BAYER8_GBRG,
    BAYER8_GRBG,
    BAYER8_BGGR,
    R32G32B32A32F, // 32bit per channel float rgba
    I32F,
    NO_PIXELFORMAT
} PixelFormat;

std::ostream& operator<<(std::ostream& os, PixelFormat pf);

std::string getPixelFormatString(PixelFormat PF);
PixelFormat stringToPixelFormat(const std::string& s);
bool pixelFormatIsColored(PixelFormat pf);
bool pixelFormatIsBayer(PixelFormat pf);
bool pixelFormatHasAlpha(PixelFormat pf);
bool pixelFormatIsPlanar(PixelFormat pf);
unsigned getNumPixelFormatPlanes(PixelFormat pf);

}
#endif
