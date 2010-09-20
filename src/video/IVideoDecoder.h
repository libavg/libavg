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

#ifndef _IVideoDecoder_H_
#define _IVideoDecoder_H_

#include "../api.h"

#include "VideoInfo.h"

#include "../graphics/Bitmap.h"
#include "../audio/IAudioSource.h"

#include <string>

namespace avg {

enum FrameAvailableCode {
    FA_NEW_FRAME, FA_USE_LAST_FRAME, FA_STILL_DECODING
};

enum StreamSelect {
    SS_AUDIO, SS_VIDEO, SS_DEFAULT, SS_ALL
};

class AVG_API IVideoDecoder
{
    public:
        enum DecoderState {CLOSED, OPENED, DECODING};
        virtual ~IVideoDecoder() {};
        virtual void open(const std::string& sFilename, bool bSyncDemuxer) = 0;
        virtual void startDecoding(bool bDeliverYCbCr, const AudioParams* AP) = 0;
        virtual void close() = 0;
        virtual DecoderState getState() const = 0;
        virtual VideoInfo getVideoInfo() const = 0;

        virtual void seek(long long DestTime) = 0;
        virtual void loop() {};
        virtual IntPoint getSize() const = 0;
        virtual int getCurFrame() const = 0;
        virtual int getNumFramesQueued() const = 0;
        virtual long long getCurTime(StreamSelect Stream = SS_DEFAULT) const = 0;
        virtual double getNominalFPS() const = 0;
        virtual double getFPS() const = 0;
        virtual void setFPS(double FPS) = 0;
        virtual double getVolume() const = 0;
        virtual void setVolume(double Volume) = 0;
        virtual PixelFormat getPixelFormat() const = 0;

        virtual FrameAvailableCode renderToBmp(BitmapPtr pBmp,
                long long timeWanted);
        virtual FrameAvailableCode renderToBmps(std::vector<BitmapPtr>& pBmps,
                long long timeWanted) = 0;
        virtual bool isEOF(StreamSelect Stream = SS_ALL) const = 0;
        virtual void throwAwayFrame(long long timeWanted) = 0;
        
        virtual int fillAudioBuffer(AudioBufferPtr pBuffer) = 0;
};

inline FrameAvailableCode IVideoDecoder::renderToBmp(BitmapPtr pBmp, long long timeWanted)
{
    std::vector<BitmapPtr> pBmps;
    pBmps.push_back(pBmp);
    return renderToBmps(pBmps, timeWanted);
}

typedef boost::shared_ptr<IVideoDecoder> VideoDecoderPtr;

}
#endif 

