//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
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

#ifndef _IVideoDecoder_H_
#define _IVideoDecoder_H_

#include "../graphics/Bitmap.h"
#include "DisplayEngine.h"

#include <string>

namespace avg {

class IVideoDecoder
{
    public:
        virtual ~IVideoDecoder() {};
        virtual void open(const std::string& sFilename, DisplayEngine::YCbCrMode ycbcrMode) = 0;
        virtual void close() = 0;
        virtual void seek(int DestFrame) = 0;
        virtual IntPoint getSize() = 0;
        virtual int getNumFrames() = 0;
        virtual double getFPS() = 0;

        virtual bool renderToBmp(BitmapPtr pBmp) = 0;
        virtual bool renderToYCbCr420p(BitmapPtr pBmpY, BitmapPtr pBmpCb, 
                BitmapPtr pBmpCr) = 0;
        virtual bool canRenderToBuffer(int BPP) = 0;
        virtual PixelFormat getPixelFormat() = 0;
};

typedef boost::shared_ptr<IVideoDecoder> VideoDecoderPtr;

}
#endif 

