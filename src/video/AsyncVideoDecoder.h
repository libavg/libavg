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

#ifndef _AsyncVideoDecoder_H_
#define _AsyncVideoDecoder_H_

#include "../api.h"
#include "IVideoDecoder.h"
#include "VideoDecoderThread.h"
#include "AudioDecoderThread.h"
#include "FrameVideoMsg.h"
#include "AudioVideoMsg.h"

#include "../graphics/Bitmap.h"
#include "../audio/AudioParams.h"

#include <boost/thread/mutex.hpp>

#include <string>

namespace avg {

class AVG_API AsyncVideoDecoder: public IVideoDecoder
{
    public:
        AsyncVideoDecoder(VideoDecoderPtr pSyncDecoder);
        virtual ~AsyncVideoDecoder();
        virtual void open(const std::string& sFilename, bool bSyncDemuxer);
        virtual void startDecoding(bool bDeliverYCbCr, const AudioParams* AP);
        virtual void close();
        virtual DecoderState getState() const;
        virtual VideoInfo getVideoInfo() const;
        virtual void seek(long long DestTime);
        virtual IntPoint getSize() const;
        virtual int getCurFrame() const;
        virtual int getNumFramesQueued() const;
        virtual long long getCurTime(StreamSelect Stream = SS_DEFAULT) const;
        virtual double getNominalFPS() const;
        virtual double getFPS() const;
        virtual void setFPS(double FPS);
        virtual double getVolume() const;
        virtual void setVolume(double Volume);
        virtual PixelFormat getPixelFormat() const;

        virtual FrameAvailableCode renderToBmp(BitmapPtr pBmp, long long timeWanted);
        virtual FrameAvailableCode renderToYCbCr420p(BitmapPtr pBmpY, BitmapPtr pBmpCb, 
                BitmapPtr pBmpCr, long long timeWanted);
        virtual bool isEOF(StreamSelect Stream = SS_ALL) const;
        virtual void throwAwayFrame(long long timeWanted);
        
        virtual int fillAudioBuffer(AudioBufferPtr pBuffer);
        
    private:
        FrameVideoMsgPtr getBmpsForTime(long long timeWanted, 
                FrameAvailableCode& FrameAvailable);
        FrameVideoMsgPtr getNextBmps(bool bWait);
        void waitForSeekDone();
        void returnFrame(FrameVideoMsgPtr& pFrameMsg);

        DecoderState m_State;
        VideoDecoderPtr m_pSyncDecoder;
        std::string m_sFilename;

        boost::thread* m_pVDecoderThread;
        VideoDecoderThread::CmdQueuePtr m_pVCmdQ;
        VideoMsgQueuePtr m_pVMsgQ;

        boost::thread* m_pADecoderThread;
        boost::mutex m_AudioMutex;
        AudioDecoderThread::CmdQueuePtr m_pACmdQ;
        VideoMsgQueuePtr m_pAMsgQ;
        AudioVideoMsgPtr m_pAudioMsg;
        unsigned char* m_AudioMsgData;
        int m_AudioMsgSize;

        VideoInfo m_VideoInfo;

        IntPoint m_Size;
        int m_NumFrames;
        bool m_bUseStreamFPS;
        PixelFormat m_PF;
        
        bool m_bAudioEOF;
        bool m_bVideoEOF;
        bool m_bSeekPending;
        boost::mutex m_SeekMutex;
        double m_Volume;

        long long m_LastVideoFrameTime;
        long long m_LastAudioFrameTime;
};

}
#endif 

