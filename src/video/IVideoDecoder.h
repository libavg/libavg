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
#include "../audio/IAudioSource.h"

#include <string>

namespace avg {

enum YCbCrMode {
    OGL_NONE, OGL_MESA, OGL_APPLE, OGL_SHADER
};

enum FrameAvailableCode {
    FA_NEW_FRAME, FA_USE_LAST_FRAME, FA_STILL_DECODING
};

enum StreamSelect {
    SS_AUDIO, SS_VIDEO, SS_DEFAULT, SS_NONE, SS_ALL
};

class IVideoDecoder
{
    public:
        virtual ~IVideoDecoder() {};
        virtual void open(const std::string& sFilename, const AudioParams& AP,
                YCbCrMode ycbcrMode, bool bSyncDemuxer) = 0;
        virtual void close() = 0;
        virtual void seek(long long DestTime) = 0;
        virtual StreamSelect getMasterStream() = 0;
        virtual bool hasVideo() = 0;
        virtual bool hasAudio() = 0;
        virtual IntPoint getSize() = 0;
        virtual int getCurFrame() = 0;
        virtual int getNumFrames() = 0;
        virtual long long getCurTime(StreamSelect Stream = SS_DEFAULT) = 0;
        virtual long long getDuration() = 0;
        virtual double getNominalFPS() = 0;
        virtual double getFPS() = 0;
        virtual void setFPS(double FPS) = 0;
        virtual double getVolume() = 0;
        virtual void setVolume(double Volume) = 0;
        virtual void setAudioEnabled(bool bEnabled) = 0;
        virtual PixelFormat getPixelFormat() = 0;

        virtual FrameAvailableCode renderToBmp(BitmapPtr pBmp, long long TimeWanted) = 0;
        virtual FrameAvailableCode renderToYCbCr420p(BitmapPtr pBmpY, BitmapPtr pBmpCb, 
                BitmapPtr pBmpCr, long long TimeWanted) = 0;
        virtual bool isEOF(StreamSelect Stream = SS_ALL) = 0;
        
        virtual int fillAudioBuffer(AudioBufferPtr pBuffer) = 0;
};

typedef boost::shared_ptr<IVideoDecoder> VideoDecoderPtr;

}
#endif 

