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

#ifndef _SyncVideoDecoder_H_
#define _SyncVideoDecoder_H_

#include "../avgconfigwrapper.h"
#include "VideoDecoder.h"
#include "FFMpegDemuxer.h"
#include "FFMpegFrameDecoder.h"

namespace avg {

class AVG_API SyncVideoDecoder: public VideoDecoder
{
    public:
        SyncVideoDecoder();
        virtual ~SyncVideoDecoder();
        virtual void open(const std::string& sFilename, bool bUseHardwareAcceleration, 
                bool bEnableSound);
        virtual void startDecoding(bool bDeliverYCbCr, const AudioParams* pAP);
        virtual void close();


        virtual int getCurFrame() const;
        virtual int getNumFramesQueued() const;
        virtual float getCurTime() const;
        virtual float getFPS() const;
        virtual void setFPS(float fps);
        virtual FrameAvailableCode renderToBmps(std::vector<BitmapPtr>& pBmps,
                float timeWanted);
        virtual void throwAwayFrame(float timeWanted);

        virtual void seek(float destTime);
        virtual void loop();
        virtual bool isEOF() const;
        
    private:
        FrameAvailableCode readFrameForTime(AVFrame* pFrame, float timeWanted);
        void readFrame(AVFrame* pFrame);

        FFMpegFrameDecoderPtr m_pFrameDecoder;
        bool m_bVideoSeekDone;

        FFMpegDemuxer * m_pDemuxer;
        
        bool m_bProcessingLastFrames;
        bool m_bFirstPacket;

        bool m_bUseStreamFPS;
        float m_FPS;
        AVFrame* m_pFrame;
};

typedef boost::shared_ptr<SyncVideoDecoder> SyncVideoDecoderPtr;

}
#endif 

