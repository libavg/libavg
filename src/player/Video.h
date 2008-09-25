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

#ifndef _Video_H_
#define _Video_H_

// Python docs say python.h should be included before any standard headers (!)
#include "WrapPython.h" 

#include "Node.h"
#include "VideoBase.h"

#include "../base/Point.h"
#include "../base/IFrameListener.h"

#include "../audio/IAudioSource.h"

namespace avg {

class IVideoDecoder;

class Video : public VideoBase, IFrameListener, IAudioSource
{
    public:
        static NodeDefinition createDefinition();
        
        Video (const ArgList& Args, bool bFromXML);
        virtual ~Video ();
        
        virtual void setRenderingEngines(DisplayEngine * pDisplayEngine, 
                AudioEngine * pAudioEngine);

        const std::string& getHRef() const;
        void setHRef(const std::string& href);
        void setVolume(double Volume);
        void checkReload();

        int getNumFrames() const;
        int getCurFrame() const;
        int getNumFramesQueued() const;
        void seekToFrame(int FrameNum);
        long long getDuration() const;
        long long getCurTime() const;
        void seekToTime(long long Time);
        bool getLoop() const;
        bool isThreaded() const;
        void setEOFCallback(PyObject * pEOFCallback);

        virtual void onFrameEnd();
        
        virtual int fillAudioBuffer(AudioBufferPtr pBuffer);
        virtual IntPoint getMediaSize();

    protected:
        virtual void changeVideoState(VideoState NewVideoState);

    private:
        bool renderToSurface(ISurface * pSurface);
        bool canRenderToBackbuffer(int BitsPerPixel);
        void seek(long long DestTime);
        void onEOF();
       
        virtual void open(YCbCrMode ycbcrMode);
        virtual void close();
        virtual PixelFormat getPixelFormat();
        virtual double getFPS();
        virtual long long getNextFrameTime();

        std::string m_href;
        std::string m_Filename;
        bool m_bLoop;
        bool m_bThreaded;
        double m_FPS;
        bool m_bEOFPending;
        PyObject * m_pEOFCallback;
        int m_FramesTooLate;
        int m_FramesInRowTooLate;
        int m_FramesPlayed;

        long long m_StartTime;
        long long m_PauseTime;
        long long m_PauseStartTime;

        IVideoDecoder * m_pDecoder;
        double m_Volume;
};

}

#endif 

