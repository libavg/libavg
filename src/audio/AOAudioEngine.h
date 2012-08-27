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
//  Original author of this file is Benjamin Granzow (kontakt@bgranzow.de).
//

#ifndef _AOAudioEngine_H_
#define _AOAudioEngine_H_

#include "AOAudioEngineThread.h"

namespace avg {

class AVG_API AOAudioEngine
{   
    public:
        static AOAudioEngine* get();
        AOAudioEngine(const AudioParams& ap, float volume);
        virtual ~AOAudioEngine();
        
        virtual int getChannels();
        virtual int getSampleRate();
        virtual const AudioParams * getParams();
        
        virtual void teardown();
        
        virtual void setAudioEnabled(bool bEnabled);
        
        virtual void play();
        virtual void pause();

        virtual void addSource(IAudioSource* pSource);
        virtual void removeSource(IAudioSource* pSource);
        virtual void setVolume(float volume);
        
    private:
        bool m_bEnabled;
        AudioParams m_AP;

        boost::thread* m_pThread;
        AOAudioEngineThread::CQueuePtr m_pCmdQueue;

        static AOAudioEngine* s_pInstance;
};

}

#endif //_SDLAudioEngine_H_
