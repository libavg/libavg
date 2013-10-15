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

#ifndef _FFMpegFrameDecoder_H_
#define _FFMpegFrameDecoder_H_

#include "../avgconfigwrapper.h"

#include "../graphics/Bitmap.h"

#include "WrapFFMpeg.h"

namespace avg {

class AVG_API FFMpegFrameDecoder
{
    public:
        FFMpegFrameDecoder(AVStream* pStream);
        virtual ~FFMpegFrameDecoder();

        bool decodePacket(AVPacket* pPacket, AVFrame* pFrame, bool bFrameAfterSeek);
        bool decodeLastFrame(AVFrame* pFrame);
        void convertFrameToBmp(AVFrame* pFrame, BitmapPtr pBmp);
        void copyPlaneToBmp(BitmapPtr pBmp, unsigned char * pData, int stride);

        void handleSeek();

        virtual float getCurTime() const;
        virtual float getFPS() const;
        virtual void setFPS(float fps);

        virtual bool isEOF() const;
        
    private:
        float getFrameTime(long long dts, bool bFrameAfterSeek);

        SwsContext * m_pSwsContext;
        AVStream* m_pStream;

        bool m_bEOF;
        
        float m_TimeUnitsPerSecond;
        long long m_StartTimestamp;
        float m_LastFrameTime;

        bool m_bUseStreamFPS;
        float m_FPS;
};

typedef boost::shared_ptr<FFMpegFrameDecoder> FFMpegFrameDecoderPtr;

}
#endif 

