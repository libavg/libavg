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

#ifndef _FFMpegDecoder_H_
#define _FFMpegDecoder_H_

#include "IVideoDecoder.h"
#include "IDemuxer.h"

#include "../audio/AudioParams.h"
#include "../base/ProfilingZone.h"
#include "../avgconfigwrapper.h"

#ifdef _WIN32
#define EMULATE_INTTYPES
#if !defined INT64_C
#define INT64_C(c) c##i64
#endif
#else
// This is probably GCC-specific.
#if !defined INT64_C
#if defined __WORDSIZE && __WORDSIZE == 64
#define INT64_C(c) c ## L
#else
#define INT64_C(c) c ## LL
#endif
#endif
#endif

extern "C" {
#include <ffmpeg/avcodec.h>
#include <ffmpeg/avformat.h>
#ifdef AVG_ENABLE_SWSCALE
#include <ffmpeg/swscale.h>
#endif
}
#include <boost/thread/mutex.hpp>

namespace avg {

class AudioBuffer;

class FFMpegDecoder: public IVideoDecoder
{
    public:
        FFMpegDecoder();
        virtual ~FFMpegDecoder();
        virtual void open(const std::string& sFilename, const AudioParams* AP,
                YCbCrMode ycbcrMode, bool bThreadedDemuxer);
        virtual void close();
        virtual StreamSelect getMasterStream();
        virtual bool hasAudio();
        virtual int getNumFrames();
        virtual long long getDuration();

        virtual double getNominalFPS();
        virtual double getFPS();
        virtual double getVolume();
        virtual PixelFormat getPixelFormat();

        // Called from video thread.
        virtual IntPoint getSize();
        virtual int getCurFrame();
        virtual int getNumFramesQueued();
        virtual long long getCurTime(StreamSelect Stream = SS_DEFAULT);
        virtual void setFPS(double FPS);
        virtual FrameAvailableCode renderToBmp(BitmapPtr pBmp, long long TimeWanted);
        virtual FrameAvailableCode renderToYCbCr420p(BitmapPtr pBmpY, BitmapPtr pBmpCb, 
                BitmapPtr pBmpCr, long long TimeWanted);
        
        // Called from audio decoder thread
        virtual void setVolume(double Volume);
        virtual int fillAudioBuffer(AudioBufferPtr pBuffer);

        // Called from video and audio threads
        virtual void seek(long long DestTime);
        virtual bool hasVideo();
        virtual bool isEOF(StreamSelect Stream = SS_ALL);

    private:
        void initVideoSupport();
        PixelFormat calcPixelFormat(YCbCrMode ycbcrMode);

        AVFormatContext * m_pFormatContext;
        PixelFormat m_PF;
        std::string m_sFilename;

        // Used from video thread.
        FrameAvailableCode readFrameForTime(AVFrame& Frame, long long TimeWanted);
        void convertFrameToBmp(AVFrame& Frame, BitmapPtr pBmp);
        long long getFrameTime(AVPacket* pPacket);
        double calcStreamFPS();

#ifdef AVG_ENABLE_SWSCALE
        SwsContext * m_pSwsContext;
#endif
        IntPoint m_Size;
        double m_TimeUnitsPerSecond;
        bool m_bUseStreamFPS;
        ProfilingZone * m_pRenderToBmpProfilingZone;
        ProfilingZone * m_pConvertImageProfilingZone;

        // Used from audio thread.
        int copyRawAudio(unsigned char* buf, int size);
        int copyResampledAudio(unsigned char* buf, int size);
        void resampleAudio();
        int decodeAudio();
        void volumize(AudioBufferPtr pBuffer);

        int m_AStreamIndex;
        AudioParams m_AP;
        AVPacket * m_AudioPacket;
        unsigned char * m_AudioPacketData;
        int m_AudioPacketSize;
        char * m_pSampleBuffer;
        int m_SampleBufferStart;
        int m_SampleBufferEnd;
        int m_SampleBufferLeft;
        char * m_pResampleBuffer;
        int m_ResampleBufferEnd;
        int m_ResampleBufferStart;
        int m_ResampleBufferSize;
        int m_EffectiveSampleRate;
        ReSampleContext * m_pAudioResampleContext;
        double m_Volume;
        double m_LastVolume;
        long long m_AudioStartTimestamp;

        // Used from video and audio threads.
        void readFrame(AVFrame& Frame, long long& FrameTime);
        long long getStartTime(StreamSelect Stream = SS_DEFAULT);

        IDemuxer * m_pDemuxer;
        AVStream * m_pVStream;
        AVStream * m_pAStream;
        int m_VStreamIndex;
        bool m_bEOFPending;
        bool m_bVideoEOF;
        bool m_bAudioEOF;
        boost::mutex m_AudioMutex;
        double m_LastAudioFrameTime;
        unsigned char * m_pPacketData;
        AVPacket * m_pPacket;
        int m_PacketLenLeft;
        bool m_bFirstPacket;
        long long m_VideoStartTimestamp;
        long long m_LastVideoFrameTime;

        double m_FPS;
        long long m_StreamTimeOffset;

        static bool m_bInitialized;
        // Prevents different decoder instances from executing open/close simultaneously
        static boost::mutex s_OpenMutex;   
};

}
#endif 

