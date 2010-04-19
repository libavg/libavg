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

#ifndef _SoundNode_H_
#define _SoundNode_H_

#include "../api.h"

#include "AreaNode.h"

#include "../base/IFrameEndListener.h"
#include "../base/UTF8String.h"
#include "../audio/IAudioSource.h"

namespace avg {

class IVideoDecoder;

class AVG_API SoundNode : public AreaNode, IFrameEndListener, IAudioSource
{
    public:
        static NodeDefinition createDefinition();

        SoundNode(const ArgList& Args);
        virtual ~SoundNode();

        virtual void setRenderingEngines(DisplayEngine * pDisplayEngine, 
                AudioEngine * pAudioEngine);
        virtual void connect(CanvasPtr pCanvas);
        virtual void disconnect(bool bKill);

        void play();
        void stop();
        void pause();

        const UTF8String& getHRef() const;
        void setHRef(const UTF8String& href);
        double getVolume();
        void setVolume(double Volume);
        void checkReload();

        long long getDuration() const;
        std::string getAudioCodec() const;
        int getAudioSampleRate() const;
        int getNumAudioChannels() const;

        long long getCurTime() const;
        void seekToTime(long long Time);
        bool getLoop() const;
        void setEOFCallback(PyObject * pEOFCallback);

        virtual void onFrameEnd();

        virtual int fillAudioBuffer(AudioBufferPtr pBuffer);

    private:
        void seek(long long DestTime);
        void onEOF();

        enum SoundState {Unloaded, Paused, Playing};
        void changeSoundState(SoundState NewSoundState);
        void open();
        void startDecoding();
        void close();
        void exceptionIfUnloaded(const std::string& sFuncName) const;

        UTF8String m_href;
        std::string m_Filename;
        bool m_bLoop;
        PyObject * m_pEOFCallback;
        bool m_bAudioEnabled;

        long long m_StartTime;
        long long m_PauseTime;
        long long m_PauseStartTime;

        IVideoDecoder * m_pDecoder;
        double m_Volume;
        SoundState m_State;
};

}

#endif 

