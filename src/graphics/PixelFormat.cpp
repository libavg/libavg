//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2014 Ulrich von Zadow
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

#include "PixelFormat.h"

#include "../base/StringHelper.h"
#include "../base/Exception.h"
#include "../base/Logger.h"

#include <ostream>

using namespace std;

namespace avg {

std::ostream& operator <<(std::ostream& os, PixelFormat pf)
{
    os << getPixelFormatString(pf);
    return os;
}

string getPixelFormatString(PixelFormat pf)
{
    switch (pf) {
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
        case A8:
            return "A8";
        case R8:
            return "R8";
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
        case YCbCrA420p:
            return "YCbCrA420p";
        case BAYER8:
            return "BAYER8";
        case BAYER8_RGGB:
            return "BAYER8_RGGB";
        case BAYER8_GBRG:
            return "BAYER8_GBRG";
        case BAYER8_GRBG:
            return "BAYER8_GRBG";
        case BAYER8_BGGR:
            return "BAYER8_BGGR";
        case R32G32B32A32F:
            return "R32G32B32A32F";
        case I32F:
            return "I32F";
#ifdef AVG_ENABLE_V4L2_JPEG
        case JPEG:
            return "JPEG";
#endif
        case NO_PIXELFORMAT:
            return "NO_PIXELFORMAT";
        default:
            return "Unknown " + toString(int(pf));
    }
}

PixelFormat stringToPixelFormat(const string& s)
{
    if (s == "B5G6R5") {
        return B5G6R5;
    }
    if (s == "B8G8R8") {
        return B8G8R8;
    }
    if (s == "B8G8R8A8") {
        return B8G8R8A8;
    }
    if (s == "B8G8R8X8") {
        return B8G8R8X8;
    }
    if (s == "A8B8G8R8") {
        return A8B8G8R8;
    }
    if (s == "X8B8G8R8") {
        return X8B8G8R8;
    }
    if (s == "R5G6B5") {
        return R5G6B5;
    }
    if (s == "R8G8B8") {
        return R8G8B8;
    }
    if (s == "R8G8B8A8") {
        return R8G8B8A8;
    }
    if (s == "R8G8B8X8") {
        return R8G8B8X8;
    }
    if (s == "A8R8G8B8") {
        return A8R8G8B8;
    }
    if (s == "X8R8G8B8") {
        return X8R8G8B8;
    }
    if (s == "I8") {
        return I8;
    }
    if (s == "I16") {
        return I16;
    }
    if (s == "A8") {
        return A8;
    }
    if (s == "R8") {
        return R8;
    }
    if (s == "YCbCr411") {
        return YCbCr411;
    }
    if (s == "YCbCr422") {
        return YCbCr422;
    }
    if (s == "YUYV422") {
        return YUYV422;
    }
    if (s == "YCbCr420p") {
        return YCbCr420p;
    }
    if (s == "YCbCrJ420p") {
        return YCbCrJ420p;
    }
    if (s == "YCbCrA420p") {
        return YCbCrA420p;
    }
    if (s == "BAYER8") {
        return BAYER8;
    }
    if (s == "BAYER8_RGGB") {
        return BAYER8_RGGB;
    }
    if (s == "BAYER8_GBRG") {
        return BAYER8_GBRG;
    }
    if (s == "BAYER8_GRBG") {
        return BAYER8_GRBG;
    }
    if (s == "BAYER8_BGGR") {
        return BAYER8_BGGR;
    }
    if (s == "R32G32B32A32F") {
        return R32G32B32A32F;
    }
    if (s == "I32F") {
        return I32F;
    }
#ifdef AVG_ENABLE_V4L2_JPEG
    if (s == "JPEG") {
        return JPEG;
    }
#endif
    return NO_PIXELFORMAT;
}

std::vector<std::string> getSupportedPixelFormats()
{
    std::vector<std::string> pixelFormatsVector;
    int itPixelFormat = 0;
    while((PixelFormat)itPixelFormat != NO_PIXELFORMAT){
        std::string format = getPixelFormatString((PixelFormat)itPixelFormat);
        pixelFormatsVector.push_back(format);
        itPixelFormat++;
    }
    return pixelFormatsVector;
}

bool pixelFormatIsColored(PixelFormat pf)
{
    return (pf != I8 && pf != I16 && pf != I32F);
}

bool pixelFormatIsBayer(PixelFormat pf)
{
    return (pf == BAYER8 || pf == BAYER8_RGGB || pf == BAYER8_GBRG
            || pf == BAYER8_GRBG || pf == BAYER8_BGGR);
}

bool pixelFormatHasAlpha(PixelFormat pf)
{
    return pf == B8G8R8A8 || pf == A8B8G8R8 || pf == R8G8B8A8 || pf == A8R8G8B8 ||
            pf == YCbCrA420p;
}

bool pixelFormatIsPlanar(PixelFormat pf)
{
    return pf == YCbCr420p || pf == YCbCrJ420p || pf == YCbCrA420p;
}

bool AVG_API pixelFormatIsBlueFirst(PixelFormat pf)
{
    return pf == B5G6R5 || pf == B8G8R8 || pf == B8G8R8X8 || pf == B8G8R8A8;
}

unsigned getNumPixelFormatPlanes(PixelFormat pf)
{
    switch (pf) {
        case YCbCr420p:
        case YCbCrJ420p:
            return 3;
        case YCbCrA420p:
            return 4;
        default:
            return 1; 
    }
}

unsigned getBytesPerPixel(PixelFormat pf)
{
    switch (pf) {
        case R32G32B32A32F:
            return 16;
        case A8B8G8R8:
        case X8B8G8R8:
        case A8R8G8B8:
        case X8R8G8B8:
        case B8G8R8A8:
        case B8G8R8X8:
        case R8G8B8A8:
        case R8G8B8X8:
        case I32F:
            return 4;
        case R8G8B8:
        case B8G8R8:
#ifdef AVG_ENABLE_V4L2_JPEG
        case JPEG:
#endif
            return 3;
        case B5G6R5:
        case R5G6B5:
        case I16:
            return 2;
        case I8:
        case A8:
        case R8:
        case BAYER8:
        case BAYER8_RGGB:
        case BAYER8_GBRG:
        case BAYER8_GRBG:
        case BAYER8_BGGR:
            return 1;
        case YUYV422:
        case YCbCr422:
            return 2;
        default:
            AVG_LOG_ERROR("getBytesPerPixel(): Unknown format " <<
                    getPixelFormatString(pf) << ".");
            AVG_ASSERT(false);
            return 0;
    }
}

}
