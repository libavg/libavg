//
//  libavg - Media Playback Engine.
//  Copyright (C) 2003-2020 Ulrich von Zadow
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

#ifndef _WrapFFMpeg_H_
#define _WrapFFMpeg_H_

#include <string>

#include "../avgconfigwrapper.h"

#ifdef _WIN32
#define EMULATE_INTTYPES
#else
// This is probably GCC-specific.
#if !defined INT64_C
#if defined __WORDSIZE && __WORDSIZE == 64
#define INT64_C(c) c ## L
#define UINT64_C(c) c ## UL
#else
#define INT64_C(c) c ## LL
#define UINT64_C(c) c ## ULL
#endif
#endif
#endif

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/avutil.h>
#include <libavutil/pixdesc.h>
#include <libavutil/mathematics.h>
#include <libavutil/opt.h>
#include <libavutil/error.h>

#ifndef URL_WRONLY
        #define url_fopen avio_open
        #define url_fclose avio_close
        #define URL_WRONLY AVIO_FLAG_WRITE
#endif
#include <libswresample/swresample.h>
#include <libswresample/version.h>
}

// Old ffmpeg has PixelFormat, new ffmpeg uses AVPixelFormat.
// Intermediate versions define PixelFormat in terms of AVPixelFormat for compatibility.
// libavg also defines avg::PixelFormat.
#ifdef PixelFormat
    // In this case, PixelFormat is #defined and collides with avg::PixelFormat.
    // AVPixelFormat is also defined.
    #undef PixelFormat
#endif

namespace avg
{
    const std::string getAVErrorString(int errNum);
}

#endif
