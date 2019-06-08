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

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(54, 25, 00)
  #define AV_CODEC_ID_MPEG1VIDEO CODEC_ID_MPEG1VIDEO
  #define AV_CODEC_ID_MPEG2VIDEO CODEC_ID_MPEG2VIDEO
  #define AV_CODEC_ID_H264 CODEC_ID_H264
  #define AV_CODEC_ID_WMV3 CODEC_ID_WMV3
  #define AV_CODEC_ID_VC1 CODEC_ID_VC1
  #define AV_CODEC_ID_MJPEG CODEC_ID_MJPEG
  #define AV_CODEC_ID_NONE CODEC_ID_NONE
#endif

#ifndef URL_WRONLY
        #define url_fopen avio_open
        #define url_fclose avio_close
        #define URL_WRONLY AVIO_FLAG_WRITE
#endif
#include <libswresample/swresample.h>
#include <libswresample/version.h>
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,1)
#define av_frame_alloc  avcodec_alloc_frame
#endif
}

// Old ffmpeg has PixelFormat, new ffmpeg uses AVPixelFormat.
// Intermediate versions define PixelFormat in terms of AVPixelFormat for compatibility.
// libavg also defines avg::PixelFormat.
#ifdef PixelFormat
    // In this case, PixelFormat is #defined and collides with avg::PixelFormat.
    // AVPixelFormat is also defined.
    #undef PixelFormat
#endif
#ifndef AV_PIX_FMT_NE
    // Old version, no AVPixelFormat defined.
    // ::PixelFormat is a typedef, so no collision with avg::PixelFormat.
    #define AVPixelFormat ::PixelFormat
    #define AV_PIX_FMT_NONE PIX_FMT_NONE
    #define AV_PIX_FMT_RGB24 PIX_FMT_RGB24
    #define AV_PIX_FMT_RGB32 PIX_FMT_RGB32
    #define AV_PIX_FMT_BGR24 PIX_FMT_BGR24
    #define AV_PIX_FMT_RGBA PIX_FMT_RGBA
    #define AV_PIX_FMT_BGRA PIX_FMT_BGRA
    #define AV_PIX_FMT_YUV420P PIX_FMT_YUV420P
    #define AV_PIX_FMT_YUVJ420P PIX_FMT_YUVJ420P
    #define AV_PIX_FMT_YUVA420P PIX_FMT_YUVA420P
    #define AV_PIX_FMT_YUYV422 PIX_FMT_YUYV422
#endif

namespace avg
{
    const std::string getAVErrorString(int errNum);
}

#endif
