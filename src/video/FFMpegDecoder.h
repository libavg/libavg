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

#ifndef _FFMpegDecoder_H_
#define _FFMpegDecoder_H_

#include "../avgconfigwrapper.h"
#include "IVideoDecoder.h"
#include "IDemuxer.h"

#include "../audio/AudioParams.h"
#include "../base/ProfilingZone.h"
#include "../avgconfigwrapper.h"

#include "WrapFFMpeg.h"

#include <boost/thread/mutex.hpp>

namespace avg {

class AudioBuffer;

class AVG_API FFMpegDecoder: public IVideoDecoder
{
    public:
        FFMpegDecoder();
        virtual ~FFMpegDecoder();
        virtual void open(const std::string& sFilename, bool bThreadedDemuxer);
        virtual void startDecoding(bool bDeliverYCbCr, const AudioParams* pAP);
        virtual void close();
        virtual DecoderState getState() const;
        virtual VideoInfo getVideoInfo() const;

        virtual double getNominalFPS() const;
        virtual double getFPS() const;
        virtual double getVolume() const;
        virtual PixelFormat getPixelFormat() const;

        // Called from video thread.
        virtual IntPoint getSize() const;
        virtual int getCurFrame() const;
        virtual int getNumFramesQueued() const;
        virtual double getCurTime(StreamSelect stream = SS_DEFAULT) const;
        virtual void setFPS(double fps);
        virtual FrameAvailableCode renderToBmps(std::vector<BitmapPtr>& pBmps,
                double timeWanted);
        virtual void throwAwayFrame(double timeWanted);
        
        // Called from audio decoder thread
        virtual void setVolume(double volume);
        virtual int fillAudioBuffer(AudioBufferPtr pBuffer);

        // Called from video and audio threads
        virtual void seek(double destTime);
        virtual void loop();
        virtual bool isEOF(StreamSelect stream = SS_ALL) const;

    private:
        void initVideoSupport();
        PixelFormat calcPixelFormat(bool bUseYCbCr);
        virtual double getDuration() const;
        virtual int getNumFrames() const;

        DecoderState m_State;
        AVFormatContext * m_pFormatContext;
        PixelFormat m_PF;
        std::string m_sFilename;
        bool m_bThreadedDemuxer;

        // Used from video thread.
        FrameAvailableCode readFrameForTime(AVFrame& Frame, double timeWanted);
        void convertFrameToBmp(AVFrame& Frame, BitmapPtr pBmp);
        double getFrameTime(long long dts);
        double calcStreamFPS() const;
        std::string getStreamPF() const;

        SwsContext * m_pSwsContext;
        IntPoint m_Size;
        double m_TimeUnitsPerSecond;
        bool m_bUseStreamFPS;

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
        double m_AudioStartTimestamp;

        // Used from video and audio threads.
        double readFrame(AVFrame& frame);
        double getStartTime();

        IDemuxer * m_pDemuxer;
        AVStream * m_pVStream;
        AVStream * m_pAStream;
        int m_VStreamIndex;
        bool m_bEOFPending;
        bool m_bVideoEOF;
        bool m_bAudioEOF;
        boost::mutex m_AudioMutex;
        double m_LastAudioFrameTime;
        bool m_bFirstPacket;
        double m_VideoStartTimestamp;
        double m_LastVideoFrameTime;

        double m_FPS;
        double m_StreamTimeOffset;

        static bool s_bInitialized;
        // Prevents different decoder instances from executing open/close simultaneously
        static boost::mutex s_OpenMutex;   
};

}
#endif 

