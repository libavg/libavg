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

#include <boost/thread/mutex.hpp>

namespace avg {

class AudioBuffer;
typedef boost::shared_ptr<AudioBuffer> AudioBufferPtr;
class VDPAUDecoder;

class AVG_API FFMpegDecoder: public VideoDecoder
{
    public:
        FFMpegDecoder();
        virtual ~FFMpegDecoder();
        virtual void open(const std::string& sFilename, bool bThreadedDemuxer,
                bool bUseHardwareAcceleration, bool bEnableSound);
        virtual void startDecoding(bool bDeliverYCbCr, const AudioParams* pAP);
        virtual void close();
        virtual DecoderState getState() const;
        virtual VideoInfo getVideoInfo() const;

        virtual float getNominalFPS() const;
        virtual float getFPS() const;
        virtual PixelFormat getPixelFormat() const;

        // Called from video thread.
        virtual IntPoint getSize() const;
        virtual int getCurFrame() const;
        virtual int getNumFramesQueued() const;
        virtual float getCurTime(StreamSelect stream = SS_DEFAULT) const;
        virtual void setFPS(float fps);
        virtual FrameAvailableCode renderToBmps(std::vector<BitmapPtr>& pBmps,
                float timeWanted);
#ifdef AVG_ENABLE_VDPAU
        virtual FrameAvailableCode renderToVDPAU(vdpau_render_state** ppRenderState);
#endif
        virtual void throwAwayFrame(float timeWanted);
        bool isVideoSeekDone();
        int getSeekSeqNum();

        virtual void seek(float destTime);
        virtual void loop();
        virtual bool isEOF(StreamSelect stream = SS_ALL) const;
        
        AVStream* getAudioStream() const;
        int getAStreamIndex() const;
        IDemuxer* getDemuxer() const;

        static void logConfig();

    private:
        void initVideoSupport();
        bool usesVDPAU() const;
        int openCodec(int streamIndex, bool bUseHardwareAcceleration);
        PixelFormat calcPixelFormat(bool bUseYCbCr);
        virtual float getDuration(StreamSelect streamSelect = SS_DEFAULT) const;
        virtual int getNumFrames() const;

        DecoderState m_State;
        AVFormatContext * m_pFormatContext;
        PixelFormat m_PF;
        std::string m_sFilename;
        bool m_bThreadedDemuxer;

        FrameAvailableCode readFrameForTime(AVFrame& frame, float timeWanted);
        void convertFrameToBmp(AVFrame& frame, BitmapPtr pBmp);
        float getFrameTime(long long dts);
        float calcStreamFPS() const;
        std::string getStreamPF() const;
        AVCodecContext const * getCodecContext() const;
        AVCodecContext * getCodecContext();

        SwsContext * m_pSwsContext;
        IntPoint m_Size;
        float m_TimeUnitsPerSecond;
        bool m_bUseStreamFPS;
        bool m_bVideoSeekDone;
        int m_SeekSeqNum;

        int m_AStreamIndex;

        float readFrame(AVFrame& frame);

        IDemuxer * m_pDemuxer;
        AVStream * m_pVStream;
        AVStream * m_pAStream;
#ifdef AVG_ENABLE_VDPAU
        VDPAUDecoder* m_pVDPAUDecoder;
#endif
        int m_VStreamIndex;
        bool m_bEOFPending;
        bool m_bVideoEOF;
        bool m_bFirstPacket;
        float m_LastVideoFrameTime;

        float m_FPS;

        static bool s_bInitialized;
        // Prevents different decoder instances from executing open/close simultaneously
        static boost::mutex s_OpenMutex;   
};

typedef boost::shared_ptr<FFMpegDecoder> FFMpegDecoderPtr;

}
#endif 

