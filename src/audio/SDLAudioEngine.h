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
//  Original author of this file is Nick Hebner (hebnern@gmail.com).
//

#ifndef _SDLAudioEngine_H_
#define _SDLAudioEngine_H_

#include "AudioEngine.h"
#include "AudioFrame.h"
#include "IProcessor.h"

#include <SDL/SDL.h>

namespace avg {

class SDLAudioEngine : public AudioEngine
{   
    public:
        SDLAudioEngine();
        virtual ~SDLAudioEngine();
        
        virtual int getChannels();
        virtual int getSampleRate();
        
        virtual void init(const AudioParams& AP);
        virtual void teardown();
        
        virtual void setAudioEnabled(bool bEnabled);
        
        virtual void play();
        virtual void pause();
        
        virtual void addSource(IAudioSource* pSource);
        virtual void removeSource(IAudioSource* pSource);
        
    private:
        void mixAudio(Uint8 *pDestBuffer, int destBufferLen);
        static void audioCallback(void *userData, Uint8 *audioBuffer, int audioBufferLen);
        void addBuffers(double *pDest, short *pSrc, int numFrames);
        
        AudioParams m_AP;
        AudioFrame* m_TempFrame;
        double * m_pMixBuffer;
        IProcessor<double>* m_pLimiter;
};

}

#endif //_SDLAudioEngine_H_
