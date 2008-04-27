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
        virtual void open(const std::string& sFilename, const AudioParams&,
                YCbCrMode ycbcrMode, bool bThreadedDemuxer);
        virtual void close();
        virtual void seek(long long DestTime);
        virtual StreamSelect getMasterStream();
        virtual void setMasterStream(StreamSelect Stream);
        virtual bool hasVideo();
        virtual bool hasAudio();
        virtual IntPoint getSize();
        virtual int getCurFrame();
        virtual int getNumFrames();
        virtual long long getCurTime(StreamSelect Stream = SS_DEFAULT);
        virtual long long getDuration();
        virtual double getNominalFPS();
        virtual double getFPS();
        virtual void setFPS(double FPS);
        virtual double getSpeedFactor();
        virtual void setSpeedFactor(double Speed);
        virtual double getVolume();
        virtual void setVolume(double Volume);
        virtual void setAudioEnabled(bool bEnabled);
        virtual void setAudioFormat(int Channels, int SampleRate);
        virtual PixelFormat getPixelFormat();

        virtual FrameAvailableCode renderToBmp(BitmapPtr pBmp, long long TimeWanted);
        virtual FrameAvailableCode renderToYCbCr420p(BitmapPtr pBmpY, BitmapPtr pBmpCb, 
                BitmapPtr pBmpCr, long long TimeWanted);
        virtual bool isEOF(StreamSelect Stream = SS_ALL);

        virtual int fillAudioBuffer(AudioBufferPtr pBuffer);

    private:
        void initVideoSupport();
        FrameAvailableCode readFrameForTime(AVFrame& Frame, long long TimeWanted);
        void readFrame(AVFrame& Frame, long long& FrameTime);
        PixelFormat calcPixelFormat(YCbCrMode ycbcrMode);
        void convertFrameToBmp(AVFrame& Frame, BitmapPtr pBmp);
        long long getFrameTime(AVPacket* pPacket);
        long long getStartTime(StreamSelect Stream = SS_DEFAULT);
        int copyRawAudio(unsigned char* buf, int size);
        int copyResampledAudio(unsigned char* buf, int size);
        void resampleAudio();
        int decodeAudio();
        void volumize(short* buffer, int size);

        IDemuxer * m_pDemuxer;
        AVFormatContext * m_pFormatContext;
        AVStream * m_pVStream;
        AVStream * m_pAStream;
        int m_VStreamIndex;
        int m_AStreamIndex;
        bool m_bEOFPending;
        bool m_bVideoEOF;
        bool m_bAudioEOF;
        bool m_bAudioEnabled;
        PixelFormat m_PF;
#ifdef AVG_ENABLE_SWSCALE
        SwsContext * m_pSwsContext;
#endif

        bool m_bForceMasterStream;
        StreamSelect m_MasterStream;
                
        int m_Channels;
        int m_SampleRate;
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
        boost::mutex m_AudioMutex;

        double m_Volume;
        double m_LastVolume;
        double m_LastAudioFrameTime;
        long long m_AudioStartTimestamp;
        
        unsigned char * m_pPacketData;
        AVPacket * m_pPacket;
        int m_PacketLenLeft;
        bool m_bFirstPacket;
        std::string m_sFilename;
        IntPoint m_Size;

        double m_TimeUnitsPerSecond;
        long long m_VideoStartTimestamp;
        long long m_LastVideoFrameTime;
        bool m_bUseStreamFPS;
        double m_FPS;
        long long m_StreamTimeOffset;
        double m_SpeedFactor;

        ProfilingZone * m_pRenderToBmpProfilingZone;
        ProfilingZone * m_pConvertImageProfilingZone;

        static bool m_bInitialized;
        // Prevents different decoder instances from executing open/close simultaneously
        static boost::mutex s_OpenMutex;   
};

}
#endif 

