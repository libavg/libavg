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
#ifndef _VAAPIDecoder_H_
#define _VAAPIDecoder_H_


#include "../avgconfigwrapper.h"
#include "../base/GLMHelper.h"

#include "WrapFFMpeg.h"
#include "VAAPIHelper.h"

#include <libavcodec/vaapi.h>

namespace avg {

class VAAPIDecoder
{
public:
    VAAPIDecoder();
    ~VAAPIDecoder();
    AVCodec* openCodec(AVCodecContext* pCodec);

    static bool isAvailable();

private:
    // Callbacks
    static int getBuffer(AVCodecContext* pContext, AVFrame* pFrame);
    static void releaseBuffer(struct AVCodecContext* pContext, AVFrame* pFrame);
    static void drawHorizBand(AVCodecContext* pContext, const AVFrame* pFrame, 
            int offset[4], int y, int type, int height);
    static AVPixelFormat getFormat(AVCodecContext* pContext, const AVPixelFormat* pFmt);

    IntPoint m_Size;
    vaapi_context*  m_VAAPIContext;

};

}
#endif

