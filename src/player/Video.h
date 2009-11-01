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
#include "../api.h"
#include "WrapPython.h" 

#include "Node.h"
#include "RasterNode.h"

#include "../base/Point.h"
#include "../base/IFrameEndListener.h"

#include "../audio/IAudioSource.h"

namespace avg {

class IVideoDecoder;

class AVG_API Video : public RasterNode, IFrameEndListener, IAudioSource
{
    public:
        static NodeDefinition createDefinition();
        
        Video (const ArgList& Args);
        virtual ~Video ();
        
        virtual void setRenderingEngines(DisplayEngine * pDisplayEngine, 
                AudioEngine * pAudioEngine);
        virtual void connect();
        virtual void disconnect(bool bKill);

        void play();
        void stop();
        void pause();

        const std::string& getHRef() const;
        void setHRef(const std::string& href);
        double getVolume();
        void setVolume(double Volume);
        virtual double getFPS() const;
        void checkReload();

        int getNumFrames() const;
        int getCurFrame() const;
        int getNumFramesQueued() const;
        void seekToFrame(int FrameNum);
        std::string getStreamPixelFormat() const;
        long long getDuration() const;
        int getBitrate() const;
        std::string getVideoCodec() const;
        std::string getAudioCodec() const;
        int getAudioSampleRate() const;
        int getNumAudioChannels() const;

        long long getCurTime() const;
        void seekToTime(long long Time);
        bool getLoop() const;
        bool isThreaded() const;
        bool hasAudio() const;
        void setEOFCallback(PyObject * pEOFCallback);

        virtual void render (const DRect& Rect);
        virtual void preRender();
        virtual void onFrameEnd();
        
        virtual int fillAudioBuffer(AudioBufferPtr pBuffer);
        virtual IntPoint getMediaSize();

    protected:

    private:
        bool renderToSurface(OGLTiledSurface * pSurface);
        void seek(long long DestTime);
        void onEOF();
       
        void open();
        void startDecoding();
        void close();
        enum VideoState {Unloaded, Paused, Playing};
        void changeVideoState(VideoState NewVideoState);
        PixelFormat getPixelFormat();
        long long getNextFrameTime() const;
        void exceptionIfNoAudio(const std::string& sFuncName) const;
        void exceptionIfUnloaded(const std::string& sFuncName) const;

        VideoState m_VideoState;

        bool m_bFrameAvailable;
        bool m_bFirstFrameDecoded;

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
        bool m_bSeekPending;

        long long m_StartTime;
        long long m_PauseTime;
        long long m_PauseStartTime;

        IVideoDecoder * m_pDecoder;
        double m_Volume;
};

}

#endif 

