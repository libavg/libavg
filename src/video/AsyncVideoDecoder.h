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

#ifndef _AsyncVideoDecoder_H_
#define _AsyncVideoDecoder_H_

#include "../api.h"
#include "FFMpegDecoder.h"
#include "AsyncDemuxer.h"
#include "VideoDecoderThread.h"
#include "AudioDecoderThread.h"
#include "VideoMsg.h"

#include "../graphics/Bitmap.h"
#include "../audio/AudioParams.h"

#include <boost/thread/mutex.hpp>

#include <string>

namespace avg {

class AVG_API AsyncVideoDecoder: public VideoDecoder
{
public:
    AsyncVideoDecoder(int queueLength);
    virtual ~AsyncVideoDecoder();
    virtual void open(const std::string& sFilename, bool bUseHardwareAcceleration, 
            bool bEnableSound);
    virtual void startDecoding(bool bDeliverYCbCr, const AudioParams* pAP);
    virtual void close();
    virtual void seek(float destTime);
    virtual void loop();
    virtual int getCurFrame() const;
    virtual int getNumFramesQueued() const;
    virtual float getCurTime(StreamSelect stream = SS_DEFAULT) const;
    virtual float getFPS() const;
    virtual void setFPS(float fps);

    virtual FrameAvailableCode renderToBmps(std::vector<BitmapPtr>& pBmps, 
            float timeWanted);
    void updateAudioStatus();
    virtual bool isEOF(StreamSelect stream = SS_ALL) const;
    virtual void throwAwayFrame(float timeWanted);
   
    AudioMsgQueuePtr getAudioMsgQ();
    AudioMsgQueuePtr getAudioStatusQ() const;

private:
    VideoMsgPtr getBmpsForTime(float timeWanted, FrameAvailableCode& frameAvailable);
    VideoMsgPtr getNextBmps(bool bWait);
    void waitForSeekDone();
    void checkForSeekDone();
    void handleVSeekMsg(VideoMsgPtr pMsg);
    void handleAudioMsg(AudioMsgPtr pMsg);
    void handleSeekDone(AudioMsgPtr pMsg);
    void returnFrame(VideoMsgPtr pFrameMsg);
    bool isSeeking() const;

    FFMpegDecoderPtr m_pSyncDecoder;
    int m_QueueLength;

    AsyncDemuxer* m_pDemuxer;

    boost::thread* m_pVDecoderThread;
    VideoDecoderThread::CQueuePtr m_pVCmdQ;
    VideoMsgQueuePtr m_pVMsgQ;

    boost::thread* m_pADecoderThread;
    AudioDecoderThread::CQueuePtr m_pACmdQ;
    AudioMsgQueuePtr m_pAMsgQ;
    AudioMsgQueuePtr m_pAStatusQ;

    bool m_bUseStreamFPS;
    float m_FPS;
    
    int m_NumSeeksSent;
    int m_NumVSeeksDone;
    bool m_bWasSeeking;

    bool m_bAudioEOF;
    bool m_bVideoEOF;
    bool m_bASeekPending;

    float m_LastVideoFrameTime;
    float m_CurVideoFrameTime;
    float m_LastAudioFrameTime;
};

typedef boost::shared_ptr<AsyncVideoDecoder> AsyncVideoDecoderPtr;

}
#endif 

