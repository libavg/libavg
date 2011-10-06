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
#if LIBAVCODEC_VERSION_MAJOR > 52
#include <libavutil/pixdesc.h>
#else
#define av_get_pix_fmt_name avcodec_get_pix_fmt_name
#endif
#if LIBAVFORMAT_VERSION_MAJOR < 53
#define AVMEDIA_TYPE_VIDEO CODEC_TYPE_VIDEO
#define AVMEDIA_TYPE_AUDIO CODEC_TYPE_AUDIO
#endif

}

#endif
