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
#if LIBAVFORMAT_VERSION_MAJOR < 53
#define AVMEDIA_TYPE_VIDEO CODEC_TYPE_VIDEO
#define AVMEDIA_TYPE_AUDIO CODEC_TYPE_AUDIO
#endif

#if LIBAVFORMAT_VERSION_MAJOR > 52
  #define SAMPLE_FMT_S16 AV_SAMPLE_FMT_S16
  #define SAMPLE_FMT_FLT AV_SAMPLE_FMT_FLT
  #define SAMPLE_FMT_DBL AV_SAMPLE_FMT_DBL
  #define SAMPLE_FMT_S32 AV_SAMPLE_FMT_S32
  #define SAMPLE_FMT_U8 AV_SAMPLE_FMT_U8
  #define SAMPLE_FMT_S16P AV_SAMPLE_FMT_S16P
  #define SAMPLE_FMT_FLTP AV_SAMPLE_FMT_FLTP
  #define SampleFormat AVSampleFormat
#endif

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
#ifdef HAVE_LIBAVRESAMPLE_AVRESAMPLE_H
    #include <libavresample/avresample.h>
    #include <libavresample/version.h>
#endif
}

#ifdef PixelFormat
#undef PixelFormat
#else
#define AVPixelFormat ::PixelFormat
#endif

#endif
