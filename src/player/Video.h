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

#ifndef _Video_H_
#define _Video_H_

// Python docs say python.h should be included before any standard headers (!)
#include "WrapPython.h" 

#include "Node.h"
#include "VideoBase.h"

#include "../base/Point.h"
#include "../base/IFrameListener.h"

namespace avg {

class IVideoDecoder;

class Video : public VideoBase, IFrameListener
{
    public:
        static NodeDefinition getNodeDefinition();
        
        Video (const ArgList& Args, Player * pPlayer);
        virtual ~Video ();
        
        virtual void setDisplayEngine(DisplayEngine * pEngine);
        virtual void disconnect();

        const std::string& getHRef() const;
        void setHRef(const std::string& href);
        void checkReload();

        int getNumFrames() const;
        int getCurFrame() const;
        void seekToFrame(int FrameNum);
        bool getLoop() const;
        bool isThreaded() const;
        void setEOFCallback(PyObject * pEOFCallback);

        virtual std::string getTypeStr();

        virtual void onFrameEnd();

    protected:
        virtual void changeVideoState(VideoState NewVideoState);

    private:
        void initVideoSupport();

        bool renderToSurface(ISurface * pSurface);
        bool canRenderToBackbuffer(int BitsPerPixel);
        void seek(int DestFrame);
        void onEOF();
       
        virtual void open(YCbCrMode ycbcrMode);
        virtual void close();
        virtual PixelFormat getPixelFormat();
        virtual IntPoint getMediaSize();
        virtual double getFPS();
        virtual long long getCurTime();

        std::string m_href;
        std::string m_Filename;
        bool m_bLoop;
        bool m_bThreaded;
        double m_FPS;
        bool m_bEOFPending;
        PyObject * m_pEOFCallback;
        int m_FramesTooLate;
        int m_FramesPlayed;

        int m_CurFrame;
        long long m_StartTime;
        long long m_PauseTime;
        long long m_PauseStartTime;

        IVideoDecoder * m_pDecoder;

        static bool m_bInitialized;
};

}

#endif 

