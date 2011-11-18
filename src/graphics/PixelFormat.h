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

#ifndef _PixelFormat_H_
#define _PixelFormat_H_

#include "../api.h"
#include <string>
#include <vector>

namespace avg {

typedef enum {
    B5G6R5, B8G8R8, B8G8R8A8, B8G8R8X8, A8B8G8R8, X8B8G8R8,
    R5G6B5, R8G8B8, R8G8B8A8, R8G8B8X8, A8R8G8B8, X8R8G8B8,
    I8, I16,
    A8,
    YCbCr411,  
    YCbCr422,  
    YUYV422,   
    YCbCr420p, 
    YCbCrJ420p,
    YCbCrA420p,
    BAYER8,    
    BAYER8_RGGB,
    BAYER8_GBRG,
    BAYER8_GRBG,
    BAYER8_BGGR,
    R32G32B32A32F, // 32bit per channel float rgba
    I32F,
    NO_PIXELFORMAT
} PixelFormat;

AVG_API std::ostream& operator <<(std::ostream& os, PixelFormat pf);

std::string AVG_API getPixelFormatString(PixelFormat pf);
PixelFormat AVG_API stringToPixelFormat(const std::string& s);
std::vector<std::string> AVG_API getSupportedPixelFormats();
bool AVG_API pixelFormatIsColored(PixelFormat pf);
bool AVG_API pixelFormatIsBayer(PixelFormat pf);
bool AVG_API pixelFormatHasAlpha(PixelFormat pf);
bool AVG_API pixelFormatIsPlanar(PixelFormat pf);
unsigned AVG_API getNumPixelFormatPlanes(PixelFormat pf);
unsigned getBytesPerPixel(PixelFormat pf);

}
#endif
