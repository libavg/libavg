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

#ifndef _FFMpegDecoder_H_
#define _FFMpegDecoder_H_

#include "../avgconfigwrapper.h"
#include "VideoDecoder.h"
#include "IDemuxer.h"

#include "../audio/AudioParams.h"

#include "WrapFFMpeg.h"

#ifdef AVG_ENABLE_VDPAU
#include "VDPAUHelper.h"
#include <libavcodec/vdpau.h>
#endif

namespace avg {

class AudioBuffer;
typedef boost::shared_ptr<AudioBuffer> AudioBufferPtr;
class VDPAUDecoder;
class AsyncDemuxer;

class AVG_API FFMpegDecoder
{
    public:
        FFMpegDecoder(AsyncDemuxer* pDemuxer, AVStream* pStream, int streamIndex, 
                PixelFormat pf, bool bUseVDPAU);
        virtual ~FFMpegDecoder();

        virtual int getCurFrame() const;
        virtual float getCurTime() const;
        virtual void setFPS(float fps);
        virtual FrameAvailableCode renderToBmps(std::vector<BitmapPtr>& pBmps,
                float timeWanted);
#ifdef AVG_ENABLE_VDPAU
        virtual FrameAvailableCode renderToVDPAU(vdpau_render_state** ppRenderState);
#endif
        virtual void throwAwayFrame(float timeWanted);
        bool isVideoSeekDone();
        int getSeekSeqNum();

        virtual bool isEOF(StreamSelect stream = SS_ALL) const;
        
    private:
        FrameAvailableCode readFrameForTime(AVFrame& frame, float timeWanted);
        void convertFrameToBmp(AVFrame& frame, BitmapPtr pBmp);
        float getFrameTime(long long dts);
        std::string getStreamPF() const;

        SwsContext * m_pSwsContext;
        float m_TimeUnitsPerSecond;
        bool m_bUseStreamFPS;
        bool m_bVideoSeekDone;
        int m_SeekSeqNum;

        float readFrame(AVFrame& frame);

        AsyncDemuxer* m_pDemuxer;
        AVStream* m_pStream;
        int m_StreamIndex;
        PixelFormat m_PF;
        bool m_bUseVDPAU;

        bool m_bEOFPending;
        bool m_bVideoEOF;
        bool m_bFirstPacket;
        long long m_VideoStartTimestamp;
        float m_LastVideoFrameTime;

        float m_FPS;
};

typedef boost::shared_ptr<FFMpegDecoder> FFMpegDecoderPtr;

}
#endif 

